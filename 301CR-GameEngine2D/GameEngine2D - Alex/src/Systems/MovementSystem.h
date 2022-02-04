#ifndef MOVEMENTSYSTEM_H
#define MOVEMENTSYSTEM_H

#include "../ECS/ECS.h"
#include "../EventBus/EventBus.h"
#include "../Events/CollisionEvent.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"

class MovementSystem : public System
{
public:
	MovementSystem()
	{
		RequireComponent<TransformComponent>();
		RequireComponent<RigidBodyComponent>();
	}

	void SubscribeToEvents(const std::unique_ptr<EventBus>& eventBus) {
		eventBus->SubscribeToEvent<CollisionEvent>(this, &MovementSystem::onCollision);
	}

	void onCollision(CollisionEvent& event) {
		Entity a = event.a;
		Entity b = event.b;
		Logger::Log("Collision event emitted: " + std::to_string(a.GetId()) + " and " + std::to_string(b.GetId()));

		if (a.BelongsToGroup("enemies") && b.BelongsToGroup("obstacles")) {
			// "a" is the enemy, "b" is the obstacle
			OnEnemyHitsObstacle(a, b);
		}

		if (a.BelongsToGroup("obstacles") && b.BelongsToGroup("enemies")) {
			// "b" is the enemy, "a" is the obstacle
			OnEnemyHitsObstacle(b, a);
		}
	}

	void OnEnemyHitsObstacle(Entity enemy, Entity obstacles) {
		if (enemy.HasComponent<RigidBodyComponent>() && enemy.HasComponent<SpriteComponent>()) {
			auto& rigidbody = enemy.GetComponent<RigidBodyComponent>();
			auto& sprite = enemy.GetComponent<SpriteComponent>();

			if (rigidbody.velocity.x != 0) {
				rigidbody.velocity.x *= -1;
				sprite.flip = (sprite.flip == SDL_FLIP_NONE) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
			}


			if (rigidbody.velocity.y != 0) {
				rigidbody.velocity.y *= -1;
				sprite.flip = (sprite.flip == SDL_FLIP_NONE) ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;
			}
		}
	}

	void Update(double deltaTime)
	{
		//Loop all entities that the system is interested in
		for(auto entity : GetSystemEntities()) 
		{
			//Update entity position based on its velocity
			auto& transform = entity.GetComponent<TransformComponent>();
			const auto rigidbody = entity.GetComponent<RigidBodyComponent>();

			//Update the entity position based on its velocity 
			transform.position.x += rigidbody.velocity.x * deltaTime;
			transform.position.y += rigidbody.velocity.y * deltaTime;

			//Prevent the main player from moving outside the map boundaries
			if (entity.HasTag("player")) {
				int protectionLeft = 10;
				int protectionTop = 10;
				int protectionRight = 50;
				int protectionBottom = 50;

				transform.position.x = transform.position.x < protectionLeft ? protectionLeft : transform.position.x;
				transform.position.x = transform.position.x > Game::mapWidth - protectionRight ? Game::mapWidth - protectionRight : transform.position.x;
				transform.position.y = transform.position.y < protectionTop ? protectionTop : transform.position.y;
				transform.position.y = transform.position.y > Game::mapHeight - protectionBottom ? Game::mapHeight - protectionBottom : transform.position.y;
			}

			//Check if entity is outisde the map boundaries
			bool isEntityOutsideMap = (
				transform.position.x < 0 || 
				transform.position.x > Game::mapWidth ||
				transform.position.y < 0 ||
				transform.position.y >Game::mapHeight
				);

				// Kill all entities that move outside the map boundaries
				if (isEntityOutsideMap && !entity.HasTag("player")) {
					entity.Kill();
			}
		}
	}
};

#endif // !MOVEMENTSYSTEM_H


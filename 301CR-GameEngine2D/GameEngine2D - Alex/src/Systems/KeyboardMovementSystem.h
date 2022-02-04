#ifndef KEYBOARDMOVEMENTSYSTEM_H
#define KEYBOARDMOVEMENTSYSTEM_H

#include "../ECS/ECS.h"
#include "../EventBus/EventBus.h"
#include "../Events/KeyPressedEvent.h"
#include "../Components/KeyboardControlledComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"

class KeyboardMovementSystem : public System {
public:
	KeyboardMovementSystem() {
		RequireComponent<KeyboardControlledComponent>();
		RequireComponent<SpriteComponent>();
		RequireComponent<RigidBodyComponent>();
	}

	void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus) {
		eventBus->SubscribeToEvent<KeyPressedEvent>(this, &KeyboardMovementSystem::OnKeyPressed);
	}

	void OnKeyPressed(KeyPressedEvent& event) {
		for (auto entity : GetSystemEntities()) {
			const auto keyboardcontrol = entity.GetComponent<KeyboardControlledComponent>();
			auto& sprite = entity.GetComponent<SpriteComponent>();
			auto& rigidbody = entity.GetComponent<RigidBodyComponent>();

			switch (event.symbol) {
				case SDLK_UP:
					rigidbody.velocity = keyboardcontrol.upVelocity;
					sprite.srcRect.y = sprite.height * 0;
					break;
				case SDLK_RIGHT:
					rigidbody.velocity = keyboardcontrol.rightVelocity;
					sprite.srcRect.y = sprite.height * 1;
					break;
				case SDLK_DOWN:
					rigidbody.velocity = keyboardcontrol.downVelocity;
					sprite.srcRect.y = sprite.height * 2;
					break;
				case SDLK_LEFT:
					rigidbody.velocity = keyboardcontrol.leftVelocity;
					sprite.srcRect.y = sprite.height * 3;
					break;
			}
		}

		//DEBUG:: Checking if a key is pressed
		/*std::string keyCode = std::to_string(event.symbol);
		std::string keySymbol(1, event.symbol);
		Logger::Log("Key pressed  event emitted: [" + keyCode + "] " + keySymbol);*/
	}

	void Update(){

	}
};

#endif // !KEYBOARDMOVEMENTSYSTEM_H

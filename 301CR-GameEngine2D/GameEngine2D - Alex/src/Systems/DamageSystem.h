#ifndef DAMAGESYSTEM_H
#define DAMAGESYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/ProjectileComponent.h"
#include "../Components/HealthComponent.h"
#include "../EventBus/EventBus.h"
#include "../Events/CollisionEvent.h"
#include <fstream>


class DamageSystem : public System {
/*private:
    SDL_Renderer* renderer;
    sol::state lua;
    std::unique_ptr<Registry> registry;
    std::unique_ptr<AssetManager> assetManager;
    std::unique_ptr<EventBus> eventBus;*/

public:
    //LevelLoader loader;
    int score = 0;
    
    DamageSystem() {
        RequireComponent<BoxColliderComponent>();
    }

    void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus) {
        eventBus->SubscribeToEvent<CollisionEvent>(this, &DamageSystem::onCollision);
    }

    void onCollision(CollisionEvent& event) {
        Entity a = event.a;
        Entity b = event.b;
        Logger::Log("Collision event emitted: " + std::to_string(a.GetId()) + " and " + std::to_string(b.GetId()));
    
        if (a.BelongsToGroup("projectiles") && b.HasTag("player")) {
            //"a" is the projectile and "b" is the player
            OnProjectileHitsPlayer(a, b);

        }
        if (b.BelongsToGroup("projectiles") && a.HasTag("player")) {
            //"b" is the projectile and "a" is the player
            OnProjectileHitsPlayer(b, a);
        }
        if (a.BelongsToGroup("projectiles") && b.BelongsToGroup("enemies")) {
            //"a" is the projectile and "b" is th enemy
            OnProjectileHitsEnemy(a, b);

        }
        if (b.BelongsToGroup("projectiles") && a.BelongsToGroup("enemies")) {
            //"b" is the projectile and "a" is th enemy
            OnProjectileHitsEnemy(b, a);

        }

    }

    void OnProjectileHitsPlayer(Entity projectile, Entity player) {
        auto projectileComponent = projectile.GetComponent<ProjectileComponent>();

        if (!projectileComponent.isFriendly) {
            //Reduce the health of the player by the projectile hitPercentDamage
             auto& health = player.GetComponent<HealthComponent>();

            //Substract the health of the player 
            health.healthPercentage -= projectileComponent.hitPercentDamage;

            //Kills the player when the health reaches below or equal to zero
            if (health.healthPercentage <= 0) {
                player.Kill();
            }


            //Kill the projectile
            projectile.Kill();
        }

    }   

    void OnProjectileHitsEnemy(Entity projectile, Entity enemy) {
        const auto projectileComponent = projectile.GetComponent<ProjectileComponent>();

        //Only damage the enemy is the projectile is friendly
        if (projectileComponent.isFriendly) {
            auto& health = enemy.GetComponent<HealthComponent>();

            //Substract from enemy health
            health.healthPercentage -= projectileComponent.hitPercentDamage;

            //Kills the enemy if health reaches below or equal to zero
            if (health.healthPercentage <= 0) {
                enemy.Kill();
                score += 100;
                Logger::Log("Your Score is now: " + std::to_string(score));

                std::ofstream myfile("score.html", std::ios_base::app | std::ios_base::out);
                if (myfile.is_open())
                {
                    myfile << std::to_string(score);
                    myfile << " \n";
                    myfile.close();


                }
                else
                    Logger::Error("Unable to open file!");

             //loader.LoadLevel(lua, registry, assetManager, renderer, 1);
            }

            //Destroy projectile
            projectile.Kill();
        }
    }

    void Update() {
        // TODO:...
    }
};

#endif // !DAMAGESYSTEM_H

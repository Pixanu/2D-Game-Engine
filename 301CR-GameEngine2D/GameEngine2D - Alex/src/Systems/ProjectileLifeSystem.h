#ifndef PROJECTILELIFESYSTEM_H
#define PROJECTILELIFESYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/ProjectileComponent.h"

class ProjectileLifeSystem : public System {

public:
	ProjectileLifeSystem() {
		RequireComponent<ProjectileComponent>();
	}

	void Update() {
		for (auto entity : GetSystemEntities()) {
			auto projectile = entity.GetComponent<ProjectileComponent>();

			//Kill projectile after they reach their duration limit
			if (SDL_GetTicks() - projectile.startTime > projectile.duration) {
				entity.Kill();
			}
		}
	}
};

#endif // !PROJECTILELIFESYSTEM_H

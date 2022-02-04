#ifndef LEVELLOADER_H
#define LEVELLOADER_H

#include "../ECS/ECS.h"
#include"../AssetManager/AssetManager.h"
#include <SDL.h>
#include <sol/sol.hpp>
#include <memory>

class LevelLoader {

public:
	LevelLoader();
	~LevelLoader();

	void LoadLevel(sol::state& lua, const std::unique_ptr<Registry>& registry, const std::unique_ptr<AssetManager>& assetManager, SDL_Renderer* renderer, int  level);
};

#endif // !LEVELLOADER_H



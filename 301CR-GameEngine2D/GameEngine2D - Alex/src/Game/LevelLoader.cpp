#include "./LevelLoader.h"
#include "./Game.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/AnimationComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/KeyboardControlledComponent.h"
#include "../Components/CameraFollowComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/TextLabelComponent.h"
//#include "../Components/NetworkComponent.h"
#include <fstream>

LevelLoader::LevelLoader() {
	Logger::Log("LevelLoader contructor called ");
}

LevelLoader::~LevelLoader() {
	Logger::Log("LevelLoader destructor called ");
}

void LevelLoader::LoadLevel(sol::state& lua, const std::unique_ptr<Registry>& registry, const std::unique_ptr<AssetManager>& assetManager, SDL_Renderer* renderer, int  levelNumber) {
	
	sol::load_result script = lua.load_file("./assets/scripts/Level" + std::to_string(levelNumber) + ".lua");
	
	//Syntax checker for the script, not executting the script

	if (!script.valid()) {
		sol::error err = script;
		std::string errorMessage = err.what();
		Logger::Error("Error loading the lua script: " + errorMessage);
		return;
	}
	
	//Executes the script usign the Sol State
	//Load the entities and components from luascript level1.lua
	lua.script_file("./assets/scripts/Level" + std::to_string(levelNumber) + ".lua");

	//Read the table for the current table 
	sol::table level = lua["Level"];

	///////////////////////////////////////////////////
	// Read the Level assets
	///////////////////////////////////////////////////

	sol::table assets = level["assets"];

	int i = 0;
	while (true) {
		sol::optional<sol::table> hasAsset = assets[i];
		if (hasAsset == sol::nullopt) {
			break;
		}
		sol::table asset = assets[i];
		std::string assetType = asset["type"];
		std::string assetId = asset["id"];

		if (assetType == "texture") {
			assetManager->AddTextures(renderer, assetId, asset["file"]);
			
			Logger::Log("New textures were loaded to the asset store, id: " + assetId);
		}
		if (assetType == "font") {
			assetManager->AddFont(assetId,asset["file"],asset["font_size"]);
			
			Logger::Log("New fonts were loaded to the asset store, id: " + assetId);
		}
		i++;
	}


	///////////////////////////////////////////////////
	// Read the tilemap info
	///////////////////////////////////////////////////
	sol::table map = level["tilemap"];
	std::string mapFilePath = map["map_file"];
	std::string mapTextureId = map["texture_asset_id"];
	int mapNumRows = map["num_rows"];
	int mapNumCols = map["num_cols"];
	int tileSize = map["tile_size"];
	double mapScale = map["scale"];

	std::fstream mapFile;
	mapFile.open(mapFilePath);
	for (int y = 0; y < mapNumRows; y++) {
		for (int x = 0; x < mapNumCols; x++) {
			char ch;
			mapFile.get(ch);
			int srcRectY = std::atoi(&ch) * tileSize;
			mapFile.get(ch);
			int srcRectX = std::atoi(&ch) * tileSize;
			mapFile.ignore();

			Entity tile = registry->CreateEntity();
			tile.Group("tiles");
			tile.AddComponent<TransformComponent>(glm::vec2(x * (mapScale * tileSize), y * (mapScale * tileSize)), glm::vec2(mapScale, mapScale), 0.0);
			tile.AddComponent<SpriteComponent>(mapTextureId, tileSize, tileSize, 0, false, srcRectX, srcRectY);
		}
	}
	mapFile.close();
	Game::mapWidth = mapNumCols * tileSize * mapScale;
	Game::mapHeight = mapNumRows * tileSize *mapScale;


	///////////////////////////////////////////////////
	// Read the entities and Components
	///////////////////////////////////////////////////
	sol::table entities = level["entities"];
	i = 0;
	while (true) {
		sol::optional<sol::table> hasEntity = entities[i];
		if (hasEntity == sol::nullopt) {
			break;
		}

		sol::table entity = entities[i];

		Entity newEntity = registry->CreateEntity();

		//Tag
		sol::optional<std::string> tag = entity["tag"];
		if (tag != sol::nullopt) {
			newEntity.Tag(entity["tag"]);
		}

		//Group
		sol::optional<std::string> group = entity["group"];
		if (group != sol::nullopt) {
			newEntity.Group(entity["group"]);
		}
		
		//Components
		sol::optional<sol::table> hasComponents = entity["components"];
		if (hasComponents != sol::nullopt) {
			//Transform
			sol::optional<sol::table> transform = entity["components"]["transform"];
			if (transform != sol::nullopt) {
				newEntity.AddComponent<TransformComponent>(
					glm::vec2(
						entity["components"]["transform"]["position"]["x"],
						entity["components"]["transform"]["position"]["y"]
					),
					glm::vec2(
						entity["components"]["transform"]["scale"]["x"].get_or(1.0),
						entity["components"]["transform"]["scale"]["y"].get_or(1.0)
					),
					entity["components"]["transform"]["rotation"].get_or(0.0)
					);
			}

			//RigidBody
			sol::optional<sol::table> rigidbody = entity["components"]["rigidbody"];
			if (rigidbody != sol::nullopt) {
				newEntity.AddComponent<RigidBodyComponent>(
					glm::vec2(
						entity["components"]["rigidbody"]["velocity"]["x"].get_or(0.0),
						entity["components"]["rigidbody"]["velocity"]["y"].get_or(0.0)
					)
				);
			}

			//Sprite
			sol::optional<sol::table> sprite = entity["components"]["sprite"];
			if (sprite != sol::nullopt) {
				newEntity.AddComponent<SpriteComponent>(
					entity["components"]["sprite"]["texture_asset_id"],
					entity["components"]["sprite"]["width"],
					entity["components"]["sprite"]["height"],
					entity["components"]["sprite"]["z_index"].get_or(1),
					entity["components"]["sprite"]["fixed"].get_or(false),
					entity["components"]["sprite"]["src_rect_x"].get_or(0),
					entity["components"]["sprite"]["src_rect_y"].get_or(0)
					);
			}

			//Animation
			sol::optional<sol::table> animation = entity["components"]["animation"];
			if (animation != sol::nullopt) {
				newEntity.AddComponent<AnimationComponent>(
					entity["components"]["animation"]["num_frames"].get_or(1),
					entity["components"]["animation"]["speed_rate"].get_or(1)
					);
			}

			//BoxCollider
			sol::optional<sol::table> collider = entity["components"]["boxcollider"];
			if (collider != sol::nullopt) {
				newEntity.AddComponent<BoxColliderComponent>(
					entity["components"]["boxcollider"]["width"],
					entity["components"]["boxcollider"]["height"],
					glm::vec2(
						entity["components"]["boxcollider"]["offset"]["x"].get_or(0),
						entity["components"]["boxcollider"]["offset"]["y"].get_or(0)
					)
					);
			}

			//Health
			sol::optional<sol::table> health = entity["components"]["health"];
			if (health != sol::nullopt) {
				newEntity.AddComponent<HealthComponent>(
					static_cast<int>(entity["components"]["health"]["health_percentage"].get_or(100))
					);
			}

			//ProjectileEmitter
			sol::optional<sol::table> projectileEmitter = entity["components"]["projectile_emitter"];
			if (projectileEmitter != sol::nullopt) {
				newEntity.AddComponent<ProjectileEmitterComponent>(
					glm::vec2(
						entity["components"]["projectile_emitter"]["projectile_velocity"]["x"],
						entity["components"]["projectile_emitter"]["projectile_velocity"]["y"]
					),
					static_cast<int>(entity["components"]["projectile_emitter"]["repeat_frequency"].get_or(1)) * 1000,
					static_cast<int>(entity["components"]["projectile_emitter"]["projectile_duration"].get_or(10)) * 1000,
					static_cast<int>(entity["components"]["projectile_emitter"]["hit_percentage_damage"].get_or(10)),
					entity["components"]["projectile_emitter"]["friendly"].get_or(false)
					);
			}

			//CameraFollow
			sol::optional<sol::table> cameraFollow = entity["components"]["camera_follow"];
			if (cameraFollow != sol::nullopt) {
				newEntity.AddComponent<CameraFollowComponent>();
			}

			//KeyboardControlled
			sol::optional<sol::table> keyboardControlled = entity["components"]["keyboard_controller"];
			if (keyboardControlled != sol::nullopt) {
				newEntity.AddComponent<KeyboardControlledComponent>(
					glm::vec2(
						entity["components"]["keyboard_controller"]["up_velocity"]["x"],
						entity["components"]["keyboard_controller"]["up_velocity"]["y"]
					),
					glm::vec2(
						entity["components"]["keyboard_controller"]["right_velocity"]["x"],
						entity["components"]["keyboard_controller"]["right_velocity"]["y"]
					),
					glm::vec2(
						entity["components"]["keyboard_controller"]["down_velocity"]["x"],
						entity["components"]["keyboard_controller"]["down_velocity"]["y"]
					),
					glm::vec2(
						entity["components"]["keyboard_controller"]["left_velocity"]["x"],
						entity["components"]["keyboard_controller"]["left_velocity"]["y"]
					)
					);
			}
		}
		i++;
	}



	//Add Assets to the asset manager
	/*
	assetManager->AddTextures(renderer, "car", "./assets/images/truck-ford-right.png");
	assetManager->AddTextures(renderer, "helicopter", "./assets/images/chopper-spritesheet.png");
	assetManager->AddTextures(renderer, "tilemap", "./assets/tilemaps/jungle.png");
	assetManager->AddTextures(renderer, "tank", "./assets/images/tank-panther-right.png");
	assetManager->AddTextures(renderer, "bullet-image", "./assets/images/bullet.png");
	assetManager->AddTextures(renderer, "tree", "./assets/images/tree.png");

	assetManager->AddFont("charriot-font", "./assets/fonts/charriot.ttf", 14);

	//Load tile map
	int tileSize = 32;
	double tileScale = 2.0;
	int mapNumCols = 25;
	int mapNumRows = 20;
	std::fstream mapFile;
	mapFile.open("./assets/tilemaps/jungle.map");

	for (int y = 0; y < mapNumRows; y++) {
		for (int x = 0; x < mapNumCols; x++) {
			char ch;
			mapFile.get(ch);
			int srcRectY = std::atoi(&ch) * tileSize;
			mapFile.get(ch);
			int srcRectX = std::atoi(&ch) * tileSize;
			mapFile.ignore();

			Entity tile = registry->CreateEntity();
			tile.Group("tiles");
			tile.AddComponent<TransformComponent>(glm::vec2(x * (tileScale * tileSize), y * (tileScale * tileSize)), glm::vec2(tileScale, tileScale), 0.0);
			tile.AddComponent<SpriteComponent>("tilemap", tileSize, tileSize, 0, false, srcRectX, srcRectY);
		}
	}
	mapFile.close();
	Game::mapWidth = mapNumCols * tileSize * tileScale;
	Game::mapHeight = mapNumRows * tileSize * tileScale;


	//Create an entity

	Entity helicopter = registry->CreateEntity();
	helicopter.Tag("player");
	helicopter.AddComponent<TransformComponent>(glm::vec2(10.0, 10.0), glm::vec2(1.0, 1.0), 0.0);
	helicopter.AddComponent<RigidBodyComponent>(glm::vec2(40.0, 0.0));
	helicopter.AddComponent<SpriteComponent>("helicopter", 32, 32, 1);
	helicopter.AddComponent<AnimationComponent>(2, 15, true);
	helicopter.AddComponent<BoxColliderComponent>(32, 32, glm::vec2(0, 5));
	helicopter.AddComponent<ProjectileEmitterComponent>(glm::vec2(150.0, 150.0), 0, 10000, 10, true);
	helicopter.AddComponent<KeyboardControlledComponent>(glm::vec2(0, -80), glm::vec2(80, 0), glm::vec2(0, 80), glm::vec2(-80, 0));
	helicopter.AddComponent<CameraFollowComponent>();
	helicopter.AddComponent<HealthComponent>(100);

	Entity tree = registry->CreateEntity();
	tree.Group("obstacles");
	tree.AddComponent<TransformComponent>(glm::vec2(600.0, 500.0), glm::vec2(1.0, 1.0), 0.0);
	tree.AddComponent<SpriteComponent>("tree", 16, 32, 1);
	tree.AddComponent<BoxColliderComponent>(16, 32);


	Entity car = registry->CreateEntity();
	car.Group("enemies");
	car.AddComponent<TransformComponent>(glm::vec2(120.0, 500.0), glm::vec2(1.0, 1.0), 0.0);
	car.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
	car.AddComponent<SpriteComponent>("car", 32, 32, 1);
	car.AddComponent<BoxColliderComponent>(32, 32, glm::vec2(5, 5));
	car.AddComponent<ProjectileEmitterComponent>(glm::vec2(100.0, 0.0), 5000, 10000, 10, false);
	car.AddComponent<HealthComponent>(100);

	Entity tank = registry->CreateEntity();
	tank.Group("enemies");
	tank.AddComponent<TransformComponent>(glm::vec2(500.0, 500.0), glm::vec2(1.0, 1.0), 0.0);
	tank.AddComponent<RigidBodyComponent>(glm::vec2(20.0, 0.0));
	tank.AddComponent<SpriteComponent>("tank", 32, 32, 1);
	tank.AddComponent<BoxColliderComponent>(32, 32, glm::vec2(5, 5));
	//tank.AddComponent<ProjectileEmitterComponent>(glm::vec2(100.0, 0.0), 5000, 10000, 10, false);
	tank.AddComponent<HealthComponent>(100);


	Entity label = registry->CreateEntity();
	SDL_Color white = { 255,255,255 };
	label.AddComponent<TextLabelComponent>(glm::vec2(100, 100), "Game Engine - Ivan Alexandru - Test Game", "charriot-font", white, true);
	*/

}
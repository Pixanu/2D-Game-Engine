#include "Game.h"
#include "../Game/LevelLoader.h"
#include "../ECS/ECS.h"
#include "../Systems/MovementSystem.h"
#include "../Systems/CameraMovementSystem.h"
#include "../Systems/RenderSystem.h"
#include "../Systems/AnimationSystem.h"
#include "../Systems/CollisionSystem.h"
#include "../Systems/RenderColliderSystem.h"
#include "../Systems/DamageSystem.h"
#include "../Systems/ProjectileEmitSystem.h"
#include "../Systems/KeyboardMovementSystem.h"
#include "../Systems/ProjectileLifeSystem.h"
#include "../Systems/RenderTextSystem.h"
#include "../Systems/RenderHealthBarSystem.h"
#include "../Systems/RenderGUISystem.h"
//#include "../Systems/NetworkSystem.h"

#include <SDL.h>
#include <SDL_image.h>
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_sdl.h>
#include <imgui/imgui_impl_sdl.h>


int Game::windowWidth;
int Game::windowHeight;
int Game::mapWidth;
int Game::mapHeight;

Game::Game()
{
	isRunning = false;
	isDebug = false;
	registry = std::make_unique<Registry>();
	assetManager = std::make_unique<AssetManager>();
	eventBus = std::make_unique<EventBus>();
	Logger::Log("Game Constructor Called");
}


Game::~Game()
{
	Logger::Log("Game Destructor Called");
}


void Game::Initialize()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		Logger::Error("Error initializing SDL.");
		return;
	}

	if (TTF_Init() != 0) {
		Logger::Error("Error initializing SDL TTF.");
		return;
	}

	SDL_DisplayMode displayMode;
	SDL_GetCurrentDisplayMode(0, &displayMode);
	windowHeight = displayMode.h;
	windowWidth = displayMode.w;
	window = SDL_CreateWindow(
		"GameEngine2D-Ivan Alexandru",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		windowWidth,
		windowHeight,
		SDL_WINDOW_BORDERLESS);

	if (!window)
	{
		Logger::Error("Error creating SDL window.");
		return;
	}
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer)
	{
		Logger::Error("Error creating SDL renderer.");
		return;
	}

	//Initialize theImGui context
	ImGui::CreateContext();
	ImGuiSDL::Initialize(renderer, windowWidth, windowHeight);


	//Not Needed
	//SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

	//Initialize the camera view with the entire screen area
	camera.x = 0;
	camera.y = 0;
	camera.w = windowWidth;
	camera.h = windowHeight;

	isRunning = true;
}

void Game::ProcessInput()
{
	SDL_Event sdlEvent;
	while (SDL_PollEvent(&sdlEvent))
	{
		//ImGui SDL input
		ImGui_ImplSDL2_ProcessEvent(&sdlEvent);
		ImGuiIO& io = ImGui::GetIO();

		int mouseX, mouseY;
		const int buttons = SDL_GetMouseState(&mouseX, &mouseY);

		io.MousePos = ImVec2(mouseX, mouseY);
		io.MouseDown[0] = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
		io.MouseDown[1] = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);


		//Handle core SDL events( close window, key pressed,etc.)
		switch (sdlEvent.type)
		{
		case SDL_QUIT:
			isRunning = false;
			break;
		case SDL_KEYDOWN:
			if (sdlEvent.key.keysym.sym == SDLK_ESCAPE)
			{
				isRunning = false;
			}
			if (sdlEvent.key.keysym.sym == SDLK_F10){
				isDebug = !isDebug;
			}
			eventBus->EmitEvent<KeyPressedEvent>(sdlEvent.key.keysym.sym);
			break;

		}

	}
}

void Game::Setup()
{
	//Add systems that need to be processed
	registry->AddSystem<MovementSystem>();
	registry->AddSystem<RenderSystem>();
	registry->AddSystem<AnimationSystem>();
	registry->AddSystem<CollisionSystem>();
	registry->AddSystem<RenderColliderSystem>();
	registry->AddSystem<DamageSystem>();
	registry->AddSystem<KeyboardMovementSystem>();
	registry->AddSystem<CameraMovementSystem>();
	registry->AddSystem<ProjectileEmitSystem>();
	registry->AddSystem<ProjectileLifeSystem>();
	registry->AddSystem<RenderTextSystem>();
	registry->AddSystem<RenderHealthBarSystem>();
	registry->AddSystem<RenderGUISystem>();
	//registry->AddSystem<NetworkSystem>();
	
	//Load the level
	LevelLoader loader;
	lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::os);

	loader.LoadLevel(lua,registry,assetManager,renderer, 1);
}


void Game::Update()
{
	int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - millisecsPrevFrame);
	if (timeToWait > 0 && timeToWait <= MILLISECS_PER_FRAME)
	{
		SDL_Delay(timeToWait);
	}

	//Difference in ticks since last frame which are converted into seconds
	double deltaTime = (SDL_GetTicks() - millisecsPrevFrame) / 1000.0;

	//Store the current frame time
	millisecsPrevFrame = SDL_GetTicks();

	//Reset all event handlers for the current frames
	eventBus->Reset();

	//Perform the subscription of the events for all systems
	registry->GetSystem<MovementSystem>().SubscribeToEvents(eventBus);
	registry->GetSystem<DamageSystem>().SubscribeToEvents(eventBus);
	registry->GetSystem<KeyboardMovementSystem>().SubscribeToEvents(eventBus);
	registry->GetSystem<ProjectileEmitSystem>().SubscribeToEvents(eventBus);

	//Update the registry to process the entities that are waiting to either be creater or deleted
	registry->Update();


	//Ask all systems to update
	registry->GetSystem<MovementSystem>().Update(deltaTime);
	registry->GetSystem<AnimationSystem>().Update();
	registry->GetSystem<CollisionSystem>().Update(eventBus);
	registry->GetSystem<ProjectileEmitSystem>().Update(registry);
	registry->GetSystem<CameraMovementSystem>().Update(camera);
	registry->GetSystem<ProjectileLifeSystem>().Update();
	//registry->GetSystem<NetworkSystem>().Update();

}

void Game::Render()
{
	SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
	SDL_RenderClear(renderer);

	//Invoke all the systems that need to render 
	registry->GetSystem<RenderSystem>().Update(renderer, assetManager, camera);
	registry->GetSystem<RenderTextSystem>().Update(renderer, assetManager, camera);
	registry->GetSystem<RenderHealthBarSystem>().Update(renderer, assetManager, camera);

	if (isDebug) {
		registry->GetSystem<RenderColliderSystem>().Update(renderer, camera);
		registry->GetSystem<RenderGUISystem>().Update(registry, camera);
	}

	SDL_RenderPresent(renderer);
}

void Game::Run()
{
	Setup();
	while (isRunning)
	{
		ProcessInput();
		Update();
		Render();
	}
}

void Game::Destroy()
{
	ImGuiSDL::Deinitialize();
	ImGui::DestroyContext();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
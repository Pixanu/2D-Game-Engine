# 301CR - GameEngine2D - Ivan Alexandru

Game Engine created using an ECS System:

-SDL2 - library for rendering and handling input.
-GLM - a math helper library.
-DearImGui - library to create UI for engine tooling.
-Sol - binding library for C++ & Lua.
-Lua - fast and easy to use scripting language.
-Enet - network communication layer.


How to run the Engine through Visual Studio: 

- Step 1: Open GameEngine2D - Alex.vcxproj 

- Step 2: Make sure the project is set to Debug/Release x86

- Step 3: Run


How to run the Server through Visual Studio: 

- Step 1: Open Server.vcxproj

- Step 2: Make sure the project is set to Debug x64

- Step 3: Run


Before Running: The server is running but not working with the engine, only works in Debug Mode x64, based on the class example.
The current problem is that is not working/linking with the NetworkSystem inside the engine which has been commented out.

Controls for the game:

Movement: Arrow Keys

Attack: SPACE 

Developer Mode: F10 (The user is able to spawn other enemies on the map with the help of ImGui)


The score is being stored inside a .html file either insider GameEngine2D-Alex when run thorough Visual Studio,
or inside GameEngine2D-Alex/Release when the is run thorough the executable.

To change to another level recompilation of the code is necessary: 

Go inside the Visual Studio Project - Game.cpp - Void::Game::Setup() and look for:

"loader.LoadLevel(lua,registry,assetManager,renderer, 1)" and change it from "1" to "2" to load the second level.

IN CASE OF FATAL ERROR WHEN STARTING THE PROGRAM FROM VISUAL STUDIO:
On my machine works but in case of Fatal Erorr occurring when running one possible fix is:

Possible Error:
Properties - C/C++ - Language - Conformance Mode - Should be set to "No(/permissive)" - solution found when testing on University Pc's

Other Possible Error:
Properties - Linker - General - Enable Incremental Linking - Has been set to "YES(/INCREMENTAL)" - in case of an error should be set to "NO(/INCREMENTAL:NO)"

Project powered by PIKUMA.com



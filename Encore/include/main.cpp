//
// Created by marie on 08/07/2024.
//

#define SOL_ALL_SAFETIES_ON 1
#define SOL_USE_LUA_HPP 1
#include <sol/sol.hpp>
#include <raylib.h>

int main(int argc, char *argv[]) {
	sol::state lua;

	lua.open_libraries(sol::lib::base);


	InitWindow(800,450, "encore");
	SetTargetFPS(60);
#ifdef _DEBUG
	lua.script_file("scripts/testing.lua");

#else
	lua.script_file("scripts/testing.lua");
#endif


	std::string scriptString = "";
	while (!WindowShouldClose()) {
		if (IsKeyPressed(KEY_F5)) {
			lua.script_file("scripts/testing.lua");
		}
		BeginDrawing();

		ClearBackground(RAYWHITE);


		scriptString = lua["ScriptString"];
		DrawText("Encore is running!", 190, 200, 20, LIGHTGRAY);
		DrawText(scriptString.c_str(), 190, 225, 20, LIGHTGRAY);

		EndDrawing();
	}

	CloseWindow();
	return 0;
}


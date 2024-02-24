#include "rapidjson/document.h"
#include "raylib.h"
#include <vector>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include "song/song.h"
#include "audio/audio.h"
int main(int argc, char* argv[])
{
#ifdef NDEBUG
	ShowWindow(GetConsoleWindow(), 0);
#endif
	InitWindow(800, 600, "FNFestClone");
	InitAudioDevice();
	SetTargetFPS(60);

	Camera3D camera = { 0 };
	camera.position = Vector3{ 0.0f, 10.0f, 10.0f };
	camera.target = Vector3{ 0.0f, 0.0f, 0.0f };
	camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
	camera.fovy = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;
	Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };

	//TESTING LOADING FILES
	std::filesystem::path executablePath(GetApplicationDirectory());
	std::filesystem::path directory = executablePath.parent_path();
	std::filesystem::path lovedontdiepath = directory / "Songs/The Fray - Love Don't Die";
	Song lovedontdie;
	lovedontdie.LoadSong(lovedontdiepath / "info.json");
	std::vector<Music> loadedStreams = LoadStems(lovedontdie.stemsPath);
	
	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(RAYWHITE);
		DrawTextureEx(lovedontdie.albumArt, Vector2{ 200,200 }, 0.0f, 0.35f, RAYWHITE);
		DrawText(lovedontdie.title.c_str(),400,200,20,BLACK);
		DrawText(lovedontdie.artist.c_str(), 400, 230, 20, BLACK);
		for (Music& stream : loadedStreams) {
			UpdateMusicStream(stream);
			PlayMusicStream(stream);
		}
		BeginMode3D(camera);

		EndMode3D();

		DrawFPS(10, 10);

		EndDrawing();
	}
	CloseWindow();
	return 0;
}

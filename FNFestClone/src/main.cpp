#include "rapidjson/document.h"
#include "raylib.h"
#include <vector>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include "song/song.h"
#include "song/songlist.h"
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
	std::filesystem::path songsPath = directory / "Songs";
	SongList songList = LoadSongs(songsPath);
	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(RAYWHITE);
		float curSong = 0.0f;
		for (Song song : songList) {
			DrawTextureEx(song.albumArt, Vector2{ 0,50*curSong }, 0.0f, 0.1f, RAYWHITE);
			DrawText(song.title.c_str(), 50, 50 * curSong, 20, BLACK);
			DrawText(song.artist.c_str(), 50, (50*curSong)+20, 16, BLACK);
			curSong++;
		}
		BeginMode3D(camera);

		EndMode3D();

		EndDrawing();
	}
	CloseWindow();
	return 0;
}

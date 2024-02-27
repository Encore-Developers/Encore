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
#include "game/arguments.h"
#include "game/utility.h"
#include "raygui.h"
#include <stdlib.h>

vector<std::string> ArgumentList::arguments;

bool compareNotes(const Note& a, const Note& b) {
	return a.time < b.time;
}
int main(int argc, char* argv[])
{
#ifdef NDEBUG
	ShowWindow(GetConsoleWindow(), 0);
#endif

	InitWindow(800, 600, "Encore");

	ArgumentList::InitArguments(argc, argv);

	std::string FPSCapStringVal = ArgumentList::GetArgValue("fpscap");
	int targetFPSArg = 0;

	if (FPSCapStringVal != "")
	{
		assert(str2int(&targetFPSArg, FPSCapStringVal.c_str()) == STR2INT_SUCCESS);
		TraceLog(LOG_INFO, "Argument overridden target FPS: %d", targetFPSArg);
	}

	//https://www.raylib.com/examples/core/loader.html?name=core_custom_frame_control

	double previousTime = GetTime();
	double currentTime = 0.0;
	double updateDrawTime = 0.0;
	double waitTime = 0.0;
	float deltaTime = 0.0f;

	float timeCounter = 0.0f;
	
	int targetFPS = targetFPSArg == 0 ? 60 : targetFPSArg;

	TraceLog(LOG_INFO, "Target FPS: %d", targetFPS);

	InitAudioDevice();

	Camera3D camera = { 0 };

	camera.position = Vector3{ 0.0f, 5.0f, -3.5f };
	camera.target = Vector3{ 0.0f, 0.0f, 6.5f };
	camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
	camera.fovy = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;

	std::filesystem::path executablePath(GetApplicationDirectory());

	std::filesystem::path directory = executablePath.parent_path();

	std::filesystem::path songsPath = directory / "Songs";

	SongList songList = LoadSongs(songsPath);
	bool midiLoaded = false;
	bool isPlaying = false;
	bool streamsLoaded = false;
	std::vector<Music> loadedStreams;
	int curPlayingSong = 0;
	int curNoteIdx = 0;
	int curODPhrase = 0;
	Model noteModel = LoadModel((directory / "Assets/note.obj").string().c_str());
	Texture2D noteTexture = LoadTexture((directory / "Assets/note_d.png").string().c_str());
	Texture2D emitTexture = LoadTexture((directory / "Assets/note_e.png").string().c_str());
	noteModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = noteTexture;
	noteModel.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
	noteModel.materials[0].maps[MATERIAL_MAP_EMISSION].texture = emitTexture;
	noteModel.materials[0].maps[MATERIAL_MAP_EMISSION].color = WHITE;
	Model noteModelOD = LoadModel((directory / "Assets/note.obj").string().c_str());
	Texture2D noteTextureOD = LoadTexture((directory / "Assets/note_od_d.png").string().c_str());
	noteModelOD.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = noteTextureOD;
	noteModelOD.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
	noteModelOD.materials[0].maps[MATERIAL_MAP_EMISSION].texture = emitTexture;
	noteModelOD.materials[0].maps[MATERIAL_MAP_EMISSION].color = WHITE;
	Model liftModel = LoadModel((directory / "Assets/lift.obj").string().c_str());
	liftModel.materials[0].maps[MATERIAL_MAP_ALBEDO].color = Color{ 172,82,217,127 };
	Model liftModelOD = LoadModel((directory / "Assets/lift.obj").string().c_str());
	liftModelOD.materials[0].maps[MATERIAL_MAP_ALBEDO].color = Color{ 217, 183, 82 ,127 };
	while (!WindowShouldClose())
	{
		BeginDrawing();

		ClearBackground(DARKGRAY);
		
		
		if (!isPlaying) {
			float curSong = 0.0f;
			for (Song song : songList.songs) {
				if (GuiButton({ 0,0 + (60 * curSong),300,60 }, "")) {
					std::cout << "Clicked song: '" << song.artist << " - " << song.title << "'" << std::endl;
					curPlayingSong = (int)curSong;
					isPlaying = true;
				}
				DrawTextureEx(song.albumArt, Vector2{ 5,(60 * curSong) + 5 }, 0.0f, 0.1f, RAYWHITE);
				DrawText(song.title.c_str(), 60, (60 * curSong) + 5, 20, BLACK);
				DrawText(song.artist.c_str(), 60, (60 * curSong) + 25, 16, BLACK);
				curSong++;
			}
		}
		else {
			if (!streamsLoaded) {
				loadedStreams = LoadStems(songList.songs[curPlayingSong].stemsPath);
				streamsLoaded = true;
			}
			else {
				for (Music& stream : loadedStreams) {
					UpdateMusicStream(stream);
					PlayMusicStream(stream);
				}
			}
			if (!midiLoaded) {
				smf::MidiFile midiFile;
				midiFile.read(songList.songs[curPlayingSong].midiPath.string());
				std::cout << midiFile.linkNotePairs() << std::endl;
				for (int i = 0; i < midiFile.getTrackCount(); i++)
				{
					std::string trackName = "";
					for (int j = 0; j < midiFile[i].getSize(); j++) {
						if (midiFile[i][j].isMeta()) {
							if ((int)midiFile[i][j][1] == 3) {
								for (int k = 3; k < midiFile[i][j].getSize(); k++) {
									trackName += midiFile[i][j][k];
								}
								SongParts songPart = partFromString(trackName);
								std::cout << "TRACKNAME " << trackName << ": " << int(songPart) << std::endl;
								if (songPart != SongParts::Invalid) {
									songList.songs[curPlayingSong].parts[(int)songPart]->hasPart = true;
									std::string diffstr = "ESY: ";
									for (int diff = 0; diff < 4; diff++) {
										Chart newChart;
										newChart.parseNotes(midiFile, i, midiFile[i], diff);
										std::sort(newChart.notes.begin(), newChart.notes.end(), compareNotes);
										if (diff == 1) diffstr = "MED: ";
										else if (diff == 2) diffstr = "HRD: ";
										else if (diff == 3) diffstr = "EXP: ";
										songList.songs[curPlayingSong].parts[(int)songPart]->charts.push_back(newChart);
										std::cout << trackName << " " << diffstr << newChart.notes.size() << std::endl;
									}
								}
							}
						}
					}
				}
				midiLoaded = true;
			}
			else {
				BeginMode3D(camera);
				DrawTriangle3D(Vector3{ 2.0f,0.0f,0.0f }, Vector3{ -2.0f,0.0f,0.0f }, Vector3{ -2.0f,0.0f,20.0f }, BLACK);
				DrawTriangle3D(Vector3{ 2.0f,0.0f,0.0f }, Vector3{ -2.0f,0.0f,20.0f }, Vector3{ 2.0f,0.0f,20.0f }, BLACK);
				double musicTime = (double)GetMusicTimePlayed(loadedStreams[0]);
				Chart& dmsExpert = songList.songs[curPlayingSong].parts[2]->charts[3];
				for (int i = curNoteIdx; i < dmsExpert.notes.size(); i++) {
					Note& curNote = dmsExpert.notes[i];
					double relTime = curNote.time - musicTime;
					double relEnd = (curNote.time+curNote.len) - musicTime;
					if (relTime > 4.0) {
						break;
					}
					else {
						if (curNote.lift == true) {
							DrawModel(liftModel, Vector3{ 1.6f - (0.8f * curNote.lane),0.125f,2.5f + (12.5f * (float)relTime) },1.0f, WHITE);
						}
						else {
							if (curNote.len > 0.125) {
								DrawLine3D(Vector3{ 1.6f - (0.8f * curNote.lane),0.05f,2.5f + (12.5f * (float)relTime) }, Vector3{ 1.6f - (0.8f * curNote.lane),0.05f,2.5f + (12.5f * (float)relEnd) }, Color{ 172,82,217,255 });
							}
							DrawModel(noteModel, Vector3{ 1.6f - (0.8f * curNote.lane),0.125f,2.5f + (12.5f * (float)relTime) }, 1.0f, WHITE);
							
						}
					}

					if (relTime < 0.0 && curNoteIdx < dmsExpert.notes.size()) curNoteIdx = i + 1;

				}
				EndMode3D();
			}
			DrawText(songList.songs[curPlayingSong].title.c_str(), 5, 5, 30, WHITE);
			DrawText(songList.songs[curPlayingSong].artist.c_str(), 5, 40, 24, WHITE);
		}
		

		EndDrawing();

		//SwapScreenBuffer();

		currentTime = GetTime();
		updateDrawTime = currentTime - previousTime;

		if (targetFPS > 0)
		{
			waitTime = (1.0f / (float)targetFPS) - updateDrawTime;
			if (waitTime > 0.0)
			{
				WaitTime((float)waitTime);
				currentTime = GetTime();
				deltaTime = (float)(currentTime - previousTime);
			}
		}
		else deltaTime = (float)updateDrawTime;

		previousTime = currentTime;
	}
	CloseWindow();
	return 0;
}

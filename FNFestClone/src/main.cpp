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
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	// 800 , 600
	InitWindow(1600, 900, "Encore");
	//ToggleBorderlessWindowed();
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

	bool expert = true;
	float diffDistance = expert ? 2.0f : 1.5f;
	float lineDistance = expert ? 1.5f : 1.0f;

	float timeCounter = 0.0f;

	int targetFPS = targetFPSArg == 0 ? 60 : targetFPSArg;

	TraceLog(LOG_INFO, "Target FPS: %d", targetFPS);

	InitAudioDevice();

	Camera3D camera = { 0 };

	// Y UP!!!! REMEMBER!!!!!!
	//							  x,    y,     z
							// 0.0f, 5.0f, -3.5f
	camera.position = Vector3{ 0.0f, 7.0f, -12.0f };
	// 0.0f, 0.0f, 6.5f
	camera.target = Vector3{ 0.0f, 0.0f, 13.6f };

	camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
	camera.fovy = 34.5f;
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
	Model expertHighway = LoadModel((directory / "Assets/expert.obj").string().c_str());
	Texture2D highwayTexture = LoadTexture((directory / "Assets/highway.png").string().c_str());
	expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTexture;
	expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
	// expertHighway.materials[1].maps[MATERIAL_MAP_ALBEDO].texture = sidesTexture;
	// expertHighway.materials[1].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
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

	std::vector<double> laneTimes = { 0.0,0.0,0.0,0.0,0.0 };
	std::vector<double> liftTimes = { 0.0,0.0,0.0,0.0,0.0 };
	std::vector<bool> heldFrets = { false,false,false,false,false };
	while (!WindowShouldClose())
	{
		if (isPlaying) {
			if (IsKeyPressed(KEY_A)) {
				laneTimes[0] = (double)GetMusicTimePlayed(loadedStreams[0]);
				liftTimes[0] = 0.0;
				heldFrets[0] = true;
			}
			if (IsKeyReleased(KEY_A)) {
				laneTimes[0] = 0.0;
				liftTimes[0] = (double)GetMusicTimePlayed(loadedStreams[0]);
				heldFrets[0] = false;
			}
			if (IsKeyPressed(KEY_S)) {
				laneTimes[1] = (double)GetMusicTimePlayed(loadedStreams[0]);
				liftTimes[1] = 0.0;
				heldFrets[1] = true;
			}
			if (IsKeyReleased(KEY_S)) {
				laneTimes[1] = 0.0;
				liftTimes[1] = (double)GetMusicTimePlayed(loadedStreams[0]);
				heldFrets[1] = false;
			}
			if (IsKeyPressed(KEY_J)) {
				laneTimes[2] = (double)GetMusicTimePlayed(loadedStreams[0]);
				liftTimes[2] = 0.0;
				heldFrets[2] = true;
			}
			if (IsKeyReleased(KEY_J)) {
				laneTimes[2] = 0.0;
				liftTimes[2] = (double)GetMusicTimePlayed(loadedStreams[0]);
				heldFrets[2] = false;
			}
			if (IsKeyPressed(KEY_K)) {
				laneTimes[3] = (double)GetMusicTimePlayed(loadedStreams[0]);
				liftTimes[3] = 0.0;
				heldFrets[3] = true;
			}
			if (IsKeyReleased(KEY_K)) {
				laneTimes[3] = 0.0;
				liftTimes[3] = (double)GetMusicTimePlayed(loadedStreams[0]);
				heldFrets[3] = false;
			}
			if (IsKeyPressed(KEY_L)) {
				laneTimes[4] = (double)GetMusicTimePlayed(loadedStreams[0]);
				liftTimes[4] = 0.0;
				heldFrets[4] = true;
			}
			if (IsKeyReleased(KEY_L)) {
				laneTimes[4] = 0.0;
				liftTimes[4] = (double)GetMusicTimePlayed(loadedStreams[0]);
				heldFrets[4] = false;
			}
		}
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
				
				DrawText( song.title.c_str(), 60, (60 * curSong) + 5, 20, BLACK);
				DrawText( song.artist.c_str(),  60, (60 * curSong) + 25, 16, BLACK);
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
				DrawModel(expertHighway, Vector3{0,0,0}, 1.0f, WHITE);
				for (int i = 0; i < 5; i++) {
					if (heldFrets[i]==true) {
						DrawCube(Vector3{diffDistance - (1.0f * i),0,2.5f }, 0.75, 0.125, 0.25, Color{ 84,8,207,255 });
					}
					else {
						DrawCube(Vector3{diffDistance - (1.0f * i),0,2.5f }, 0.75, 0.125, 0.25, Color{ 35,21,69,255 });
					}
				}
				// DrawTriangle3D(Vector3{ 2.5f,0.0f,0.0f }, Vector3{ -2.5f,0.0f,0.0f }, Vector3{ -2.5f,0.0f,20.0f }, BLACK);
				// DrawTriangle3D(Vector3{ 2.5f,0.0f,0.0f }, Vector3{ -2.5f,0.0f,20.0f }, Vector3{ 2.5f,0.0f,20.0f }, BLACK);
				for (int i = 0; i < 4; i++) {
					DrawLine3D(Vector3{ lineDistance - i, 0.05f, 0 }, Vector3{ lineDistance - i, 0.05f, 20 }, Color{ 255,255,255,255 });
				}
				DrawLine3D(Vector3{ 2.5f, 0.05f, 2.0f }, Vector3{ -2.5f, 0.05f, 2.0f}, WHITE);
				double musicTime = (double)GetMusicTimePlayed(loadedStreams[0]);
				Chart& dmsExpert = songList.songs[curPlayingSong].parts[0]->charts[3];
				if (dmsExpert.odPhrases.size() > 0) {
					if (dmsExpert.notes[curNoteIdx].time+dmsExpert.notes[curNoteIdx].len > dmsExpert.odPhrases[curODPhrase].end && curODPhrase < dmsExpert.odPhrases.size()) curODPhrase++;
				}
				for (int i = curNoteIdx; i < dmsExpert.notes.size(); i++) {
					Note& curNote = dmsExpert.notes[i];
					if (curNote.lift == true) {
						if (curNote.time - 0.075 < liftTimes[curNote.lane] && curNote.time + 0.075 > laneTimes[curNote.lane]) {
							curNote.hit = true;
						}
					}
					else {
						if (curNote.time - 0.075 < laneTimes[curNote.lane] && curNote.time + 0.075 > laneTimes[curNote.lane]) {
							curNote.hit = true;
							if (curNote.len > 0.2) {
								curNote.held = true;
							}
						}
						if (laneTimes[curNote.lane] == 0.0 && curNote.len > 0.2) {
							curNote.held = false;
						}
					}
					
					double relTime = curNote.time - musicTime;
					double relEnd = (curNote.time + curNote.len) - musicTime;
					bool od = false;
					if (dmsExpert.odPhrases.size() > 0) {
						if (curNote.time >= dmsExpert.odPhrases[curODPhrase].start && curNote.time <= dmsExpert.odPhrases[curODPhrase].end) {
							od = true;
						}

					}
					if (relTime > 1.5) {
						break;
					}
					if (curNote.lift == true && !curNote.hit) {
						// lifts						//  distance between notes 
						//									(furthest left - lane distance)
						if (od == true)					//  1.6f	0.8
							DrawModel(liftModelOD, Vector3{ diffDistance - (1.0f * curNote.lane),0,2.5f + (12.5f * (float)relTime) }, 1.0f, WHITE);
						// energy phrase
						else
							DrawModel(liftModel, Vector3{ diffDistance - (1.0f * curNote.lane),0,2.5f + (12.5f * (float)relTime) }, 1.0f, WHITE);
						// regular 
					}
					else {
						// sustains
						if (curNote.len > 0.2) {
							if (curNote.hit == true && curNote.held == true) {
								if (curNote.heldTime < curNote.len) {
									curNote.heldTime = 0.0 - relTime;
									if (relTime < 0.0) relTime = 0.0;
								}
								if (relEnd <=0.0) {
									if (relTime < 0.0) relTime = relEnd;
									curNote.held = false;
								}
							}
							else if (curNote.hit == true && curNote.held == false) {
								relTime = relTime + curNote.heldTime;
							}
							if (od == true)
								DrawLine3D(Vector3{ diffDistance - (1.0f * curNote.lane),0.05f,2.5f + (12.5f * (float)relTime) }, Vector3{ diffDistance - (1.0f * curNote.lane),0.05f,2.5f + (12.5f * (float)relEnd) }, Color{ 217, 183, 82 ,255 });

							else
								DrawLine3D(Vector3{ diffDistance - (1.0f * curNote.lane),0.05f,2.5f + (12.5f * (float)relTime) }, Vector3{ diffDistance - (1.0f * curNote.lane),0.05f,2.5f + (12.5f * (float)relEnd) }, Color{ 172,82,217,255 });
						}
						// regular notes
						if ((curNote.len>=0.2 && (curNote.held==true||curNote.hit==false)) || (curNote.len<0.2 && curNote.hit==false)) {
							if (od == true)
								DrawModel(noteModelOD, Vector3{ diffDistance - (1.0f * curNote.lane),0,2.5f + (12.5f * (float)relTime) }, 1.0f, WHITE);
							else
								DrawModel(noteModel, Vector3{ diffDistance - (1.0f * curNote.lane),0,2.5f + (12.5f * (float)relTime) }, 1.0f, WHITE);
						}
					}
					if (relEnd < -1 && curNoteIdx < dmsExpert.notes.size()) curNoteIdx = i + 1;

				}
				EndMode3D();
			}
			DrawText(songList.songs[curPlayingSong].title.c_str(), 5, 5, 30, WHITE);
			DrawText(songList.songs[curPlayingSong].artist.c_str(), 5, 40, 24, WHITE);
			
			DrawFPS(5, 60);
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

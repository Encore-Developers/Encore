#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
#include "raylib.h"
#include <vector>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include "song/song.h"
#include "song/songlist.h"
#include "audio/audio.h"
#include "game/arguments.h"
#include "game/utility.h"
#include "game/player.h"
#include "game/keybinds.h"
#include "raygui.h"

#include <stdlib.h>
#include "GLFW/glfw3.h"
#include <cstdio>
vector<std::string> ArgumentList::arguments;

bool compareNotes(const Note& a, const Note& b) {
	return a.time < b.time;
}

std::vector<int> KEYBINDS_5K{ KEY_D,KEY_F,KEY_J,KEY_K,KEY_L };
std::vector<int> KEYBINDS_4K{ KEY_D,KEY_F,KEY_J,KEY_K };
std::vector<int> prev4k = KEYBINDS_4K;
std::vector<int> prev5k = KEYBINDS_5K;
bool changing4k = false;

rapidjson::Value vectorToJsonArray(const std::vector<int>& vec, rapidjson::Document::AllocatorType& allocator) {
	rapidjson::Value array(rapidjson::kArrayType);
	for (const auto& value : vec) {
		array.PushBack(value, allocator);
	}
	return array;
}

static void saveKeyBinds() {
	rapidjson::Document document;
	document.SetObject();
	rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
	rapidjson::Value keybinds4k = vectorToJsonArray(KEYBINDS_4K, allocator);
	rapidjson::Value keybinds5k = vectorToJsonArray(KEYBINDS_5K, allocator);
	document.AddMember("4k", keybinds4k, allocator);
	document.AddMember("5k", keybinds5k, allocator);
	char writeBuffer[2048];
	FILE* fp = fopen("keybinds.json","wb");
	rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));

	rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
	document.Accept(writer);
	fclose(fp);
}

int instrument = 0;
int diff = 0;
bool midiLoaded = false;
bool isPlaying = false;
bool streamsLoaded = false;
int selectStage = 0;
std::vector<std::pair<Music,int>> loadedStreams;
int curPlayingSong = 0;
int curNoteIdx = 0;
int curODPhrase = 0;
int curBeatLine = 0;
int curBPM = 0;


Vector2 viewScroll = { 0,0 };
Rectangle view = { 0 };

std::vector<float> bns = { 0.5f,0.75f,1.0f,1.25f,1.5f,1.75f,2.0f };
int bn = 4;
std::string bnsButton = "Track Speed 1.5x";

std::vector<double> laneTimes = { 0.0,0.0,0.0,0.0,0.0 };
std::vector<double> liftTimes = { 0.0,0.0,0.0,0.0,0.0 };
std::vector<bool> heldFrets = { false,false,false,false,false };
std::vector<bool> overhitFrets = { false,false,false,false,false };
std::vector<bool> tapRegistered{ false,false,false,false,false };
std::vector<bool> liftRegistered{ false,false,false,false,false };
SongList songList;
static void notesCallback(GLFWwindow* wind, int key, int scancode, int action, int mods) {
    // if (selectStage == 2) {
        if (action < 2) {

            Chart &curChart = songList.songs[curPlayingSong].parts[instrument]->charts[diff];
            float eventTime = GetMusicTimePlayed(loadedStreams[0].first);
            if (key == KEY_SPACE && overdriveFill > 0 && !overdrive) {
                overdriveActiveTime = eventTime;
                overdriveActiveFill = overdriveFill;
                overdrive = true;

            }
            int lane = -1;
            if (diff == 3) {
                for (int i = 0; i < 5; i++) {
                    if (key == KEYBINDS_5K[i]) {
                        if (action == GLFW_PRESS) {
                            heldFrets[i] = true;
                        } else if (action == GLFW_RELEASE) {
                            heldFrets[i] = false;
                            overhitFrets[i] = false;
                        }
                        lane = i;
                    }

                }
            } else {
                for (int i = 0; i < 4; i++) {
                    if (key == KEYBINDS_4K[i]) {
                        if (action == GLFW_PRESS) {
                            heldFrets[i] = true;
                        } else if (action == GLFW_RELEASE) {
                            heldFrets[i] = false;
                            overhitFrets[i] = false;
                        }
                        lane = i;
                    }
                }
            }

            for (int i = curNoteIdx; i < curChart.notes.size(); i++) {
                Note &curNote = curChart.notes[i];

                // if (curNote.time > eventTime + 0.1) break;
                if (lane != curNote.lane) continue;
                if ((curNote.lift && action == GLFW_RELEASE) || action == GLFW_PRESS) {
                    if ((curNote.time + VideoOffset)- (action == GLFW_RELEASE ? goodBackend * liftTimingMult : goodBackend) < eventTime &&
                            (curNote.time + VideoOffset) + (action == GLFW_RELEASE ? goodFrontend * liftTimingMult : goodFrontend) > eventTime &&
                            !curNote.hit) {

                        curNote.hit = true;
                        if ((curNote.len) > curNote.sustainThreshold && !curNote.lift) {
                            curNote.held = true;
                        }
                        if ((curNote.time + VideoOffset) - perfectBackend < eventTime && curNote.time + perfectFrontend > eventTime) {
                            curNote.perfect = true;

                        }
                        if (curNote.perfect) lastNotePerfect = true;
                        else lastNotePerfect = false;

                        break;
                    }
                    if (curNote.miss) lastNotePerfect = false;
                }
                // if (curNote.time + 0.1 < GetMusicTimePlayed(loadedStreams[0]) && !curNote.hit) {
                //	    curNote.miss = true;
                // }
                if (action == GLFW_RELEASE && curNote.held && (curNote.len) > curNote.sustainThreshold) {
                    curNote.held = false;
                    mute = true;
                    lastNotePerfect = false;
                    // SetAudioStreamVolume(loadedStreams[instrument].stream, missVolume);
                }
                if (action == GLFW_PRESS && !curNote.hit && !curNote.accounted && (curNote.time + VideoOffset) - perfectBackend > eventTime &&
                    !overhitFrets[lane]) {
                    player::OverHit();
                    overhitFrets[lane] = true;
                }

            }
        }
    //}
}


int main(int argc, char* argv[])
{
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	SetConfigFlags(FLAG_VSYNC_HINT);
	SetTraceLogLevel(LOG_NONE);

	// 800 , 600
	InitWindow(1,1, "Encore");
	SetWindowPosition(50,50);
	SetWindowSize(GetMonitorWidth(GetCurrentMonitor()) * 0.75, GetMonitorHeight(GetCurrentMonitor()) * 0.75);
	bool windowToggle = true;
	ArgumentList::InitArguments(argc, argv);

	std::string FPSCapStringVal = ArgumentList::GetArgValue("fpscap");
	int targetFPSArg = 0;

	if (FPSCapStringVal != "")
	{	
#ifdef NDEBUG
		str2int(&targetFPSArg, FPSCapStringVal.c_str());
#else		
		assert(str2int(&targetFPSArg, FPSCapStringVal.c_str()) == STR2INT_SUCCESS);
#endif
		TraceLog(LOG_INFO, "Argument overridden target FPS: %d", targetFPSArg);
	}


	//https://www.raylib.com/examples/core/loader.html?name=core_custom_frame_control

	double previousTime = GetTime();
	double currentTime = 0.0;
	double updateDrawTime = 0.0;
	double waitTime = 0.0;
	float deltaTime = 0.0f;


	

	float timeCounter = 0.0f;

	int targetFPS = targetFPSArg == 0 ? GetMonitorRefreshRate(GetCurrentMonitor()) : targetFPSArg;
	std::vector<string> songPartsList{ "Drums","Bass","Guitar","Vocals" };
	std::vector<string> diffList{ "Easy","Medium","Hard","Expert"};
	TraceLog(LOG_INFO, "Target FPS: %d", targetFPS);

	InitAudioDevice();

	SetExitKey(0);

	Camera3D camera = { 0 };

	// Y UP!!!! REMEMBER!!!!!!
	//							  x,    y,     z
							// 0.0f, 5.0f, -3.5f
	//								 6.5f
	camera.position = Vector3{ 0.0f, 7.0f, -9.5f };
	// 0.0f, 0.0f, 6.5f
	camera.target = Vector3{ 0.0f, 0.0f, 13.0f };

	camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
	camera.fovy = 34.5f;
	camera.projection = CAMERA_PERSPECTIVE;
	

	std::filesystem::path executablePath(GetApplicationDirectory());

	std::filesystem::path directory = executablePath.parent_path();

	std::filesystem::path songsPath = directory / "Songs";
	bool keybindsError = false;
	if (std::filesystem::exists(directory / "keybinds.json")) {
		std::ifstream ifs(directory / "keybinds.json");

		if (!ifs.is_open()) {
			std::cerr << "Failed to open JSON file." << std::endl;
		}

		std::string jsonString((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		ifs.close();
		rapidjson::Document document;
		document.Parse(jsonString.c_str());
		
		if (document.IsObject())
		{
			if (document.HasMember("4k") && document["4k"].IsArray()) {
				const rapidjson::Value& arr = document["4k"];
				if (arr.Size() == 4) {
					for (int i = 0; i < 4; i++) {
						if (arr[i].IsInt()) {
							KEYBINDS_4K[i] = arr[i].GetInt();
						}
						else {
							keybindsError = true;
						}
					}
					prev4k = KEYBINDS_4K;
				}
				else {
					keybindsError = true;
				}
			}
			else {
				keybindsError = true;
			}
			if (document.HasMember("5k") && document["5k"].IsArray()) {
				const rapidjson::Value& arr = document["5k"]; 
				if (arr.Size() == 5) {
					for (int i = 0; i < 5; i++) {
						if (arr[i].IsInt()) {
							KEYBINDS_5K[i] = arr[i].GetInt();
						}
						else {
							keybindsError = true;
						}
					}
					prev5k = KEYBINDS_5K;
				}
				else {
					keybindsError = true;
				}
			}
			else {
				keybindsError = true;
			}
            if (document.HasMember("avOffset")) {
                const rapidjson::Value& offset = document["avOffset"];
                VideoOffset = -(float)(offset.GetInt()/1000);
            }
		}
	}
	else {
		keybindsError = true;
	}
	if (keybindsError == true) {
		std::ofstream defaultbinds(directory / "keybinds.json");
		if (defaultbinds.is_open()) {
			// Write text to the file
			defaultbinds << "{\"4k\":[68, 70, 74, 75],\"5k\":[68, 70, 74, 75, 76]\n}";
			// Close the file
			defaultbinds.close();
		}
		else {
			std::cerr << "Error: Unable to open keybinds file for writing.\n";
		}
	}

	songList = LoadSongs(songsPath);

	ChangeDirectory(GetApplicationDirectory());
	//assets loading
	Model smasherReg = LoadModel((directory / "Assets/highway/smasher.obj").string().c_str());
	Texture2D smasherRegTex = LoadTexture((directory / "Assets/highway/smasher_reg.png").string().c_str());
	smasherReg.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = smasherRegTex;
	smasherReg.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
	
	Model smasherBoard = LoadModel((directory / "Assets/highway/board.obj").string().c_str());
	Texture2D smasherBoardTex = LoadTexture((directory / "Assets/highway/smasherBoard.png").string().c_str());
	smasherBoard.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = smasherBoardTex;
	smasherBoard.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;

	Model smasherPressed = LoadModel((directory / "Assets/highway/smasher.obj").string().c_str());
	Texture2D smasherPressTex = LoadTexture((directory / "Assets/highway/smasher_press.png").string().c_str());
	smasherPressed.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = smasherPressTex;
	smasherPressed.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;

	Model odFrame = LoadModel((directory / "Assets/ui/od_frame.obj").string().c_str());
	Model odBar = LoadModel((directory / "Assets/ui/od_fill.obj").string().c_str());
	Model multFrame = LoadModel((directory / "Assets/ui/multcircle_frame.obj").string().c_str());
	Model multBar = LoadModel((directory / "Assets/ui/multcircle_fill.obj").string().c_str());
	Model multCtr3 = LoadModel((directory / "Assets/ui/multbar_3.obj").string().c_str());
	Model multCtr5 = LoadModel((directory / "Assets/ui/multbar_5.obj").string().c_str());
	Model multNumber = LoadModel((directory / "Assets/ui/mult_number_plane.obj").string().c_str());
	Texture2D odMultFrame = LoadTexture((directory / "Assets/ui/mult_base.png").string().c_str());
	Texture2D odMultFill = LoadTexture((directory / "Assets/ui/mult_fill.png").string().c_str());
    Texture2D odMultFillActive = LoadTexture((directory / "Assets/ui/mult_fill_od.png").string().c_str());
	Texture2D multNumberTex = LoadTexture((directory / "Assets/ui/mult_number.png").string().c_str());
	Shader odMultShader = LoadShader(0, "Assets/ui/odmult.fs");
	Shader multNumberShader = LoadShader(0, "Assets/ui/multnumber.fs");
	odFrame.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = odMultFrame;
	odBar.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = odMultFrame;
	multFrame.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = odMultFrame;
	multBar.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = odMultFrame;
	multCtr3.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = odMultFrame;
	multCtr5.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = odMultFrame;
	odBar.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFill;
    multBar.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFill;
    multCtr3.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFill;
    multCtr5.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFill;
	odBar.materials[0].shader = odMultShader;
	multBar.materials[0].shader = odMultShader;
	multCtr3.materials[0].shader = odMultShader;
	multCtr5.materials[0].shader = odMultShader;
	int odLoc= GetShaderLocation(odMultShader, "overdrive");
	int comboCounterLoc = GetShaderLocation(odMultShader, "comboCounter");
	int multLoc = GetShaderLocation(odMultShader, "multBar");
	int isBassOrVocalLoc = GetShaderLocation(odMultShader, "isBassOrVocal");
	odMultShader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(odMultShader, "fillTex");

	multNumber.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = multNumberTex;
	multNumber.materials[0].shader = multNumberShader;
	int uvOffsetXLoc= GetShaderLocation(multNumberShader, "uvOffsetX");
	int uvOffsetYLoc = GetShaderLocation(multNumberShader, "uvOffsetY");
	
	Model expertHighway = LoadModel((directory / "Assets/highway/expert.obj").string().c_str());
	Model emhHighway = LoadModel((directory / "Assets/highway/emh.obj").string().c_str());
	Texture2D highwayTexture = LoadTexture((directory / "Assets/highway/highway.png").string().c_str());
	Texture2D highwayTextureOD = LoadTexture((directory / "Assets/highway/highway_od.png").string().c_str());
	expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTexture;
	expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
	emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTexture;
	emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
	Model noteModel = LoadModel((directory / "Assets/notes/note.obj").string().c_str());
	Texture2D noteTexture = LoadTexture((directory / "Assets/notes/note_d.png").string().c_str()); 
	Texture2D emitTexture = LoadTexture((directory / "Assets/notes/note_e.png").string().c_str());
	noteModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = noteTexture;
	noteModel.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
	noteModel.materials[0].maps[MATERIAL_MAP_EMISSION].texture = emitTexture;
	noteModel.materials[0].maps[MATERIAL_MAP_EMISSION].color = WHITE;
	Model noteModelOD = LoadModel((directory / "Assets/notes/note.obj").string().c_str());
	Texture2D noteTextureOD = LoadTexture((directory / "Assets/notes/note_od_d.png").string().c_str());
	Texture2D emitTextureOD = LoadTexture((directory / "Assets/notes/note_od_e.png").string().c_str());
	noteModelOD.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = noteTextureOD;
	noteModelOD.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
	noteModelOD.materials[0].maps[MATERIAL_MAP_EMISSION].texture = emitTextureOD;
	noteModelOD.materials[0].maps[MATERIAL_MAP_EMISSION].color = WHITE;
	Model liftModel = LoadModel((directory / "Assets/notes/lift.obj").string().c_str());
	liftModel.materials[0].maps[MATERIAL_MAP_ALBEDO].color = Color{ 172,82,217,127 };
	Model liftModelOD = LoadModel((directory / "Assets/notes/lift.obj").string().c_str());
	liftModelOD.materials[0].maps[MATERIAL_MAP_ALBEDO].color = Color{ 217, 183, 82 ,127 };

    //Sound clapOD = LoadSound((directory / "Assets/highway/clap.ogg").string().c_str());
    //SetSoundVolume(clapOD, 0.375);

	GLFWkeyfun origCallback = glfwSetKeyCallback(glfwGetCurrentContext(), notesCallback);

	
	while (!WindowShouldClose())
	{ 
		float diffDistance = diff == 3 ? 2.0f : 1.5f;
		float lineDistance = diff == 3 ? 1.5f : 1.0f;
		BeginDrawing();

		ClearBackground(DARKGRAY);

		
		
		if (!isPlaying) {
			//keybinds:
			if (selectStage == -1) {
				if (GuiButton({ ((float)GetScreenWidth()/2)-150,((float)GetScreenHeight()-60),100,60}, "Cancel")) {
					selectStage = 0;
					KEYBINDS_4K = prev4k;
					KEYBINDS_5K = prev5k;
				}
				if (GuiButton({ ((float)GetScreenWidth() / 2) + 150,((float)GetScreenHeight() - 60),100,60 }, "Apply")) {
					selectStage = 0;
					prev4k = KEYBINDS_4K;
					prev5k = KEYBINDS_5K;
					saveKeyBinds();
				}
				for (int i = 0; i < 5; i++) {
					int j = i - 2;
					std::string charStr = "UNKNOWN";
					if (KEYBINDS_5K[i] >= 39 && KEYBINDS_5K[i] < 96 && KEYBINDS_5K[i] != KEY_MENU) {
						charStr=static_cast<char>(KEYBINDS_5K[i]);
					}
					else {
						charStr = getKeyStr(KEYBINDS_5K[i]);
					}
					if (GuiButton({ ((float)GetScreenWidth() / 2) + (60 * (float)j),((float)GetScreenHeight() / 2) - 120,60,60 }, charStr.c_str())) {
						changing4k = false;
						heldFrets[i] = true;
					}
					if (!changing4k && heldFrets[i]) {
						std::string changeString = "Press a key for 5k lane " + std::to_string(i);
						DrawText(changeString.c_str(), (GetScreenWidth()-MeasureText(changeString.c_str(),20))/2, (GetScreenHeight()/2)+40, 20, BLACK);
						int pressedKey = GetKeyPressed();
						if (pressedKey != 0) {
							KEYBINDS_5K[i] = pressedKey;
							heldFrets[i] = false;
						}
					}
				}
				for (int i = 0; i < 4; i++) {
					float j = i - 1.5f; 
					std::string charStr = "UNKNOWN";
					if (KEYBINDS_4K[i] >= 39 && KEYBINDS_4K[i] < 96 && KEYBINDS_4K[i] != KEY_MENU) {
						charStr = static_cast<char>(KEYBINDS_4K[i]);
					}
					else {
						charStr = getKeyStr(KEYBINDS_4K[i]);
					}
					if (GuiButton({ ((float)GetScreenWidth() / 2) + (60 * j),((float)GetScreenHeight() / 2) - 40,60,60 }, charStr.c_str())) {
						heldFrets[i] = true;
						changing4k = true;
					}
					if (changing4k && heldFrets[i]) {
						std::string changeString = "Press a key for 4k lane " + std::to_string(i);
						DrawText(changeString.c_str(), (GetScreenWidth() - MeasureText(changeString.c_str(), 20)) / 2, (GetScreenHeight() / 2) + 40, 20, BLACK);
						int pressedKey = GetKeyPressed();
						if (pressedKey != 0) {
							KEYBINDS_4K[i] = pressedKey;
							heldFrets[i] = false;
						}
					}
				}
			}
			//song select
			else if (selectStage == 0) {
				streamsLoaded = false;
				midiLoaded = false;
				isPlaying = false;
				curNoteIdx = 0;
				curODPhrase = 0;
				curBeatLine = 0;
				curBPM = 0;
				instrument = 0;
				diff = 0;
				float curSong = 0.0f;
				if (GuiButton({ (float)GetScreenWidth() - 160,0,60,60 }, "Exit")) {
					exit(0);
				}
				if (GuiButton({ (float)GetScreenWidth() - 100,0,100,60 }, "Keybinds")) {
					selectStage = -1;
				}
				if (GuiButton({ (float)GetScreenWidth() - 100,60,100,60 }, "Fullscreen")) {
					windowToggle = !windowToggle;
					ToggleBorderlessWindowed();
					if (windowToggle) {
						SetWindowPosition(50, 50);
						SetWindowSize(GetMonitorWidth(GetCurrentMonitor()) / 2, GetMonitorHeight(GetCurrentMonitor()) / 2);
					}
					else {
						SetWindowSize(GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor()));
					};
				}
				if (GuiButton({ (float)GetScreenWidth() - 150,120,150,60 }, bnsButton.c_str())) {
					if (bn == 6) bn = 0; else bn++;
					bnsButton = "Track Speed "+std::to_string(bns[bn])+"x";
				}
				GuiSetStyle(0, 19, 0x505050ff);
				GuiScrollPanel({ 0, 0, (float)GetScreenWidth() * (3.0f / 5.0f)+15, (float)GetScreenHeight()}, NULL, { 0, 0, (float)GetScreenWidth() * (3.0f / 5.0f), 60.0f * songList.songs.size()}, &viewScroll, &view);
				for (Song song : songList.songs) {

					if (GuiButton(Rectangle { 0,viewScroll.y + (60 * curSong),((float)GetScreenWidth() * (3.0f/5.0f))+2 , 60 }, "")) {
						curPlayingSong = (int)curSong;
						selectStage = 1;
					}
					DrawTextureEx(song.albumArt, Vector2{ 5,viewScroll.y+(60 * curSong) + 5 }, 0.0f, 0.1f, RAYWHITE);
					DrawText(song.title.c_str(), 65, viewScroll.y + (60 * curSong) + 15, 30, BLACK);
					DrawText(song.artist.c_str(), ((float)GetScreenWidth()*(1.5f/4.0f)), viewScroll.y + (60 * curSong) + 20, 20, BLACK);
					curSong++;
				}

			}
			//instrument select
			else if (selectStage == 1) {
				if (!midiLoaded) {
					if (!songList.songs[curPlayingSong].midiParsed) {
						smf::MidiFile midiFile;
						midiFile.read(songList.songs[curPlayingSong].midiPath.string());
						songList.songs[curPlayingSong].getTiming(midiFile, 0, midiFile[0]);
						for (int i = 0; i < midiFile.getTrackCount(); i++)
						{
							std::string trackName;
							for (int j = 0; j < midiFile[i].getSize(); j++) {
								if (midiFile[i][j].isMeta()) {
									if ((int)midiFile[i][j][1] == 3) {
										for (int k = 3; k < midiFile[i][j].getSize(); k++) {
											trackName += midiFile[i][j][k];
										}
										SongParts songPart = partFromString(trackName);
										if (trackName == "BEAT")
											songList.songs[curPlayingSong].parseBeatLines(midiFile, i, midiFile[i]);
										else if (trackName == "EVENTS") {
											songList.songs[curPlayingSong].getStartEnd(midiFile, i, midiFile[i]);
										}
										else {
											if (songPart != SongParts::Invalid) {
												songList.songs[curPlayingSong].parts[(int)songPart]->hasPart = true;
												for (int diff = 0; diff < 4; diff++) {
													Chart newChart;
													newChart.parseNotes(midiFile, i, midiFile[i], diff);
													std::sort(newChart.notes.begin(), newChart.notes.end(), compareNotes);
													std::vector<BPM>& bpms = songList.songs[curPlayingSong].bpms;
													int curNoteOut = 0;
													for (Note& note : newChart.notes) {
														if (curBPM < bpms.size() - 1) {
															int nextBPM = curBPM + 1;
															if (note.time >= bpms[curBPM].time && note.time < bpms[nextBPM].time) {
																note.sustainThreshold = 20 / bpms[curBPM].bpm;
															}
															else if (note.time >= bpms[nextBPM].time) {
																note.sustainThreshold = 20 / bpms[nextBPM].bpm;
																curBPM++;
															}
														}
														else {
															if (note.time >= bpms[curBPM].time) {
																note.sustainThreshold = 20 / bpms[curBPM].bpm;
															}
														}
														curNoteOut++;
													}
													curBPM = 0;
													songList.songs[curPlayingSong].parts[(int)songPart]->charts.push_back(newChart);
												}
											}
										}
									}
								}
							}
						}
						songList.songs[curPlayingSong].midiParsed = true;
					}
					midiLoaded = true;
				}
				else {
					if (GuiButton({ 0,0,60,60 }, "<")) {
						midiLoaded = false;
						selectStage = 0;
					}
					for (int i = 0; i < 4; i++) {
						if (songList.songs[curPlayingSong].parts[i]->hasPart) {
							if (GuiButton({ 0,60 + (60 * (float)i),300,60 }, "")) {
								instrument = i;
								int isBassOrVocal = 0;
								if (instrument == 1 || instrument == 3) {
									isBassOrVocal = 1;
								}
								SetShaderValue(odMultShader, isBassOrVocalLoc, &isBassOrVocal, SHADER_UNIFORM_INT);
								selectStage = 2;
							}
							DrawText(songPartsList[i].c_str(), 20, 75 + (60 * (float)i), 30, BLACK);
							DrawText((std::to_string(songList.songs[curPlayingSong].parts[i]->diff+1)+"/7").c_str(), 220, 75 + (60 * (float)i), 30, BLACK);
						}
					}
				}
				
			}
			else if (selectStage == 2) {
				for (int i = 0; i < 4; i++) {
					if (GuiButton({ 0,0,60,60 }, "<")) {
						selectStage = 1;
					}
					if (songList.songs[curPlayingSong].parts[instrument]->charts[i].notes.size()>0) {
						if (GuiButton({ 0,60 + (60 * (float)i),300,60 }, "")) {
							diff = i;
							selectStage = 3;
							isPlaying = true;
							glfwSetKeyCallback(glfwGetCurrentContext(), notesCallback);
						}
					}
					DrawText(diffList[i].c_str(), 150-(MeasureText(diffList[i].c_str(),30)/2), 75 + (60 * (float)i), 30, BLACK);
				}
			}
		}
		else {
			int starsval = stars();
            DrawText(TextFormat("Stars: %01i", starsval), 5, GetScreenHeight() - 310, 24, goldStars ? GOLD : WHITE);
			DrawText(TextFormat("Notes Hit: %01i", notesHit), 5, GetScreenHeight() - 250, 24, FC ? GOLD : WHITE);
			DrawText(TextFormat("Notes Missed: %01i", notesMissed), 5, GetScreenHeight() - 220, 24, ((combo == 0) && (!FC)) ? RED : WHITE);
			DrawText(TextFormat("Score: %01i", score), 5, GetScreenHeight() - 190, 24, WHITE);
			DrawText(TextFormat("Combo: %01i", combo), 5, GetScreenHeight() - 160, 24, ((combo <= 5) && (!FC)) ? RED : WHITE);
			DrawText(TextFormat("Multiplier: %01i", multiplier(instrument)), 5, GetScreenHeight() - 130, 24, (multiplier(instrument) >= 4) ? SKYBLUE : WHITE);
			DrawText(TextFormat("FC run: %s", FC ? "True" : "False"), 5, GetScreenHeight() - 100, 24, FC ? GOLD : WHITE);
            DrawText(TextFormat("Strikes: %01i", playerOverhits), 5, GetScreenHeight() - 70, 24, FC ? GOLD : WHITE);
            DrawText(TextFormat("%s", lastNotePerfect ? "Perfect" : ""), 5, (GetScreenHeight()-370), 48, GOLD);
			float multFill = (!overdrive ? (float)(multiplier(instrument)-1) : ((float)(multiplier(instrument)/2)-1)) / (float)maxMultForMeter(instrument);
			SetShaderValue(odMultShader, multLoc, &multFill, SHADER_UNIFORM_FLOAT);
			SetShaderValue(multNumberShader, uvOffsetXLoc, &uvOffsetX, SHADER_UNIFORM_FLOAT);
			SetShaderValue(multNumberShader, uvOffsetYLoc, &uvOffsetY, SHADER_UNIFORM_FLOAT);
			float comboFill = comboFillCalc(instrument);
			SetShaderValue(odMultShader, comboCounterLoc, &comboFill, SHADER_UNIFORM_FLOAT);
			SetShaderValue(odMultShader, odLoc, &overdriveFill, SHADER_UNIFORM_FLOAT);
            DrawText(TextFormat("Perfect Hit: %01i", perfectHit), 5, GetScreenHeight() - 280, 24, (perfectHit > 0) ? GOLD : WHITE);



			if (GuiButton({ 0,0,60,60 }, "<")) {
				for (Note &note : songList.songs[curPlayingSong].parts[instrument]->charts[diff].notes) {
					note.accounted = false;
					note.hit = false;
					note.miss = false;
					note.held = false;
					note.heldTime = 0;
					note.perfect = false;

				}
				glfwSetKeyCallback(glfwGetCurrentContext(), origCallback);
                // notes = songList.songs[curPlayingSong].parts[instrument]->charts[diff].notes.size();
                // notes = songList.songs[curPlayingSong].parts[instrument]->charts[diff];
				for (odPhrase &phrase : songList.songs[curPlayingSong].parts[instrument]->charts[diff].odPhrases) {
					phrase.missed = false;
					phrase.notesHit = 0;
					phrase.added = false;
				}
				selectStage = 0;

				overdrive = false;
				overdriveFill = 0.0f;
				overdriveActiveFill = 0.0f;
				overdriveActiveTime = 0.0;
				isPlaying = false;
				midiLoaded = false;
				streamsLoaded = false;
			}
			if (!streamsLoaded) {
				loadedStreams = LoadStems(songList.songs[curPlayingSong].stemsPath);
				streamsLoaded = true;
				player::resetPlayerStats();
			}
			else {
				for (auto& stream : loadedStreams) {
					UpdateMusicStream(stream.first);
					PlayMusicStream(stream.first);
					if(instrument==stream.second)
						SetAudioStreamVolume(stream.first.stream, mute ? missVolume : selInstVolume);
					else
						SetAudioStreamVolume(stream.first.stream, otherInstVolume);

				}

			}
			double musicTime = GetMusicTimePlayed(loadedStreams[0].first);
			if (musicTime >= songList.songs[curPlayingSong].end) {
				for (Note& note : songList.songs[curPlayingSong].parts[instrument]->charts[diff].notes) {
					note.accounted = false;
					note.hit = false;
					note.miss = false;
					note.held = false;
					note.heldTime = 0;
					note.perfect = false;
                    // notes += 1;
				}
				glfwSetKeyCallback(glfwGetCurrentContext(), origCallback);
                // notes = (int)songList.songs[curPlayingSong].parts[instrument]->charts[diff].notes.size();
				for (odPhrase& phrase : songList.songs[curPlayingSong].parts[instrument]->charts[diff].odPhrases) {
					phrase.missed = false;
					phrase.notesHit = 0;
					phrase.added = false;
				}
				selectStage = 0;
				overdrive = false;
				overdriveFill = 0.0f;
				overdriveActiveFill = 0.0f;
				overdriveActiveTime = 0.0;
				isPlaying = false;
				midiLoaded = false;
				streamsLoaded = false;

			}
			if (overdrive) {
				expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTextureOD;
				emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTextureOD;
                multBar.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFillActive;
                multCtr3.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFillActive;
                multCtr5.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFillActive;


				overdriveFill = overdriveActiveFill-((musicTime-overdriveActiveTime)/(1920 / songList.songs[curPlayingSong].bpms[curBPM].bpm));
				if (overdriveFill <= 0) { 
					overdrive = false; 
					overdriveActiveFill = 0;
					overdriveActiveTime = 0.0;
					expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTexture;
					emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTexture;
                    multBar.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFill;
                    multCtr3.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFill;
                    multCtr5.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFill;

				}
			}
			for (int i = curBPM; i < songList.songs[curPlayingSong].bpms.size(); i++) {
				if (musicTime > songList.songs[curPlayingSong].bpms[i].time && i < songList.songs[curPlayingSong].bpms.size() - 1)
					curBPM++;
			}

			if (musicTime < 5.0) {
				DrawText(songList.songs[curPlayingSong].title.c_str(),  5, 65 , 30, WHITE);
				DrawText(songList.songs[curPlayingSong].artist.c_str(), 5, 100, 24, WHITE);
			}
				
			BeginMode3D(camera);
			if (diff == 3) {
				DrawModel(expertHighway, Vector3{ 0,0,0 }, 1.0f, WHITE);
				for (int i = 0; i < 5; i++) {
					if (heldFrets[i]) {
						DrawModel(smasherPressed, Vector3{ diffDistance - (float)(i), 0.01f, smasherPos }, 1.0f, WHITE);
					}
					else {
						DrawModel(smasherReg, Vector3{ diffDistance - (float)(i), 0.01f, smasherPos }, 1.0f, WHITE);
					}
				}
				for (int i = 0; i < 4; i++) {
					float radius = (i == 1) ? 0.05 : 0.02;
						
					DrawCylinderEx(Vector3{ lineDistance - i, 0, smasherPos + 0.5f}, Vector3{ lineDistance - i, 0, 20 }, radius, radius, 15, Color{128,128,128,128});
				}
					
				DrawModel(smasherBoard, Vector3{ 0, 0.001f, 0}, 1.04f, WHITE);
			}
			else {
                DrawModel(emhHighway, Vector3{0, 0, 0}, 1.0f, WHITE);
                for (int i = 0; i < 4; i++) {
                    if (heldFrets[i]) {
                        DrawModel(smasherPressed, Vector3{diffDistance - (float)(i), 0, smasherPos}, 1.0f, WHITE);
                    } else {
                        DrawModel(smasherReg, Vector3{diffDistance - (float)(i), 0, smasherPos}, 1.0f, WHITE);

                    }
                }
                for (int i = 0; i < 3; i++) {
                    float radius = (i == 1) ? 0.03 : 0.01;
                    DrawCylinderEx(Vector3{lineDistance - (float)i, 0, smasherPos}, Vector3{lineDistance - (float)i, 0, highwayLength}, radius,
                                    radius, 4.0f, Color{128, 128, 128, 128});
                }
                DrawModel(smasherBoard, Vector3{ 0, 0.001f, 0}, 1.04f, WHITE);
            }
				if (songList.songs[curPlayingSong].beatLines.size() >= 0) {
					for (int i = curBeatLine; i < songList.songs[curPlayingSong].beatLines.size(); i++) {
						if (songList.songs[curPlayingSong].beatLines[i].first >= songList.songs[curPlayingSong].music_start && songList.songs[curPlayingSong].beatLines[i].first <= songList.songs[curPlayingSong].end) {
							double relTime = ((songList.songs[curPlayingSong].beatLines[i].first - musicTime) + VideoOffset) * bns[bn];
							if (relTime > 1.5) break;
							float radius = songList.songs[curPlayingSong].beatLines[i].second ? 0.03f : 0.0075f;
							DrawCylinderEx(Vector3{ -diffDistance - 0.5f,0,smasherPos + (highwayLength * (float)relTime) }, Vector3{ diffDistance + 0.5f,0,smasherPos + (highwayLength * (float)relTime) }, radius, radius, 4, Color{ 128,128,128,128 });
							if (relTime < -1 && curBeatLine < songList.songs[curPlayingSong].beatLines.size() - 1) {
                                curBeatLine++;

                            }
						}

					}
				}
			// DrawTriangle3D(Vector3{ 2.5f,0.0f,0.0f }, Vector3{ -2.5f,0.0f,0.0f }, Vector3{ -2.5f,0.0f,20.0f }, BLACK);
			// DrawTriangle3D(Vector3{ 2.5f,0.0f,0.0f }, Vector3{ -2.5f,0.0f,20.0f }, Vector3{ 2.5f,0.0f,20.0f }, BLACK);

            notes = songList.songs[curPlayingSong].parts[instrument]->charts[diff].notes.size();
            DrawModel(odFrame, Vector3{ 0,1.0f,0 }, 0.75f, WHITE);
			DrawModel(odBar, Vector3{ 0,1.0f,0 }, 0.75f, WHITE);
			DrawModel(multFrame, Vector3{ 0,1.0f,0 }, 0.75f, WHITE);
			DrawModel(multBar, Vector3{ 0,1.0f,0 }, 0.75f, WHITE);
			if (instrument == 1 || instrument == 3) {

				DrawModel(multCtr5, Vector3{ 0,1.0f,0 }, 0.75f, WHITE);
			}
			else {

				DrawModel(multCtr3, Vector3{ 0,1.0f,0 }, 0.75f, WHITE);
			}
			DrawModel(multNumber, Vector3{ 0,1.0f,0 }, 0.75f, WHITE);
				

			// DrawLine3D(Vector3{ 2.5f, 0.05f, 2.0f }, Vector3{ -2.5f, 0.05f, 2.0f}, WHITE);
			Chart& curChart = songList.songs[curPlayingSong].parts[instrument]->charts[diff];
			if (curChart.odPhrases.size() > 0 && curODPhrase < curChart.odPhrases.size()) {
				if (curChart.notes[curNoteIdx].time+curChart.notes[curNoteIdx].len > curChart.odPhrases[curODPhrase].end && curODPhrase < curChart.odPhrases.size()-1) curODPhrase++;
			}
			for (int i = curNoteIdx; i < curChart.notes.size(); i++) {
				Note& curNote = curChart.notes[i];
				bool od = false;
				if (curChart.odPhrases.size() > 0) {
					if (curNote.time >= curChart.odPhrases[curODPhrase].start && curNote.time <= curChart.odPhrases[curODPhrase].end && !curChart.odPhrases[curODPhrase].missed) {
						if (curNote.miss == true) {
							curChart.odPhrases[curODPhrase].missed = true;
						}
						else {
							if (curNote.hit && !curNote.countedForODPhrase) {
								curChart.odPhrases[curODPhrase].notesHit++;
								curNote.countedForODPhrase = true;
							}
						}
						od = true;
					}
					if (curChart.odPhrases[curODPhrase].notesHit == curChart.odPhrases[curODPhrase].noteCount && !curChart.odPhrases[curODPhrase].added && overdriveFill < 1.0f) {
						overdriveFill += 0.25f;
						if (overdriveFill > 1.0f) overdriveFill = 1.0f;
						if (overdrive) {
							overdriveActiveFill = overdriveFill;
							overdriveActiveTime = musicTime;
						}
						curChart.odPhrases[curODPhrase].added = true;
					}
				}
				if (curNote.hit && !curNote.accounted) {
					player::HitNote(curNote.perfect, instrument);
					curNote.accounted = true;
				}
				else if (!curNote.accounted && !curNote.hit && curNote.time + 0.1 < musicTime) {
                    curNote.miss = true;
					player::MissNote();
					curNote.accounted = true;
				}

				double relTime = ((curNote.time - musicTime) + VideoOffset) * bns[bn];
				double relEnd = (((curNote.time + curNote.len) - musicTime) + VideoOffset) * bns[bn];
				if (relTime > 1.5) {
					break;
				}
				if (relEnd > 1.5) relEnd = 1.5;
				if (curNote.lift && !curNote.hit) {
					// lifts						//  distance between notes 
					//									(furthest left - lane distance)
					if (od)					//  1.6f	0.8
						DrawModel(liftModelOD, Vector3{ diffDistance - (1.0f * curNote.lane),0,smasherPos + (highwayLength * (float)relTime) }, 1.1f, WHITE);
					// energy phrase
					else
						DrawModel(liftModel, Vector3{ diffDistance - (1.0f * curNote.lane),0,smasherPos + (highwayLength * (float)relTime) }, 1.1f, WHITE);
					// regular 
				}
				else {
					// sustains
					if ((curNote.len)> curNote.sustainThreshold) {
						if (curNote.hit && curNote.held) {
							if (curNote.heldTime < (curNote.len * bns[bn])) {
								curNote.heldTime = 0.0 - relTime;
								if (relTime < 0.0) relTime = 0.0;
							}
							if (relEnd <=0.0) {
								if (relTime < 0.0) relTime = relEnd;
								curNote.held = false;
							}
						}
						else if (curNote.hit && !curNote.held) {
							relTime = relTime + curNote.heldTime;
						}

                        /*Color SustainColor = Color{ 69,69,69,255 };
                        if (curNote.held) {
                            if (od) {
                                Color SustainColor = Color{ 217, 183, 82 ,255 };
                            }
                            Color SustainColor = Color{ 172,82,217,255 };
                        }*/

                        if (curNote.held && !od) {
                            DrawCylinderEx(Vector3{diffDistance - (1.0f * curNote.lane), 0.05f, smasherPos + (highwayLength * (float)relTime)}, Vector3{diffDistance-(1.0f * curNote.lane),0.05f, smasherPos + (highwayLength * (float)relEnd)}, 0.1f, 0.1f, 15, Color{ 230,100,230,255 });
                        }
                        if (od && curNote.held) {
                            DrawCylinderEx(Vector3{diffDistance - (1.0f * curNote.lane), 0.05f, smasherPos + (highwayLength * (float)relTime)}, Vector3{diffDistance-(1.0f * curNote.lane),0.05f, smasherPos + (highwayLength * (float)relEnd)}, 0.1f, 0.1f, 15, Color{ 255, 255, 180 ,255 });
                        }
                        if (!curNote.held && curNote.hit || curNote.miss) {
                            DrawCylinderEx(Vector3{diffDistance - (1.0f * curNote.lane), 0.05f, smasherPos + (highwayLength * (float)relTime)}, Vector3{diffDistance-(1.0f * curNote.lane),0.05f, smasherPos + (highwayLength * (float)relEnd)}, 0.1f, 0.1f, 15, Color{ 69,69,69,255 });
                        }
                        if (!curNote.hit && !curNote.accounted && !curNote.miss) {
                            if (od) {
                                DrawCylinderEx(Vector3{diffDistance - (1.0f * curNote.lane), 0.05f, smasherPos + (highwayLength * (float)relTime)}, Vector3{diffDistance-(1.0f * curNote.lane),0.05f, smasherPos + (highwayLength * (float)relEnd)}, 0.1f, 0.1f, 15, Color{ 200, 170, 70 ,255 });
                            } else {
                                DrawCylinderEx(Vector3{diffDistance - (1.0f * curNote.lane), 0.05f,
                                                       smasherPos + (highwayLength * (float) relTime)},
                                               Vector3{diffDistance - (1.0f * curNote.lane), 0.05f,
                                                       smasherPos + (highwayLength * (float) relEnd)}, 0.1f, 0.1f, 15,
                                               Color{150, 19, 150, 255});
                            }
                        }


							// DrawLine3D(Vector3{ diffDistance - (1.0f * curNote.lane),0.05f,smasherPos + (12.5f * (float)relTime) }, Vector3{ diffDistance - (1.0f * curNote.lane),0.05f,smasherPos + (12.5f * (float)relEnd) }, Color{ 172,82,217,255 });
					}
					// regular notes
					if (((curNote.len) >= curNote.sustainThreshold && (curNote.held || !curNote.hit)) || ((curNote.len) < curNote.sustainThreshold && !curNote.hit)) {
						if (od) {
							if ((!curNote.held && !curNote.miss ) || !curNote.hit) {
								DrawModel(noteModelOD, Vector3{ diffDistance - (1.0f * curNote.lane),0,smasherPos + (highwayLength * (float)relTime) }, 1.1f, WHITE);
							};
								
						}
						else {
							if ((!curNote.held && !curNote.miss ) || !curNote.hit) {
								DrawModel(noteModel, Vector3{ diffDistance - (1.0f * curNote.lane),0,smasherPos + (highwayLength * (float)relTime) }, 1.1f, WHITE);
							};
								
						}

					}



				}
                if (curNote.miss) {
                    DrawModel(curNote.lift ? liftModel : noteModel, Vector3{ diffDistance - (1.0f * curNote.lane),0,smasherPos + (highwayLength * (float)relTime) }, 1.0f, RED);
                }
                if (curNote.hit && GetMusicTimePlayed(loadedStreams[0].first) < curNote.time + 0.05f) {
                    DrawCube(Vector3{diffDistance - (1.0f * curNote.lane), 0, smasherPos}, 1.0f, 0.5f, 0.5f, curNote.perfect ? Color{255,215,0,128} : Color{255,255,255,64});
                }



				if (relEnd < -1 && curNoteIdx < curChart.notes.size()-1) curNoteIdx = i + 1;

			}
#ifndef NDEBUG
			// DrawTriangle3D(Vector3{ -diffDistance - 0.5f,0.05f,smasherPos + (highwayLength * goodFrontend * bns[bn]) }, Vector3{ diffDistance + 0.5f,0.05f,smasherPos + (highwayLength * goodFrontend * bns[bn]) }, Vector3{ diffDistance + 0.5f,0.05f,smasherPos - (highwayLength * goodBackend * bns[bn]) }, Color{ 0,255,0,80 });
			// DrawTriangle3D(Vector3{ diffDistance + 0.5f,0.05f,smasherPos - (highwayLength * goodBackend * bns[bn]) }, Vector3{ -diffDistance - 0.5f,0.05f,smasherPos - (highwayLength * goodBackend * bns[bn]) }, Vector3{ -diffDistance - 0.5f,0.05f,smasherPos + (highwayLength * goodFrontend * bns[bn]) }, Color{ 0,255,0,80 });

            // DrawTriangle3D(Vector3{ -diffDistance - 0.5f,0.05f,smasherPos + (highwayLength * perfectFrontend * bns[bn]) }, Vector3{ diffDistance + 0.5f,0.05f,smasherPos + (highwayLength * perfectFrontend * bns[bn]) }, Vector3{ diffDistance + 0.5f,0.05f,smasherPos - (highwayLength * perfectBackend * bns[bn]) }, Color{ 190,255,0,80 });
            // DrawTriangle3D(Vector3{ diffDistance + 0.5f,0.05f,smasherPos - (highwayLength * perfectBackend * bns[bn]) }, Vector3{ -diffDistance - 0.5f,0.05f,smasherPos - (highwayLength * perfectBackend * bns[bn]) }, Vector3{ -diffDistance - 0.5f,0.05f,smasherPos + (highwayLength * perfectFrontend * bns[bn]) }, Color{ 190,255,0,80 });
#endif
            EndMode3D();
		
			
			
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

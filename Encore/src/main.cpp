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
#include "game/assets.h"
#include "game/settings.h"
#include "raygui.h"

#include <stdlib.h>
#include "GLFW/glfw3.h"
#include <cstdio>
vector<std::string> ArgumentList::arguments;

bool compareNotes(const Note& a, const Note& b) {
	return a.time < b.time;
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
int selLane = 0;
bool changingKey = false;

Vector2 viewScroll = { 0,0 };
Rectangle view = { 0 };


std::string trackSpeedButton = "Track Speed 1.50x";


std::vector<double> laneTimes = { 0.0,0.0,0.0,0.0,0.0 };
std::vector<double> liftTimes = { 0.0,0.0,0.0,0.0,0.0 };
std::vector<bool> heldFrets = { false,false,false,false,false };
std::vector<bool> overhitFrets = { false,false,false,false,false };
std::vector<bool> tapRegistered{ false,false,false,false,false };
std::vector<bool> liftRegistered{ false,false,false,false,false };
SongList songList;
Settings settings;
Assets assets;

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
                    if (key == settings.keybinds5K[i]) {
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
                    if (key == settings.keybinds4K[i]) {
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
                    if ((curNote.time)- (action == GLFW_RELEASE ? goodBackend * liftTimingMult : goodBackend) < eventTime+InputOffset &&
                            (curNote.time) + (action == GLFW_RELEASE ? goodFrontend * liftTimingMult : goodFrontend) > eventTime+InputOffset &&
                            !curNote.hit) {

                        curNote.hit = true;
                        if ((curNote.len) > curNote.sustainThreshold && !curNote.lift) {
                            curNote.held = true;
                        }
                        if ((curNote.time) - perfectBackend < eventTime + InputOffset&& curNote.time + perfectFrontend > eventTime+InputOffset) {
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
				if (action == GLFW_PRESS && eventTime<songList.songs[curPlayingSong].music_start && !curNote.hit && !curNote.accounted && (curNote.time) - perfectBackend > eventTime + InputOffset &&
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

	settings.loadSettings(directory / "settings.json");

	songList = LoadSongs(songsPath);

	ChangeDirectory(GetApplicationDirectory());
	assets.loadAssets(directory);

	GLFWkeyfun origCallback = glfwSetKeyCallback(glfwGetCurrentContext(), notesCallback);
	glfwSetKeyCallback(glfwGetCurrentContext(), origCallback);
	GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0x505050ff);
	GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
	while (!WindowShouldClose())
	{ 
		float diffDistance = diff == 3 ? 2.0f : 1.5f;
		float lineDistance = diff == 3 ? 1.5f : 1.0f;
		BeginDrawing();

		ClearBackground(DARKGRAY);

		if (!isPlaying) {
			if (selectStage == -1) {
				if (GuiButton({ ((float)GetScreenWidth()/2)-350,((float)GetScreenHeight()-60),100,60}, "Cancel")) {
					selectStage = 0;
					settings.keybinds4K = settings.prev4k;
					settings.keybinds5K = settings.prev5k;
					settings.trackSpeed = settings.prevTrackSpeed;
					settings.inputOffsetMS = settings.prevInputOffsetMS;
					settings.avOffsetMS = settings.prevAvOffsetMS;
				}
				if (GuiButton({ ((float)GetScreenWidth() / 2) + 250,((float)GetScreenHeight() - 60),100,60 }, "Apply")) {
					selectStage = 0;
					settings.prev4k = settings.keybinds4K;
					settings.prev5k = settings.keybinds5K;
					settings.prevTrackSpeed = settings.trackSpeed;
					settings.prevInputOffsetMS = settings.inputOffsetMS;
					settings.prevAvOffsetMS = settings.avOffsetMS;
					settings.saveSettings(directory / "settings.json");
				}
				if (GuiButton({ (float)GetScreenWidth() / 2 - 125,(float)GetScreenHeight() / 2-160,250,60 }, "")) {
					if (settings.trackSpeed == settings.trackSpeedOptions.size()-1) settings.trackSpeed = 0; else settings.trackSpeed++;
					trackSpeedButton = "Track Speed " + truncateFloatString(settings.trackSpeedOptions[settings.trackSpeed]) + "x";
				}
				DrawText(trackSpeedButton.c_str(), (float)GetScreenWidth()/2 - MeasureText(trackSpeedButton.c_str(),20)/2, (float)GetScreenHeight() / 2 - 140,20,BLACK);
				float avOffsetFloat = (float)settings.avOffsetMS;
				DrawText("A/V Offset", (float)GetScreenWidth()/2 - MeasureText("A/V Offset",20) / 2, (float)GetScreenHeight() / 2 - 80, 20, WHITE);
				DrawText(" -500 ", (float)GetScreenWidth() / 2 - 125 - MeasureText(" -500 ", 20), (float)GetScreenHeight() / 2 - 50, 20, WHITE);
				DrawText(" 500 ", (float)GetScreenWidth() / 2 + 125 , (float)GetScreenHeight() / 2 - 50, 20, WHITE);
				if (GuiSliderBar({ (float)GetScreenWidth() / 2 - 125,(float)GetScreenHeight() / 2 - 60,250,40 }, "", "", &avOffsetFloat, -500.0f, 500.0f)) {
					settings.avOffsetMS = (int)avOffsetFloat;
				}
				DrawText(std::to_string(settings.avOffsetMS).c_str(), (float)GetScreenWidth() / 2 - (MeasureText(std::to_string(settings.avOffsetMS).c_str(), 20) / 2), (float)GetScreenHeight() / 2 - 50, 20, BLACK);
				float inputOffsetFloat = (float)settings.inputOffsetMS;
				DrawText("Input Offset", (float)GetScreenWidth() / 2 - MeasureText("Input Offset", 20) / 2, (float)GetScreenHeight() / 2, 20, WHITE);
				DrawText(" -500 ", (float)GetScreenWidth() / 2 - 125 - MeasureText(" -500 ", 20), (float)GetScreenHeight() / 2 + 30, 20, WHITE);
				DrawText(" 500 ", (float)GetScreenWidth() / 2 + 125, (float)GetScreenHeight() / 2 + 30, 20, WHITE);
				if (GuiSliderBar({ (float)GetScreenWidth() / 2 - 125,(float)GetScreenHeight() / 2 + 20,250,40 }, "", "", &inputOffsetFloat, -500.0f, 500.0f)) {
					settings.inputOffsetMS = (int)inputOffsetFloat;
				}
				DrawText(std::to_string(settings.inputOffsetMS).c_str(), (float)GetScreenWidth()/2 - (MeasureText(std::to_string(settings.inputOffsetMS).c_str(), 20)/2), (float)GetScreenHeight() / 2 + 30, 20, BLACK);
				for (int i = 0; i < 5; i++) {
					float j = i - 2.0f;
					std::string charStr = "UNK";
					if (settings.keybinds5K[i] >= 39 && settings.keybinds5K[i] < 96) {
						charStr=static_cast<char>(settings.keybinds5K[i]);
					}
					else {
						charStr = getKeyStr(settings.keybinds5K[i]);
					}
					if (GuiButton({ ((float)GetScreenWidth() / 2)-40 + (80 * j),(float)GetScreenHeight()/2 + 80,80,60 }, charStr.c_str())) {
						settings.changing4k = false;
						selLane = i;
						changingKey = true;
					}
				}
				for (int i = 0; i < 4; i++) {
					float j = i - 1.5f; 
					std::string charStr = "UNK";
					if (settings.keybinds4K[i] >= 39 && settings.keybinds4K[i] < 96) {
						charStr = static_cast<char>(settings.keybinds4K[i]);
					}
					else {
						charStr = getKeyStr(settings.keybinds4K[i]);
					}
					if (GuiButton({ ((float)GetScreenWidth() / 2) - 40 + (80 * j),(float)GetScreenHeight() / 2 + 160,80,60 }, charStr.c_str())) {
						changingKey = true;
						selLane = i;
						settings.changing4k = true;
					}
				}
				if (changingKey) {
					std::vector<int>& bindsToChange = settings.changing4k ? settings.keybinds4K : settings.keybinds5K;
					DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), { 0,0,0,200 });
					std::string keyString = settings.changing4k ? "4k" : "5k";
					std::string changeString = "Press a key for " + keyString + " lane " + std::to_string(selLane+1);
					DrawText(changeString.c_str(), (GetScreenWidth() - MeasureText(changeString.c_str(), 20)) / 2, GetScreenHeight() / 2 - 10, 20, WHITE);
					int pressedKey = GetKeyPressed();
					if (pressedKey != 0) {
						bindsToChange[selLane] = pressedKey;
						selLane = 0;
						changingKey = false;
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
				if (GuiButton({ (float)GetScreenWidth() - 260,0,60,60 }, "Exit")) {
					exit(0);
				}
				if (GuiButton({ (float)GetScreenWidth() - 200,0,200,60 }, "Settings")) {
					selectStage = -1;
				}
				if (GuiButton({ (float)GetScreenWidth() - 200,60,200,60 }, "Fullscreen")) {
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
								SetShaderValue(assets.odMultShader, assets.isBassOrVocalLoc, &isBassOrVocal, SHADER_UNIFORM_INT);
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
			SetShaderValue(assets.odMultShader, assets.multLoc, &multFill, SHADER_UNIFORM_FLOAT);
			SetShaderValue(assets.multNumberShader, assets.uvOffsetXLoc, &uvOffsetX, SHADER_UNIFORM_FLOAT);
			SetShaderValue(assets.multNumberShader, assets.uvOffsetYLoc, &uvOffsetY, SHADER_UNIFORM_FLOAT);
			float comboFill = comboFillCalc(instrument);
			SetShaderValue(assets.odMultShader, assets.comboCounterLoc, &comboFill, SHADER_UNIFORM_FLOAT);
			SetShaderValue(assets.odMultShader, assets.odLoc, &overdriveFill, SHADER_UNIFORM_FLOAT);
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
			if (musicTime >= songList.songs[curPlayingSong].length) {
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
				assets.expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = assets.highwayTextureOD;
				assets.emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = assets.highwayTextureOD;
				assets.multBar.materials[0].maps[MATERIAL_MAP_EMISSION].texture = assets.odMultFillActive;
                assets.multCtr3.materials[0].maps[MATERIAL_MAP_EMISSION].texture = assets.odMultFillActive;
                assets.multCtr5.materials[0].maps[MATERIAL_MAP_EMISSION].texture = assets.odMultFillActive;


				overdriveFill = overdriveActiveFill-((musicTime-overdriveActiveTime)/(1920 / songList.songs[curPlayingSong].bpms[curBPM].bpm));
				if (overdriveFill <= 0) { 
					overdrive = false; 
					overdriveActiveFill = 0;
					overdriveActiveTime = 0.0;
					assets.expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = assets.highwayTexture;
					assets.emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = assets.highwayTexture;
                    assets.multBar.materials[0].maps[MATERIAL_MAP_EMISSION].texture = assets.odMultFill;
                    assets.multCtr3.materials[0].maps[MATERIAL_MAP_EMISSION].texture = assets.odMultFill;
                    assets.multCtr5.materials[0].maps[MATERIAL_MAP_EMISSION].texture = assets.odMultFill;

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
				DrawModel(assets.expertHighway, Vector3{ 0,0,0 }, 1.0f, WHITE);
				for (int i = 0; i < 5; i++) {
					if (heldFrets[i]) {
						DrawModel(assets.smasherPressed, Vector3{ diffDistance - (float)(i), 0.01f, smasherPos }, 1.0f, WHITE);
					}
					else {
						DrawModel(assets.smasherReg, Vector3{ diffDistance - (float)(i), 0.01f, smasherPos }, 1.0f, WHITE);
					}
				}
				for (int i = 0; i < 4; i++) {
					float radius = (i == 1) ? 0.05 : 0.02;
						
					DrawCylinderEx(Vector3{ lineDistance - i, 0, smasherPos + 0.5f}, Vector3{ lineDistance - i, 0, 20 }, radius, radius, 15, Color{128,128,128,128});
				}
					
				DrawModel(assets.smasherBoard, Vector3{ 0, 0.001f, 0 }, 1.04f, WHITE);
			}
			else {
				DrawModel(assets.emhHighway, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
				for (int i = 0; i < 4; i++) {
					if (heldFrets[i]) {
						DrawModel(assets.smasherPressed, Vector3{ diffDistance - (float)(i), 0, smasherPos }, 1.0f, WHITE);
					}
					else {
						DrawModel(assets.smasherReg, Vector3{ diffDistance - (float)(i), 0, smasherPos }, 1.0f, WHITE);

					}
				}
				for (int i = 0; i < 3; i++) {
					float radius = (i == 1) ? 0.03 : 0.01;
					DrawCylinderEx(Vector3{ lineDistance - (float)i, 0, smasherPos }, Vector3{ lineDistance - (float)i, 0, highwayLength }, radius,
						radius, 4.0f, Color{ 128, 128, 128, 128 });
				}
				DrawModel(assets.smasherBoard, Vector3{ 0, 0.001f, 0}, 1.04f, WHITE);
            }
				if (songList.songs[curPlayingSong].beatLines.size() >= 0) {
					for (int i = curBeatLine; i < songList.songs[curPlayingSong].beatLines.size(); i++) {
						if (songList.songs[curPlayingSong].beatLines[i].first >= songList.songs[curPlayingSong].music_start && songList.songs[curPlayingSong].beatLines[i].first <= songList.songs[curPlayingSong].end) {
							double relTime = ((songList.songs[curPlayingSong].beatLines[i].first - musicTime) + VideoOffset) * settings.trackSpeedOptions[settings.trackSpeed];
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
            DrawModel(assets.odFrame, Vector3{ 0,1.0f,0 }, 0.75f, WHITE);
			DrawModel(assets.odBar, Vector3{ 0,1.0f,0 }, 0.75f, WHITE);
			DrawModel(assets.multFrame, Vector3{ 0,1.0f,0 }, 0.75f, WHITE);
			DrawModel(assets.multBar, Vector3{ 0,1.0f,0 }, 0.75f, WHITE);
			if (instrument == 1 || instrument == 3) {

				DrawModel(assets.multCtr5, Vector3{ 0,1.0f,0 }, 0.75f, WHITE);
			}
			else {

				DrawModel(assets.multCtr3, Vector3{ 0,1.0f,0 }, 0.75f, WHITE);
			}
			DrawModel(assets.multNumber, Vector3{ 0,1.0f,0 }, 0.75f, WHITE);
				

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

				double relTime = ((curNote.time - musicTime) + VideoOffset) * settings.trackSpeedOptions[settings.trackSpeed];
				double relEnd = (((curNote.time + curNote.len) - musicTime) + VideoOffset) * settings.trackSpeedOptions[settings.trackSpeed];
				if (relTime > 1.5) {
					break;
				}
				if (relEnd > 1.5) relEnd = 1.5;
				if (curNote.lift && !curNote.hit) {
					// lifts						//  distance between notes 
					//									(furthest left - lane distance)
					if (od)					//  1.6f	0.8
						DrawModel(assets.liftModelOD, Vector3{ diffDistance - (1.0f * curNote.lane),0,smasherPos + (highwayLength * (float)relTime) }, 1.1f, WHITE);
					// energy phrase
					else
						DrawModel(assets.liftModel, Vector3{ diffDistance - (1.0f * curNote.lane),0,smasherPos + (highwayLength * (float)relTime) }, 1.1f, WHITE);
					// regular 
				}
				else {
					// sustains
					if ((curNote.len)> curNote.sustainThreshold) {
						if (curNote.hit && curNote.held) {
							if (curNote.heldTime < (curNote.len * settings.trackSpeedOptions[settings.trackSpeed])) {
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
								DrawModel(assets.noteModelOD, Vector3{ diffDistance - (1.0f * curNote.lane),0,smasherPos + (highwayLength * (float)relTime) }, 1.1f, WHITE);
							};

						}
						else {
							if ((!curNote.held && !curNote.miss) || !curNote.hit) {
								DrawModel(assets.noteModel, Vector3{ diffDistance - (1.0f * curNote.lane),0,smasherPos + (highwayLength * (float)relTime) }, 1.1f, WHITE);
							};
								
						}

					}



				}
                if (curNote.miss) {
                    DrawModel(curNote.lift ? assets.liftModel : assets.noteModel, Vector3{ diffDistance - (1.0f * curNote.lane),0,smasherPos + (highwayLength * (float)relTime) }, 1.0f, RED);
                }
                if (curNote.hit && GetMusicTimePlayed(loadedStreams[0].first) < curNote.time + 0.05f) {
                    DrawCube(Vector3{diffDistance - (1.0f * curNote.lane), 0, smasherPos}, 1.0f, 0.5f, 0.5f, curNote.perfect ? Color{255,215,0,128} : Color{255,255,255,64});
                }



				if (relEnd < -1 && curNoteIdx < curChart.notes.size()-1) curNoteIdx = i + 1;

			}
#ifndef NDEBUG
			// DrawTriangle3D(Vector3{ -diffDistance - 0.5f,0.05f,smasherPos + (highwayLength * goodFrontend * trackSpeed[speedSelection]) }, Vector3{ diffDistance + 0.5f,0.05f,smasherPos + (highwayLength * goodFrontend * trackSpeed[speedSelection]) }, Vector3{ diffDistance + 0.5f,0.05f,smasherPos - (highwayLength * goodBackend * trackSpeed[speedSelection]) }, Color{ 0,255,0,80 });
			// DrawTriangle3D(Vector3{ diffDistance + 0.5f,0.05f,smasherPos - (highwayLength * goodBackend * trackSpeed[speedSelection]) }, Vector3{ -diffDistance - 0.5f,0.05f,smasherPos - (highwayLength * goodBackend * trackSpeed[speedSelection]) }, Vector3{ -diffDistance - 0.5f,0.05f,smasherPos + (highwayLength * goodFrontend * trackSpeed[speedSelection]) }, Color{ 0,255,0,80 });

            // DrawTriangle3D(Vector3{ -diffDistance - 0.5f,0.05f,smasherPos + (highwayLength * perfectFrontend * trackSpeed[speedSelection]) }, Vector3{ diffDistance + 0.5f,0.05f,smasherPos + (highwayLength * perfectFrontend * trackSpeed[speedSelection]) }, Vector3{ diffDistance + 0.5f,0.05f,smasherPos - (highwayLength * perfectBackend * trackSpeed[speedSelection]) }, Color{ 190,255,0,80 });
            // DrawTriangle3D(Vector3{ diffDistance + 0.5f,0.05f,smasherPos - (highwayLength * perfectBackend * trackSpeed[speedSelection]) }, Vector3{ -diffDistance - 0.5f,0.05f,smasherPos - (highwayLength * perfectBackend * trackSpeed[speedSelection]) }, Vector3{ -diffDistance - 0.5f,0.05f,smasherPos + (highwayLength * perfectFrontend * trackSpeed[speedSelection]) }, Color{ 190,255,0,80 });
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

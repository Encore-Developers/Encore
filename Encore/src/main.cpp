#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
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

#include <cstdio>
vector<std::string> ArgumentList::arguments;

bool compareNotes(const Note& a, const Note& b) {
	return a.time < b.time;
}
std::vector<int> KEYBINDS_5K{ KEY_D,KEY_F,KEY_J,KEY_K,KEY_L };
std::vector<int> KEYBINDS_4K{ KEY_D,KEY_F,KEY_J,KEY_K };
std::vector<int> prev4k = KEYBINDS_4K;
std::vector<int> prev5k = KEYBINDS_5K;
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


std::unordered_map<int, std::string> keymap{

		{KEY_SPACE,"Space"},
		{KEY_ESCAPE,"Esc"},
		{KEY_ENTER,"Enter"},
		{KEY_TAB,"Tab"},
		{KEY_BACKSPACE,"Backspace"},
		{KEY_INSERT,"Insert"},
		{KEY_DELETE,"Del"},
		{KEY_RIGHT,"Right"},
		{KEY_LEFT,"Left"},
		{KEY_DOWN,"Down"},
		{KEY_UP,"Up"},
		{KEY_PAGE_UP,"PgUp"},
		{KEY_PAGE_DOWN,"PgDown"},
		{KEY_HOME,"Home"},
		{KEY_END,"End"},
		{KEY_CAPS_LOCK,"CapsLk"},
		{KEY_SCROLL_LOCK,"ScrLk"},
		{KEY_NUM_LOCK,"NumLk"},
		{KEY_PRINT_SCREEN,"PrtSc"},
		{KEY_PAUSE,"Pause"},
		{KEY_F1,"F1"},
		{KEY_F2,"F2"},
		{KEY_F3,"F3"},
		{KEY_F4,"F4"},
		{KEY_F5,"F5"},
		{KEY_F6,"F6"},
		{KEY_F7,"F7"},
		{KEY_F8,"F8"},
		{KEY_F9,"F9"},
		{KEY_F10,"F10"},
		{KEY_F11,"F11"},
		{KEY_F12,"F12"},
		{KEY_LEFT_SHIFT,"LShift"},
		{KEY_LEFT_CONTROL,"LCtrl"},
		{KEY_LEFT_ALT,"LShift"},
		{KEY_LEFT_SUPER,"LWin"},
		{KEY_RIGHT_SHIFT,"RShift"},
		{KEY_RIGHT_CONTROL,"RCtrl"},
		{KEY_RIGHT_ALT,"RShift"},
		{KEY_RIGHT_SUPER,"RWin"},
		{KEY_KB_MENU,"Menu"},
		{KEY_KP_0,"Num 0"},
		{KEY_KP_1,"Num 1"},
		{KEY_KP_2,"Num 2"},
		{KEY_KP_3,"Num 3"},
		{KEY_KP_4,"Num 4"},
		{KEY_KP_5,"Num 5"},
		{KEY_KP_6,"Num 6"},
		{KEY_KP_7,"Num 7"},
		{KEY_KP_8,"Num 8"},
		{KEY_KP_9,"Num 9"},
		{KEY_KP_DECIMAL,"Num ."},
		{KEY_KP_DIVIDE,"Num /"},
		{KEY_KP_MULTIPLY,"Num *"},
		{KEY_KP_SUBTRACT,"Num -"},
		{KEY_KP_ADD,"Num +"},
		{KEY_KP_ENTER,"Num Enter"},
		{KEY_KP_EQUAL,"Num ="}

};

std::string getKeyStr(int keycode)
{
	auto it = keymap.find(keycode);
	if (it != keymap.end())
	{
		return it->second;
	}
	else
	{
		return "UNKNOWN";
	}
}
int main(int argc, char* argv[])
{
#ifdef NDEBUG

#endif
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	SetConfigFlags(FLAG_VSYNC_HINT);
	// 800 , 600
	InitWindow(1920, 1080, "Encore");
	bool windowToggle = false;
	ToggleBorderlessWindowed();
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
	int instrument = 0;
	int diff = 0;
	

	float timeCounter = 0.0f;

	int targetFPS = targetFPSArg == 0 ? 180 : targetFPSArg;
	std::vector<string> songPartsList{ "Drums","Bass","Guitar","Vocals" };
	std::vector<string> diffList{ "Easy","Medium","Hard","Expert"};
	TraceLog(LOG_INFO, "Target FPS: %d", targetFPS);

	InitAudioDevice();

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
				}
				else {
					keybindsError = true;
				}
			}
			else {
				keybindsError = true;
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
			defaultbinds << "{\n    \"4k\":[68, 70, 74, 75],\n    \"5k\":[68, 70, 74, 75, 76]\n}";
			// Close the file
			defaultbinds.close();
		}
		else {
			std::cerr << "Error: Unable to open keybinds file for writing.\n";
		}
	}

	SongList songList = LoadSongs(songsPath);
	bool midiLoaded = false;
	bool isPlaying = false;
	bool streamsLoaded = false;
	int selectStage = 0;
	std::vector<Music> loadedStreams;
	int curPlayingSong = 0;
	int curNoteIdx = 0;
	int curODPhrase = 0;

	Model smasherReg = LoadModel((directory / "Assets/smasher.obj").string().c_str());
	Texture2D smasherRegTex = LoadTexture((directory / "Assets/smasher_reg.png").string().c_str());
	smasherReg.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = smasherRegTex;
	smasherReg.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;

	Model smasherPressed = LoadModel((directory / "Assets/smasher.obj").string().c_str());
	Texture2D smasherPressTex = LoadTexture((directory / "Assets/smasher_press.png").string().c_str());
	smasherPressed.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = smasherPressTex;
	smasherPressed.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;

	Model expertHighway = LoadModel((directory / "Assets/expert.obj").string().c_str());
	Model emhHighway = LoadModel((directory / "Assets/emh.obj").string().c_str());
	Texture2D highwayTexture = LoadTexture((directory / "Assets/highway.png").string().c_str());
	expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTexture;
	expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
	emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTexture;
	emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
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
	bool changing4k = false;
	
	while (!WindowShouldClose())
	{ 
		float diffDistance = diff == 3 ? 2.0f : 1.5f;
		float lineDistance = diff == 3 ? 1.5f : 1.0f;
		if (isPlaying) {
			if (diff == 3) {
				for (int i = 0; i < 5; i++) {
					if (IsKeyPressed(KEYBINDS_5K[i])) {
						laneTimes[i] = (double)GetMusicTimePlayed(loadedStreams[0]);
						liftTimes[i] = 0.0;
						heldFrets[i] = true;
					}
					if (IsKeyReleased(KEYBINDS_5K[i])) {
						laneTimes[i] = 0.0;
						liftTimes[i] = (double)GetMusicTimePlayed(loadedStreams[0]);
						heldFrets[i] = false;
					}
				}
			}
			else {
				for (int i = 0; i < 4; i++) {
					if (IsKeyPressed(KEYBINDS_4K[i])) {
						laneTimes[i] = (double)GetMusicTimePlayed(loadedStreams[0]);
						liftTimes[i] = 0.0;
						heldFrets[i] = true;
					}
					if (IsKeyReleased(KEYBINDS_4K[i])) {
						laneTimes[i] = 0.0;
						liftTimes[i] = (double)GetMusicTimePlayed(loadedStreams[0]);
						heldFrets[i] = false;
					}
				}
			}
		}
		BeginDrawing();

		ClearBackground(DARKGRAY);

		
		
		if (!isPlaying) {
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
					if (GuiButton({ (float)(GetScreenWidth() / 2) + (60 * j),(float)(GetScreenHeight() / 2) - 120,60,60 }, charStr.c_str())) {
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
					if (GuiButton({ (float)(GetScreenWidth() / 2) + (60 * j),(float)(GetScreenHeight() / 2) - 40,60,60 }, charStr.c_str())) {
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
			else if (selectStage == 0) {

				midiLoaded = false;
				float curSong = 0.0f;
				for (Song song : songList.songs) {
					if (GuiButton({ (float)GetScreenWidth() - 100,0,100,60}, "Keybinds")) {
						selectStage = -1;
					}
					if (GuiButton({ 0,0 + (60 * curSong),300,60 }, "")) {
						curPlayingSong = (int)curSong;
						selectStage = 1;
					}
					if (GuiButton({ (float)GetScreenWidth() - 100,60,100,60 }, "Fullscreen")) {
						windowToggle = !windowToggle;
						ToggleBorderlessWindowed();
						if (windowToggle) {
							SetWindowPosition(50, 50);
							SetWindowSize(1600, 800);
						}
						else {
							SetWindowSize(1920, 1080);
						};
					}
					DrawTextureEx(song.albumArt, Vector2{ 5,(60 * curSong) + 5 }, 0.0f, 0.1f, RAYWHITE);

					DrawText(song.title.c_str(), 60, (60 * curSong) + 5, 20, BLACK);
					DrawText(song.artist.c_str(), 60, (60 * curSong) + 25, 16, BLACK);
					curSong++;
				}
			}
			else if (selectStage == 1) {
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
					if (GuiButton({ 0,0,60,60 }, "<")) {
						selectStage = 0;
					}
					for (int i = 0; i < 4; i++) {
						if (songList.songs[curPlayingSong].parts[i]->hasPart) {
							if (GuiButton({ 0,60 + (60 * (float)i),300,60 }, songPartsList[i].c_str())) {
								instrument = i;
								selectStage = 2;
							}
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
						if (GuiButton({ 0,60 + (60 * (float)i),300,60 }, diffList[i].c_str())) {
							diff = i;
							selectStage = 0;
							isPlaying = true;
						}
					}
				}
			}
		}
		else {
			if (GuiButton({ 0,0,60,60 }, "<")) {
				isPlaying = false;
				streamsLoaded = false;
			}
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
			if(midiLoaded==true){
				double musicTime = (double)GetMusicTimePlayed(loadedStreams[0]);
				if (musicTime < 5.0) {
					DrawText(songList.songs[curPlayingSong].title.c_str(), 5, 65, 30, WHITE);
					DrawText(songList.songs[curPlayingSong].artist.c_str(), 5, 100, 24, WHITE);
				}
				BeginMode3D(camera);
				if (diff == 3) {
					DrawModel(expertHighway, Vector3{ 0,0,0 }, 1.0f, WHITE);
					for (int i = 0; i < 5; i++) {
						if (heldFrets[i] == true) {
							DrawModel(smasherPressed, Vector3{ diffDistance - (i + 2), 0, 0}, 1.0f, WHITE);
							// DrawCube(Vector3{ diffDistance - (1.0f * i),0,2.5f }, 0.75, 0.125, 0.25, Color{ 84,8,207,255 });
						}
						else {
							DrawModel(smasherReg, Vector3{ diffDistance - (i + 2), 0, 0}, 1.0f, WHITE);
							// DrawCube(Vector3{ diffDistance - (1.0f * i),0,2.5f }, 0.75, 0.125, 0.25, Color{ 35,21,69,255 });
						}
					}
					for (int i = 0; i < 4; i++) {
						DrawLine3D(Vector3{ lineDistance - i, 0.05f, 0 }, Vector3{ lineDistance - i, 0.05f, 20 }, Color{ 255,255,255,255 });
					}
				}
				else {
					DrawModel(emhHighway, Vector3{ 0,0,0 }, 1.0f, WHITE);
					for (int i = 0; i < 4; i++) {
						if (heldFrets[i] == true) {
							DrawModel(smasherPressed, Vector3{ diffDistance - (i + 2), 0, 0 }, 1.0f, WHITE);
						}
						else {
							DrawModel(smasherReg, Vector3{ diffDistance - (i + 2), 0, 0 }, 1.0f, WHITE);
							
						}
					}
					for (int i = 0; i < 3; i++) {
						DrawLine3D(Vector3{ lineDistance - i, 0.05f, 0 }, Vector3{ lineDistance - i, 0.05f, 20 }, Color{ 255,255,255,255 });
					}
				}
				
				
				// DrawTriangle3D(Vector3{ 2.5f,0.0f,0.0f }, Vector3{ -2.5f,0.0f,0.0f }, Vector3{ -2.5f,0.0f,20.0f }, BLACK);
				// DrawTriangle3D(Vector3{ 2.5f,0.0f,0.0f }, Vector3{ -2.5f,0.0f,20.0f }, Vector3{ 2.5f,0.0f,20.0f }, BLACK);
				
				DrawLine3D(Vector3{ 2.5f, 0.05f, 2.0f }, Vector3{ -2.5f, 0.05f, 2.0f}, WHITE);
				Chart& dmsExpert = songList.songs[curPlayingSong].parts[instrument]->charts[diff];
				if (dmsExpert.odPhrases.size() > 0 && curODPhrase < dmsExpert.odPhrases.size()) {
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
					if (dmsExpert.odPhrases.size() > 0 && curODPhrase < dmsExpert.odPhrases.size()) {
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

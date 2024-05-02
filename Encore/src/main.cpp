
#if defined(WIN32) && defined(NDEBUG)
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif
#include "rapidjson/filewritestream.h"
#include "raylib.h"
#include <vector>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include "song/song.h"
#include "song/songlist.h"
#include "audio/audio.h"
#include "game/arguments.h"
#include "game/utility.h"
#include "game/player.cpp"
#include "game/lerp.h"
#include "game/keybinds.h"
#include "game/assets.cpp"
#include "game/settings.h"
#include "game/gameplay.cpp"
#include "raygui.h"
#include <thread>
#include <cstdlib>




vector<std::string> ArgumentList::arguments;

static bool compareNotes(const Note& a, const Note& b) {
	return a.time < b.time;
}

bool midiLoaded = false;
bool isPlaying = false;


int curODPhrase = 0;
int curBeatLine = 0;
int curBPM = 0;
int selLane = 0;
bool selSong = false;
bool songsLoaded= false;
int songSelectOffset = 0;
bool changingKey = false;
bool changingOverdrive = false;
double startedPlayingSong = 0.0;
Vector2 viewScroll = { 0,0 };
Rectangle view = { 0 };


std::string trackSpeedButton;

enum Screens {
    SONG_LOADING_SCREEN,
	MENU,
	SONG_SELECT,
	INSTRUMENT_SELECT,
	DIFFICULTY_SELECT,
	GAMEPLAY,
	RESULTS,
	SETTINGS
};

int currentScreen = MENU;


SongList songList;
Settings settings;
Assets assets;
double lastAxesTime = 0.0;
std::vector<float> axesValues{0.0f,0.0f,0.0f,0.0f,0.0f,0.0f};
std::vector<int> buttonValues{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
std::vector<float> axesValues2{ 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f };
int pressedGamepadInput = -999;
int axisDirection = -1;
int controllerID = -1;

Lerp lerpCtrl = Lerp();
void createLerp(std::string key, easing_functions ease, float duration, bool startAutomatically = true);
void removeLerp(std::string key);
void startLerp(std::string key);
void updateStates();
LerpState getState(std::string key);

static void SwitchScreen(Screens screen) {
	currentScreen = screen;
	switch (screen) {
		case MENU:
			// reset lerps
			lerpCtrl.removeLerp("MENU_LOGO");
			break;

		case SONG_SELECT:
			break;
        case SONG_LOADING_SCREEN:
            break;
        case INSTRUMENT_SELECT:
            break;
        case DIFFICULTY_SELECT:
            break;
        case GAMEPLAY:
            break;
        case RESULTS:
            break;
        case SETTINGS:
            break;
    }
}

int minWidth = 640;
int minHeight = 480;


void DrawTopOvershell(int TopOvershell) {

    DrawRectangle(0,0,(GetScreenWidth()), TopOvershell+6,WHITE);
    DrawRectangle(0,0,(GetScreenWidth()), TopOvershell,BLACK);
}

void DrawBottomOvershell() {
    int BottomOvershell = GetScreenHeight() - 120;
    DrawRectangle(0,BottomOvershell-6,(GetScreenWidth()), GetScreenHeight(),WHITE);
    DrawRectangle(0,BottomOvershell,(GetScreenWidth()), GetScreenHeight(),BLACK);
}

void DrawBottomBottomOvershell() {
    int BottomBottomOvershell = GetScreenHeight() - 80;
    DrawRectangle(0,BottomBottomOvershell-6,(GetScreenWidth()), GetScreenHeight(),WHITE);
    DrawRectangle(0,BottomBottomOvershell,(GetScreenWidth()), GetScreenHeight(),BLACK);
}

int main(int argc, char* argv[])
{
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	SetConfigFlags(FLAG_VSYNC_HINT);
	
	SetTraceLogLevel(LOG_NONE);

	// 800 , 600
	InitWindow(1, 1, "Encore");
	SetWindowSize((int)GetMonitorWidth(GetCurrentMonitor()) * 0.75, (int)GetMonitorHeight(GetCurrentMonitor()) * 0.75);
	SetWindowPosition((int)(GetMonitorWidth(GetCurrentMonitor())*0.5)-(GetMonitorWidth(GetCurrentMonitor())*0.375), (int)(GetMonitorHeight(GetCurrentMonitor()) * 0.5) - (GetMonitorHeight(GetCurrentMonitor()) * 0.375));
	
	bool windowToggle = true;
	ArgumentList::InitArguments(argc, argv);

	std::string FPSCapStringVal = ArgumentList::GetArgValue("fpscap");
	int targetFPSArg = 0;

	if (!FPSCapStringVal.empty())
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
	std::vector<std::string> songPartsList{ "Drums","Bass","Guitar","Vocals" };
	std::vector<std::string> diffList{ "Easy","Medium","Hard","Expert" };
	TraceLog(LOG_INFO, "Target FPS: %d", targetFPS);


	InitAudioDevice();

	SetExitKey(0);

	Camera3D camera = { 0 };

	// Y UP!!!! REMEMBER!!!!!!
	//							  x,    y,     z
							// 0.0f, 5.0f, -3.5f
	//								 6.5f
	camera.position = Vector3{ 0.0f, 7.0f, -10.0f };
	// 0.0f, 0.0f, 6.5f
	camera.target = Vector3{ 0.0f, 0.0f, 13.0f };

	camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
	camera.fovy = 34.5f;
	camera.projection = CAMERA_PERSPECTIVE;


	std::filesystem::path executablePath(GetApplicationDirectory());

	std::filesystem::path directory = executablePath.parent_path();


	if (std::filesystem::exists(directory / "keybinds.json")) {
		settings.migrateSettings(directory / "keybinds.json", directory / "settings.json");
	}
	settings.loadSettings(directory / "settings.json");
	trackSpeedButton = "Track Speed " + truncateFloatString(settings.trackSpeedOptions[settings.trackSpeed]) + "x";


	ChangeDirectory(GetApplicationDirectory());
	assets.loadAssets(directory);
    SetWindowIcon(assets.icon);
	GLFWkeyfun origKeyCallback = glfwSetKeyCallback(glfwGetCurrentContext(), keyCallback);
	GLFWgamepadstatefun origGamepadCallback = glfwSetGamepadStateCallback(gamepadStateCallback);
	glfwSetKeyCallback(glfwGetCurrentContext(), origKeyCallback);
	glfwSetGamepadStateCallback(origGamepadCallback);
    // GuiLoadStyle((directory / "Assets/ui/encore.rgs").string().c_str());

    GuiSetStyle(BUTTON, BASE, 0x181827FF);
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt(ColorBrightness(player.accentColor, -0.5)));
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, ColorToInt(ColorBrightness(player.accentColor, -0.25)));
    GuiSetStyle(BUTTON, BORDER, 0xFFFFFFFF);
    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, 0xFFFFFFFF);
    GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, 0xFFFFFFFF);
	GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0x505050ff);
    GuiSetStyle(DEFAULT, TEXT, 0xcbcbcbFF);
    GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, 0xFFFFFFFF);
    GuiSetStyle(BUTTON, TEXT_COLOR_PRESSED, 0xFFFFFFFF);
	GuiSetStyle(DEFAULT, TEXT_SIZE, 28);


	GuiSetStyle(TOGGLE, BASE, 0x181827FF);
	GuiSetStyle(TOGGLE, BASE_COLOR_FOCUSED, ColorToInt(ColorBrightness(player.accentColor, -0.5)));
	GuiSetStyle(TOGGLE, BASE_COLOR_PRESSED, ColorToInt(ColorBrightness(player.accentColor, -0.25)));
	GuiSetStyle(TOGGLE, BORDER, 0xFFFFFFFF);
	GuiSetStyle(TOGGLE, BORDER_COLOR_FOCUSED, 0xFFFFFFFF);
	GuiSetStyle(TOGGLE, BORDER_COLOR_PRESSED, 0xFFFFFFFF);
	GuiSetStyle(TOGGLE, TEXT_COLOR_FOCUSED, 0xFFFFFFFF);
	GuiSetStyle(TOGGLE, TEXT_COLOR_PRESSED, 0xFFFFFFFF);
	GuiSetFont(assets.rubik32);


	while (!WindowShouldClose())
	{
		double curTime = GetTime();
		if (GetScreenWidth() < minWidth) {
			if (GetScreenHeight() < minHeight)
				SetWindowSize(minWidth, minHeight);
			else
				SetWindowSize(minWidth, GetScreenHeight());
		}
		if (GetScreenHeight() < minHeight) {
			if (GetScreenWidth() < minWidth)
				SetWindowSize(minWidth, minHeight);
			else
				SetWindowSize(GetScreenWidth(), minHeight);
		}
		float diffDistance = diff == 3 ? 2.0f : 1.5f;
		float lineDistance = diff == 3 ? 1.5f : 1.0f;
		BeginDrawing();

		ClearBackground(DARKGRAY);

		lerpCtrl.updateStates();

		switch (currentScreen) {
            case SONG_LOADING_SCREEN : {
                if (!songsLoaded) {
                    DrawTextRHDI(TextFormat("Songs Loaded: %01i", songList.songCount), (float) GetScreenWidth() / 2 -
                                                                                       MeasureTextRHDI(TextFormat(
                                                                                               "Songs Loaded: %01i",
                                                                                               songList.songCount)) / 2,
                                 (float) GetScreenHeight() / 2, WHITE);
                    DrawTextRHDI(TextFormat("Bad Songs: %01i", songList.badSongCount), (float) GetScreenWidth() / 2 -
                                                                                       MeasureTextRHDI(TextFormat(
                                                                                               "Bad Songs: %01i",
                                                                                               songList.badSongCount)) /
                                                                                       2,
                                 (float) GetScreenHeight() / 2 + 50, WHITE);
                    DrawTextRHDI(TextFormat("Folders: %01i", songList.directoryCount), (float) GetScreenWidth() / 2 -
                                                                                       MeasureTextRHDI(TextFormat(
                                                                                               "Folders: %01i",
                                                                                               songList.directoryCount)) /
                                                                                       2,
                                 (float) GetScreenHeight() / 2 - 50, WHITE);

                    LoadSongs(settings.songPaths);

                    songsLoaded = true;
                }
                for (Song& song : songList.songs) {
                    song.titleScrollTime = GetTime();
                    song.titleTextWidth = MeasureTextRubik(song.title.c_str(), 24);
                    song.artistScrollTime = GetTime();
                    song.artistTextWidth = MeasureTextRubik(song.artist.c_str(), 20);
                }

                currentScreen = SONG_SELECT;
                break;
            }
			case MENU: {
				lerpCtrl.createLerp("MENU_LOGO", EaseOutCubic, 1.5f);
                DrawTextureEx(assets.encoreWhiteLogo, {(float)GetScreenWidth()/2 - assets.encoreWhiteLogo.width/4, (float)lerpCtrl.getState("MENU_LOGO").value * ((float)GetScreenHeight()/5 - assets.encoreWhiteLogo.height/4)},0,0.5, WHITE);

                if (GuiButton({ ((float)GetScreenWidth() / 2) - 100,((float)GetScreenHeight() / 2) - 120,200, 60 }, "Play")) {
					SwitchScreen(SONG_LOADING_SCREEN);
				}
				if (GuiButton({ ((float)GetScreenWidth() / 2) - 100,((float)GetScreenHeight() / 2) - 30,200, 60 }, "Options")) {
					glfwSetGamepadStateCallback(gamepadStateCallbackSetControls);
					SwitchScreen(SETTINGS);
				}
				if (GuiButton({ ((float)GetScreenWidth() / 2) - 100,((float)GetScreenHeight() / 2) + 60,200, 60 }, "Quit")) {
					exit(0);
				}
                if (GuiButton({(float)GetScreenWidth()-60, (float)GetScreenHeight()-60, 60,60}, "")) {
                    OpenURL("https://github.com/Encore-Developers/Encore-Raylib");
                }

                if (GuiButton({(float)GetScreenWidth()-120, (float)GetScreenHeight()-60, 60,60}, "")) {
                    OpenURL("https://discord.gg/GhkgVUAC9v");
                }
                if (GuiButton({(float)GetScreenWidth()-180, (float)GetScreenHeight()-120, 180,60}, "Rescan Songs")) {
                    songsLoaded = false;
                }
                DrawTextureEx(assets.github, {(float)GetScreenWidth()-54, (float)GetScreenHeight()-54}, 0, 0.2, WHITE);
                DrawTextureEx(assets.discord, {(float)GetScreenWidth()-113, (float)GetScreenHeight()-48}, 0, 0.075, WHITE);
				break;
			}
			case SETTINGS: {
				if (settings.controllerType == -1 && controllerID != -1) {
					std::string gamepadName = std::string(glfwGetGamepadName(controllerID));
					settings.controllerType = getControllerType(gamepadName);
				}
				if (GuiButton({ ((float)GetScreenWidth() / 2) - 350,((float)GetScreenHeight() - 60),100,60 }, "Cancel") && !(changingKey || changingOverdrive)) {
					glfwSetGamepadStateCallback(origGamepadCallback);
					settings.keybinds4K = settings.prev4K;
					settings.keybinds5K = settings.prev5K;
					settings.keybinds4KAlt = settings.prev4KAlt;
					settings.keybinds5KAlt = settings.prev5KAlt;
					settings.keybindOverdrive = settings.prevOverdrive;
					settings.keybindOverdriveAlt = settings.prevOverdriveAlt;

					settings.controller4K = settings.prevController4K;
					settings.controller4KAxisDirection = settings.prevController4KAxisDirection;
					settings.controller5K = settings.prevController5K;
					settings.controller5KAxisDirection = settings.prevController5KAxisDirection;
					settings.controllerOverdrive = settings.prevControllerOverdrive;
					settings.controllerOverdriveAxisDirection = settings.prevControllerOverdriveAxisDirection;
					settings.controllerType = settings.prevControllerType;

                    settings.highwayLengthMult = settings.prevHighwayLengthMult;
					settings.trackSpeed = settings.prevTrackSpeed;
					settings.inputOffsetMS = settings.prevInputOffsetMS;
					settings.avOffsetMS = settings.prevAvOffsetMS;
					settings.missHighwayDefault = settings.prevMissHighwayColor;
					settings.mirrorMode = settings.prevMirrorMode;

					settings.saveSettings(directory / "settings.json");

					SwitchScreen(MENU);
				}
				if (GuiButton({ ((float)GetScreenWidth() / 2) + 250,((float)GetScreenHeight() - 60),100,60 }, "Apply") && !(changingKey || changingOverdrive)) {
					glfwSetGamepadStateCallback(origGamepadCallback);
					settings.prev4K = settings.keybinds4K;
					settings.prev5K = settings.keybinds5K;
					settings.prev4KAlt = settings.keybinds4KAlt;
					settings.prev5KAlt = settings.keybinds5KAlt;
					settings.prevOverdrive = settings.keybindOverdrive;
					settings.prevOverdriveAlt = settings.keybindOverdriveAlt;

					settings.prevController4K = settings.controller4K;
					settings.prevController4KAxisDirection = settings.controller4KAxisDirection;
					settings.prevController5K = settings.controller5K;
					settings.prevController5KAxisDirection = settings.controller5KAxisDirection;
					settings.prevControllerOverdrive = settings.controllerOverdrive;
					settings.prevControllerOverdriveAxisDirection = settings.controllerOverdriveAxisDirection;
					settings.prevControllerType = settings.controllerType;

                    settings.prevHighwayLengthMult = settings.highwayLengthMult;
					settings.prevTrackSpeed = settings.trackSpeed;
					settings.prevInputOffsetMS = settings.inputOffsetMS;
					settings.prevAvOffsetMS = settings.avOffsetMS;
					settings.prevMissHighwayColor = settings.missHighwayDefault;
					settings.prevMirrorMode = settings.mirrorMode;

					settings.saveSettings(directory / "settings.json");

					SwitchScreen(MENU);
				}
				static int selectedTab = 0;
				GuiToggleGroup({ 0,0,(float)GetScreenWidth() / 3,60 }, "Main;Keyboard Controls;Gamepad Controls", &selectedTab);
				if (selectedTab == 0) { //Main settings tab
					if (GuiButton({ (float)GetScreenWidth() / 2 - 125,(float)GetScreenHeight() / 2 - 320,250,60 }, "")) {
						if (settings.trackSpeed == settings.trackSpeedOptions.size() - 1) settings.trackSpeed = 0; else settings.trackSpeed++;
						trackSpeedButton = "Track Speed " + truncateFloatString(settings.trackSpeedOptions[settings.trackSpeed]) + "x";
					}
					DrawTextRubik(trackSpeedButton.c_str(), (float)GetScreenWidth() / 2 - MeasureTextRubik(trackSpeedButton.c_str(), 20) / 2, (float)GetScreenHeight() / 2 - 300, 20, WHITE);

					GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
					float avOffsetFloat = (float)settings.avOffsetMS;
                    float lengthSetting = settings.highwayLengthMult;
					DrawTextRubik("A/V Offset", (float)GetScreenWidth() / 2 - MeasureTextRubik("A/V Offset", 20) / 2, (float)GetScreenHeight() / 2 - 240, 20, WHITE);
					DrawTextRubik(" -500 ", (float)GetScreenWidth() / 2 - 125 - MeasureTextRubik(" -500 ", 20), (float)GetScreenHeight() / 2 - 210, 20, WHITE);
					DrawTextRubik(" 500 ", (float)GetScreenWidth() / 2 + 125, (float)GetScreenHeight() / 2 - 210, 20, WHITE);
					if (GuiSliderBar({ (float)GetScreenWidth() / 2 - 125,(float)GetScreenHeight() / 2 - 220,250,40 }, "", "", &avOffsetFloat, -500.0f, 500.0f)) {
						settings.avOffsetMS = (int)avOffsetFloat;
					}

					if (GuiButton({ (float)GetScreenWidth() / 2 - 125 - MeasureTextRubik(" -500 ", 20) - 60,(float)GetScreenHeight() / 2 - 230,60,60 }, "-1")) {
						settings.avOffsetMS--;
					}
					if (GuiButton({ (float)GetScreenWidth() / 2 + 125 + MeasureTextRubik(" -500 ", 20) ,(float)GetScreenHeight() / 2 - 230,60,60 }, "+1")) {
						settings.avOffsetMS++;
					}
                    DrawTextRubik(TextFormat("%01i ms",settings.avOffsetMS), (float)GetScreenWidth() / 2 - (MeasureTextRubik(TextFormat("%01i ms",settings.avOffsetMS), 20) / 2), (float)GetScreenHeight() / 2 - 210, 20, BLACK);


                    float lengthHeight = ((float)GetScreenHeight() / 2 )- 60;
                    DrawTextRubik("Highway Length", (float)GetScreenWidth() / 2 - MeasureTextRubik("Highway Length", 20) / 2, lengthHeight - 20, 20, WHITE);
                    DrawTextRubik(" 0.25 ", (float)GetScreenWidth() / 2 - 125 - MeasureTextRubik(" 0.25 ", 20), lengthHeight+10, 20, WHITE);
                    DrawTextRubik(" 2.50 ", (float)GetScreenWidth() / 2 + 125, lengthHeight+10, 20, WHITE);
                    if (GuiSliderBar({ (float)GetScreenWidth() / 2 - 125,lengthHeight,250,40 }, "", "", &lengthSetting, 0.25f, 2.5f)) {
                        settings.highwayLengthMult = lengthSetting;
                    }
                    if (GuiButton({ (float)GetScreenWidth() / 2 - 125 - MeasureTextRubik(" 0.25 ", 20) - 60,lengthHeight-10,60,60 }, "-0.25")) {
                        settings.highwayLengthMult-= 0.25;
                    }
                    if (GuiButton({ (float)GetScreenWidth() / 2 + 125 + MeasureTextRubik(" 2.50 ", 20) ,lengthHeight-10,60,60 }, "+0.25")) {
                        settings.highwayLengthMult+=0.25;
                    }
                    DrawTextRubik(TextFormat("%1.2fx",settings.highwayLengthMult), (float)GetScreenWidth() / 2 - (MeasureTextRubik(TextFormat("%1.2f",settings.highwayLengthMult), 20) / 2), lengthHeight+10, 20, BLACK);


                    if (GuiButton({ (float)GetScreenWidth() / 2 - 125, (float)GetScreenHeight() / 2,250,60 }, TextFormat("Miss Highway Color: %s", MissHighwayColor ? "True" : "False"))) {
						settings.missHighwayDefault = !settings.missHighwayDefault;
						MissHighwayColor = settings.missHighwayDefault;
                    }
					if (GuiButton({ (float)GetScreenWidth() / 2 - 125, (float)GetScreenHeight() / 2 + 90,250,60 }, TextFormat("Mirror mode: %s", settings.mirrorMode ? "True" : "False"))) {
						settings.mirrorMode = !settings.mirrorMode;
					}
					float inputOffsetFloat = (float)settings.inputOffsetMS;
					DrawTextRubik("Input Offset", (float)GetScreenWidth() / 2 - MeasureTextRubik("Input Offset", 20) / 2, (float)GetScreenHeight() / 2 - 160, 20, WHITE);
					DrawTextRubik(" -500 ", (float)GetScreenWidth() / 2 - 125 - MeasureTextRubik(" -500 ", 20), (float)GetScreenHeight() / 2 - 130, 20, WHITE);
					DrawTextRubik(" 500 ", (float)GetScreenWidth() / 2 + 125, (float)GetScreenHeight() / 2 - 130, 20, WHITE);
					if (GuiSliderBar({ (float)GetScreenWidth() / 2 - 125,(float)GetScreenHeight() / 2 - 140,250,40 }, "", "", &inputOffsetFloat, -500.0f, 500.0f)) {
						settings.inputOffsetMS = (int)inputOffsetFloat;
					}
					if (GuiButton({ (float)GetScreenWidth() / 2 - 125 - MeasureTextRubik(" -500 ", 20) - 60,(float)GetScreenHeight() / 2 - 150,60,60 }, "-1")) {
						settings.inputOffsetMS--;
					}
					if (GuiButton({ (float)GetScreenWidth() / 2 + 125 + MeasureTextRubik(" -500 ", 20),(float)GetScreenHeight() / 2 - 150,60,60 }, "+1")) {
						settings.inputOffsetMS++;
					}
					DrawTextRubik(TextFormat("%01i ms",settings.inputOffsetMS), (float)GetScreenWidth() / 2 - (MeasureTextRubik(TextFormat("%01i ms",settings.inputOffsetMS), 20) / 2), (float)GetScreenHeight() / 2 - 130, 20, BLACK);
					GuiSetStyle(DEFAULT, TEXT_SIZE, 28);
				}
				else if (selectedTab == 1) { //Keyboard bindings tab
					GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
					for (int i = 0; i < 5; i++) {
						float j = i - 2.0f;
						if (GuiButton({ ((float)GetScreenWidth() / 2) - 40 + (80 * j),120,80,60 }, getKeyStr(settings.keybinds5K[i]).c_str())) {
							settings.changing4k = false;
							settings.changingAlt = false;
							selLane = i;
							changingKey = true;
						}
						if (GuiButton({ ((float)GetScreenWidth() / 2) - 40 + (80 * j),180,80,60 }, getKeyStr(settings.keybinds5KAlt[i]).c_str())) {
							settings.changing4k = false;
							settings.changingAlt = true;
							selLane = i;
							changingKey = true;
						}
					}
					for (int i = 0; i < 4; i++) {
						float j = i - 1.5f;
						if (GuiButton({ ((float)GetScreenWidth() / 2) - 40 + (80 * j),300,80,60 }, getKeyStr(settings.keybinds4K[i]).c_str())) {
							settings.changingAlt = false;
							settings.changing4k = true;
							selLane = i;
							changingKey = true;
						}
						if (GuiButton({ ((float)GetScreenWidth() / 2) - 40 + (80 * j),360,80,60 }, getKeyStr(settings.keybinds4KAlt[i]).c_str())) {
							settings.changingAlt = true;
							settings.changing4k = true;
							selLane = i;
							changingKey = true;
						}
					}
					if (GuiButton({ ((float)GetScreenWidth() / 2) - 100,480,80,60 }, getKeyStr(settings.keybindOverdrive).c_str())) {
						settings.changingAlt = false;
						changingKey = false;
						changingOverdrive = true;
					}
					if (GuiButton({ ((float)GetScreenWidth() / 2) + 20,480,80,60 }, getKeyStr(settings.keybindOverdriveAlt).c_str())) {
						settings.changingAlt = true;
						changingKey = false;
						changingOverdrive = true;
					}
					if (changingKey) {
						std::vector<int>& bindsToChange = settings.changingAlt ? (settings.changing4k ? settings.keybinds4KAlt : settings.keybinds5KAlt) : (settings.changing4k ? settings.keybinds4K : settings.keybinds5K);
						DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), { 0,0,0,200 });
						std::string keyString = (settings.changing4k ? "4k" : "5k");
						std::string altString = (settings.changingAlt ? " alt" : "");
						std::string changeString = "Press a key for " + keyString + altString + " lane ";
						DrawTextRubik(changeString.c_str(), (GetScreenWidth() - MeasureTextRubik(changeString.c_str(), 20)) / 2, GetScreenHeight() / 2 - 30, 20, WHITE);
						DrawTextRubik("Or press escape to remove bound key", (GetScreenWidth() - MeasureTextRubik("Or press escape to remove bound key", 20)) / 2, GetScreenHeight() / 2 + 30, 20, WHITE);
						int pressedKey = GetKeyPressed();
						if (pressedKey != 0) {
							if (pressedKey == KEY_ESCAPE)
								pressedKey = -1;
							bindsToChange[selLane] = pressedKey;
							selLane = 0;
							changingKey = false;
						}
					}
					if (changingOverdrive) {
						DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), { 0,0,0,200 });
						std::string altString = (settings.changingAlt ? " alt" : "");
						std::string changeString = "Press a key for " + altString + " overdrive";
						DrawTextRubik(changeString.c_str(), (GetScreenWidth() - MeasureTextRubik(changeString.c_str(), 20)) / 2, GetScreenHeight() / 2 - 30, 20, WHITE);
						DrawTextRubik("Or press escape to remove bound key", (GetScreenWidth() - MeasureTextRubik("Or press escape to remove bound key", 20)) / 2, GetScreenHeight() / 2 + 30, 20, WHITE);
						int pressedKey = GetKeyPressed();
						if (pressedKey != 0) {
							if (pressedKey == KEY_ESCAPE)
								pressedKey = -1;
							if (settings.changingAlt)
								settings.keybindOverdriveAlt = pressedKey;
							else
								settings.keybindOverdrive = pressedKey;
							changingOverdrive = false;
						}
					}
					GuiSetStyle(DEFAULT, TEXT_SIZE, 28);
				}
				else if (selectedTab == 2) { //Controller bindings tab
					GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
					for (int i = 0; i < 5; i++) {
						float j = i - 2.0f;
						if (GuiButton({ ((float)GetScreenWidth() / 2) - 40 + (80 * j),240,80,60 }, getControllerStr(controllerID, settings.controller5K[i], settings.controllerType, settings.controller5KAxisDirection[i]).c_str())) {
							settings.changing4k = false;
							selLane = i;
							changingKey = true;
						}
					}
					for (int i = 0; i < 4; i++) {
						float j = i - 1.5f;
						if (GuiButton({ ((float)GetScreenWidth() / 2) - 40 + (80 * j),360,80,60 }, getControllerStr(controllerID, settings.controller4K[i], settings.controllerType, settings.controller4KAxisDirection[i]).c_str())) {
							settings.changing4k = true;
							selLane = i;
							changingKey = true;
						}
					}
					if (GuiButton({ ((float)GetScreenWidth() / 2) - 40,480,80,60 }, getControllerStr(controllerID, settings.controllerOverdrive, settings.controllerType, settings.controllerOverdriveAxisDirection).c_str())) {
						changingKey = false;
						changingOverdrive = true;
					}
					if (changingKey) {
						std::vector<int>& bindsToChange = (settings.changing4k ? settings.controller4K : settings.controller5K);
						std::vector<int>& directionToChange = (settings.changing4k ? settings.controller4KAxisDirection : settings.controller5KAxisDirection);
						DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), { 0,0,0,200 });
						std::string keyString = (settings.changing4k ? "4k" : "5k");
						std::string changeString = "Press a button/axis for controller " + keyString + " lane " + std::to_string(selLane + 1);
						DrawTextRubik(changeString.c_str(), (GetScreenWidth() - MeasureTextRubik(changeString.c_str(), 20)) / 2, GetScreenHeight() / 2 - 30, 20, WHITE);
						DrawTextRubik("Or press escape to cancel", (GetScreenWidth() - MeasureTextRubik("Or press escape to cancel", 20)) / 2, GetScreenHeight() / 2 + 30, 20, WHITE);
						if (pressedGamepadInput != -999) {
							bindsToChange[selLane] = pressedGamepadInput;
							if (pressedGamepadInput < 0) {
								directionToChange[selLane] = axisDirection;
							}
							selLane = 0;
							changingKey = false;
							pressedGamepadInput = -999;
						}
					}
					if (changingOverdrive) {
						DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), { 0,0,0,200 });
						std::string changeString = "Press a button/axis for controller overdrive";
						DrawTextRubik(changeString.c_str(), (GetScreenWidth() - MeasureTextRubik(changeString.c_str(), 20)) / 2, GetScreenHeight() / 2 - 30, 20, WHITE);
						DrawTextRubik("Or press escape to cancel", (GetScreenWidth() - MeasureTextRubik("Or press escape to cancel", 20)) / 2, GetScreenHeight() / 2 + 30, 20, WHITE);

						if (pressedGamepadInput != -999) {
							settings.controllerOverdrive = pressedGamepadInput;
							if (pressedGamepadInput < 0) {
								settings.controllerOverdriveAxisDirection = axisDirection;
							}
							changingOverdrive = false;
							pressedGamepadInput = -999;
						}
					}
					if (IsKeyPressed(KEY_ESCAPE)) {
						pressedGamepadInput = -999;
						changingKey = false;
						changingOverdrive = false;
					}
					GuiSetStyle(DEFAULT, TEXT_SIZE, 28);
				}
				break;
			}
			case SONG_SELECT: {
				streamsLoaded = false;
				midiLoaded = false;
				isPlaying = false;
				overdrive = false;
				curNoteIdx = { 0,0,0,0,0 };
				curODPhrase = 0;
				curBeatLine = 0;
				curBPM = 0;
				instrument = 0;
				diff = 0;
                int randSong = rand()%(songList.songCount-0+1)+0;
                int selectedSongInt = curPlayingSong;
                Song selectedSong = songList.songs[selectedSongInt];

                SetTextureWrap(selectedSong.albumArtBlur, TEXTURE_WRAP_REPEAT);
                SetTextureFilter(selectedSong.albumArtBlur, TEXTURE_FILTER_ANISOTROPIC_16X);
                if (selSong){
                    DrawTexturePro(selectedSong.albumArtBlur, Rectangle{0, 0, (float) selectedSong.albumArt.width,
                                                                        (float) selectedSong.albumArt.width},
                                   Rectangle{(float) GetScreenWidth() / 2, -((float) GetScreenHeight() * 2),
                                             (float) GetScreenWidth() * 2, (float) GetScreenWidth() * 2}, {0, 0}, 45,
                                   WHITE);
                }
                else {

                    Song art = songList.songs[randSong];
                    DrawTexturePro(art.albumArtBlur, Rectangle{0, 0, (float) art.albumArt.width,
                                                                        (float) art.albumArt.width},
                                   Rectangle{(float) GetScreenWidth() / 2, -((float) GetScreenHeight() * 2),
                                             (float) GetScreenWidth() * 2, (float) GetScreenWidth() * 2}, {0, 0}, 45,
                                   WHITE);
                }

                Vector2 mouseWheel = GetMouseWheelMoveV();
				if (songSelectOffset <= songList.songs.size() + 2 - (GetScreenHeight() / 60) && songSelectOffset >= 0) {
					songSelectOffset -= mouseWheel.y;
				}
				if (songSelectOffset < 0) songSelectOffset = 0;
				if (songSelectOffset > songList.songCount + 2 - (GetScreenHeight() / 60)) songSelectOffset = songList.songs.size() + 2 - (GetScreenHeight() / 60);
				if (songSelectOffset >= songList.songCount) songSelectOffset = songList.songCount - 1;


                float RightBorder = ((float)GetScreenWidth()/2)+((float)GetScreenHeight()/1.25f);
                float RightSide = RightBorder >= (float)GetScreenWidth() ? (float)GetScreenWidth() : RightBorder;
                float LeftBorder = ((float)GetScreenWidth()/2)-((float)GetScreenHeight()/1.25f);
                float LeftSide = LeftBorder <= 0 ? 0 : LeftBorder;

                float AlbumArtLeft = RightSide - 400 >= (float)GetScreenWidth()/2 ? RightSide - 300 :RightSide*0.72f;
                float AlbumArtTop = 65;
                float AlbumArtRight = (RightSide - AlbumArtLeft)-6;
                float AlbumArtBottom = (RightSide - AlbumArtLeft)-6;
                float TopOvershell = 110;
                DrawRectangle(LeftSide,0, RightSide - LeftSide, (float)GetScreenHeight(), Color(0,0,0,128));
                DrawTopOvershell(110);
                float TextPlacementTB = (float)GetScreenHeight()*0.005f;
                float TextPlacementLR = (float)GetScreenWidth()*0.10f;
                DrawTextEx(assets.redHatDisplayLarge, "Song Select", {TextPlacementLR, TextPlacementTB}, 100,1, WHITE);

                if (GuiButton({ 0,0,60,60 }, "<")) {
                    for (Song& song : songList.songs) {
                        song.titleXOffset = 0;
                        song.artistXOffset = 0;
                    }
                    SwitchScreen(MENU);
                }
                DrawRectangle(AlbumArtLeft-6,AlbumArtBottom+12,AlbumArtRight+12, GetScreenHeight()-AlbumArtBottom,WHITE);
                DrawRectangle(AlbumArtLeft,AlbumArtBottom,AlbumArtRight, GetScreenHeight()-AlbumArtBottom,GetColor(0x181827FF));
                DrawRectangle(AlbumArtLeft-6,AlbumArtTop-6,AlbumArtRight+12, AlbumArtBottom+12, WHITE);
                DrawRectangle(AlbumArtLeft,AlbumArtTop,AlbumArtRight, AlbumArtBottom,BLACK);


                // right side
                DrawLine(RightSide,0,RightSide,(float)GetScreenHeight(), WHITE);

                // left side
                DrawLine(LeftSide,0,LeftSide,(float)GetScreenHeight(), WHITE);

                // bottom
                DrawLine(0,((float)GetScreenHeight()*0.85),0,((float)GetScreenHeight()*0.85), WHITE);
                if (selSong) {
                    DrawTexturePro(selectedSong.albumArt, Rectangle{0, 0, (float) selectedSong.albumArt.width,
                                                                    (float) selectedSong.albumArt.width},
                                   Rectangle{AlbumArtLeft, AlbumArtTop, AlbumArtRight, AlbumArtBottom}, {0, 0}, 0,
                                   WHITE);
                } else {
                    Song art = songList.songs[randSong];
                    DrawTexturePro(art.albumArt, Rectangle{0, 0, (float) art.albumArt.width,
                                                                    (float) art.albumArt.width},
                                   Rectangle{AlbumArtLeft, AlbumArtTop, AlbumArtRight, AlbumArtBottom}, {0, 0}, 0,
                                   WHITE);
                }

                for (int i = songSelectOffset; i < songSelectOffset + (GetScreenHeight() / 50) - 2; i++) {
					if (songList.songCount-1 < i)
						break;

					Song& song = songList.songs[i];
                    float buttonX = ((float)GetScreenWidth()/2)-(((float)GetScreenWidth()*0.86f)/2);
					//LerpState state = lerpCtrl.createLerp("SONGSELECT_LERP_" + std::to_string(i), EaseOutCirc, 0.4f);
					float songXPos = LeftSide;//state.value * 500;
					float songYPos = (TopOvershell+6) + (45 * (i - songSelectOffset));

					if (GuiButton(Rectangle{ songXPos, songYPos,(AlbumArtLeft-LeftSide)-6, 45 }, "")) {
						curPlayingSong = i;
                        selSong = true;

					}

					// DrawTexturePro(song.albumArt, Rectangle{ songXPos,0,(float)song.albumArt.width,(float)song.albumArt.height }, { songXPos+5,songYPos + 5,50,50 }, Vector2{ 0,0 }, 0.0f, RAYWHITE);
					int songTitleWidth = (((AlbumArtLeft-LeftSide)-6)/5)*2;

					int songArtistWidth = (((AlbumArtLeft-LeftSide)-6 )/3)-15;

					if (song.titleTextWidth >= songTitleWidth) {
						if (curTime > song.titleScrollTime && curTime < song.titleScrollTime + 3.0)
							song.titleXOffset = 0;

						if (curTime > song.titleScrollTime + 3.0) {
							song.titleXOffset -= 1;

							if (song.titleXOffset < -(song.titleTextWidth - songTitleWidth)) {
								song.titleXOffset = -(song.titleTextWidth - songTitleWidth);
								song.titleScrollTime = curTime + 3.0;
							}
						}
					}
                    Color LightText = Color{203, 203, 203, 255};
					BeginScissorMode(songXPos + 15, songYPos + 10, songTitleWidth, 45);
					DrawTextEx(assets.rubikBold32,song.title.c_str(), {songXPos + 15 + song.titleXOffset, songYPos + 10}, 24,1, i == curPlayingSong && selSong ? WHITE : LightText);
					EndScissorMode();

					if (song.artistTextWidth > songArtistWidth) {
						if (curTime > song.artistScrollTime && curTime < song.artistScrollTime + 3.0)
							song.artistXOffset = 0;

						if (curTime > song.artistScrollTime + 3.0) {
							song.artistXOffset -= 1;
							if (song.artistXOffset < -(song.artistTextWidth - songArtistWidth)) {
								song.artistScrollTime = curTime + 3.0;
							}
						}
					}

                    Color SelectedText = WHITE;
					BeginScissorMode(songXPos + 30 + songTitleWidth, songYPos + 12, songArtistWidth, 45);
					DrawTextRubik(song.artist.c_str(), songXPos + 30 + songTitleWidth + song.artistXOffset, songYPos + 12, 20, i == curPlayingSong && selSong ? WHITE : LightText);
					EndScissorMode();
				}
                // hehe
                DrawBottomOvershell();
                float BottomOvershell = GetScreenHeight() - 120;
                if (selSong) {
                    if (GuiButton(Rectangle{LeftSide, BottomOvershell, 250, 34}, "Play Song")) {
                        curPlayingSong = selectedSongInt;
                        SwitchScreen(INSTRUMENT_SELECT);

                    }
                }
                DrawBottomBottomOvershell();
                break;
			}
			case INSTRUMENT_SELECT: {
                selSong = false;
                Song selectedSong = songList.songs[curPlayingSong];
                SetTextureWrap(selectedSong.albumArtBlur, TEXTURE_WRAP_REPEAT);
                SetTextureFilter(selectedSong.albumArtBlur, TEXTURE_FILTER_ANISOTROPIC_16X);
                DrawTexturePro(selectedSong.albumArtBlur, Rectangle{0,0,(float)selectedSong.albumArt.width,(float)selectedSong.albumArt.width}, Rectangle {(float)GetScreenWidth()/2, -((float)GetScreenHeight()*2),(float)GetScreenWidth() *2,(float)GetScreenWidth() *2}, {0,0}, 45, WHITE);


                float RightBorder = ((float)GetScreenWidth()/2)+((float)GetScreenHeight()/1.25f);
                float RightSide = RightBorder >= (float)GetScreenWidth() ? (float)GetScreenWidth() : RightBorder;
                float LeftBorder = ((float)GetScreenWidth()/2)-((float)GetScreenHeight()/1.25f);
                float LeftSide = LeftBorder <= 0 ? 0 : LeftBorder;

                float AlbumArtLeft = LeftSide;
                float AlbumArtTop = 50;
                float AlbumArtRight = (LeftSide + 120)-6;
                float AlbumArtBottom = (LeftSide + 120
                        )-6;
                DrawRectangle(0,0, (float)GetScreenWidth(), (float)GetScreenHeight(), Color(0,0,0,128));
                DrawTopOvershell(160);

                DrawRectangle(LeftSide,AlbumArtTop,AlbumArtRight+12, AlbumArtBottom+12, WHITE);
                DrawRectangle(LeftSide + 6,AlbumArtTop+6,AlbumArtRight, AlbumArtBottom,BLACK);
                DrawTexturePro(selectedSong.albumArt, Rectangle{0,0,(float)selectedSong.albumArt.width,(float)selectedSong.albumArt.width}, Rectangle {LeftSide + 6, AlbumArtTop+6,AlbumArtRight,AlbumArtBottom}, {0,0}, 0, WHITE);



                float BottomOvershell = GetScreenHeight() - 126;
                float TextPlacementTB = AlbumArtTop;
                float TextPlacementLR = AlbumArtRight + AlbumArtLeft+ 32;
                DrawTextEx(assets.redHatDisplayLarge, songList.songs[curPlayingSong].title.c_str(), {TextPlacementLR, TextPlacementTB-5}, 72,1, WHITE);
                DrawTextEx(assets.rubikBoldItalic32, selectedSong.artist.c_str(), {TextPlacementLR, TextPlacementTB+60}, 40,1,LIGHTGRAY);
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
												for (int loadDiff = 0; loadDiff < 4; loadDiff++) {
													Chart newChart;
													newChart.parseNotes(midiFile, i, midiFile[i], loadDiff, (int)songPart);
													if (!newChart.notes.empty()) {
														songList.songs[curPlayingSong].parts[(int)songPart]->hasPart = true;
													}
													std::sort(newChart.notes.begin(), newChart.notes.end(), compareNotes);
													int noteIdx = 0;
													for (Note& note : newChart.notes) {
														newChart.notes_perlane[note.lane].push_back(noteIdx);
														noteIdx++;
													}
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
						SwitchScreen(SONG_SELECT);
					}
                    // DrawTextRHDI(TextFormat("%s - %s", songList.songs[curPlayingSong].title.c_str(), songList.songs[curPlayingSong].artist.c_str()), 70,7, WHITE);
					for (int i = 0; i < 4; i++) {
						if (songList.songs[curPlayingSong].parts[i]->hasPart) {
							if (GuiButton({ LeftSide,BottomOvershell - 60 - (60 * (float)i),300,60 }, "")) {
								instrument = i;
								int isBassOrVocal = 0;
								if (instrument == 1 || instrument == 3) {
									isBassOrVocal = 1;
								}
								SetShaderValue(assets.odMultShader, assets.isBassOrVocalLoc, &isBassOrVocal, SHADER_UNIFORM_INT);
								SwitchScreen(DIFFICULTY_SELECT);
							}

							DrawTextRubik(songPartsList[i].c_str(), LeftSide + 20, BottomOvershell - 45 - (60 * (float)i), 30, WHITE);
							DrawTextRubik((std::to_string(songList.songs[curPlayingSong].parts[i]->diff + 1) + "/7").c_str(), LeftSide + 220, BottomOvershell - 45 - (60 * (float)i), 30, WHITE);
						}
					}
				}
                DrawBottomOvershell();
                DrawBottomBottomOvershell();
				break;
			}
			case DIFFICULTY_SELECT: {
                Song selectedSong = songList.songs[curPlayingSong];
                SetTextureWrap(selectedSong.albumArtBlur, TEXTURE_WRAP_REPEAT);
                SetTextureFilter(selectedSong.albumArtBlur, TEXTURE_FILTER_ANISOTROPIC_16X);
                DrawTexturePro(selectedSong.albumArtBlur, Rectangle{0,0,(float)selectedSong.albumArt.width,(float)selectedSong.albumArt.width}, Rectangle {(float)GetScreenWidth()/2, -((float)GetScreenHeight()*2),(float)GetScreenWidth() *2,(float)GetScreenWidth() *2}, {0,0}, 45, WHITE);

                float RightBorder = ((float)GetScreenWidth()/2)+((float)GetScreenHeight()/1.25f);
                float RightSide = RightBorder >= (float)GetScreenWidth() ? (float)GetScreenWidth() : RightBorder;
                float LeftBorder = ((float)GetScreenWidth()/2)-((float)GetScreenHeight()/1.25f);
                float LeftSide = LeftBorder <= 0 ? 0 : LeftBorder;

                float AlbumArtLeft = LeftSide;
                float AlbumArtTop = 50;
                float AlbumArtRight = (LeftSide + 120)-6;
                float AlbumArtBottom = (LeftSide + 120
                                       )-6;
                DrawRectangle(0,0, (float)GetScreenWidth(), (float)GetScreenHeight(), Color(0,0,0,128));
                DrawTopOvershell(160);

                DrawRectangle(LeftSide,AlbumArtTop,AlbumArtRight+12, AlbumArtBottom+12, WHITE);
                DrawRectangle(LeftSide + 6,AlbumArtTop+6,AlbumArtRight, AlbumArtBottom,BLACK);
                DrawTexturePro(selectedSong.albumArt, Rectangle{0,0,(float)selectedSong.albumArt.width,(float)selectedSong.albumArt.width}, Rectangle {LeftSide + 6, AlbumArtTop+6,AlbumArtRight,AlbumArtBottom}, {0,0}, 0, WHITE);



                float BottomOvershell = GetScreenHeight() - 126;
                float TextPlacementTB = AlbumArtTop;
                float TextPlacementLR = AlbumArtRight + AlbumArtLeft+ 32;
                DrawTextEx(assets.redHatDisplayLarge, songList.songs[curPlayingSong].title.c_str(), {TextPlacementLR, TextPlacementTB-5}, 72,1, WHITE);
                DrawTextEx(assets.rubikBoldItalic32, selectedSong.artist.c_str(), {TextPlacementLR, TextPlacementTB+60}, 40,1,LIGHTGRAY);
				for (int i = 0; i < 4; i++) {
					if (GuiButton({ 0,0,60,60 }, "<")) {
						SwitchScreen(INSTRUMENT_SELECT);
					}
                    // DrawTextRHDI(TextFormat("%s - %s", songList.songs[curPlayingSong].title.c_str(), songList.songs[curPlayingSong].artist.c_str()), 70,7, WHITE);
					if (!songList.songs[curPlayingSong].parts[instrument]->charts[i].notes.empty()) {
						if (GuiButton({ LeftSide,BottomOvershell - 60 - (60 * (float)i),300,60 }, "")) {
							diff = i;
							SwitchScreen(GAMEPLAY);
							isPlaying = true;
							startedPlayingSong = GetTime();
							glfwSetKeyCallback(glfwGetCurrentContext(), keyCallback);
							glfwSetGamepadStateCallback(gamepadStateCallback);
						}
						DrawTextRubik(diffList[i].c_str(), LeftSide + 150 - (MeasureTextRubik(diffList[i].c_str(), 30) / 2), BottomOvershell - 45 - (60 * (float)i), 30, WHITE);
					}
				}
                DrawBottomOvershell();
                DrawBottomBottomOvershell();
				break;
			}
			case GAMEPLAY: {
                gameplay();
			}
			case RESULTS: {
				int starsval = stars(songList.songs[curPlayingSong].parts[instrument]->charts[diff].baseScore,diff);
                for (int i = 0; i < starsval; i++) {
                    DrawTextureEx(goldStars? assets.goldStar : assets.star, {((float)GetScreenWidth()/2)+(i*40)-100,84},0,0.15f,WHITE);
                }
/*
				char* starsDisplay = (char*)"";
				if (starsval == 5) {
					starsDisplay = (char*)"*****";
				}
				else if (starsval == 4) {
					starsDisplay = (char*)"****";
				}
				else if (starsval == 3) {
					starsDisplay = (char*)"***";
				}
				else if (starsval == 2) {
					starsDisplay = (char*)"**";
				}
				else if (starsval == 1) {
					starsDisplay = (char*)"*";;
				}
				else {
					starsDisplay = (char*)"";
				}*/
                DrawTextRHDI("Results", 70,7, WHITE);
				DrawTextRubik((songList.songs[curPlayingSong].artist + " - " + songList.songs[curPlayingSong].title).c_str(), GetScreenWidth() / 2 - (MeasureTextRubik((songList.songs[curPlayingSong].artist + " - " + songList.songs[curPlayingSong].title).c_str(), 24) / 2), 48, 24, WHITE);
				if (FC) {
					DrawTextRubik("Flawless!", GetScreenWidth() / 2 - (MeasureTextRubik("Flawless!", 24) / 2), 7, 24, GOLD);
				}
				DrawTextRHDI(scoreCommaFormatter(score).c_str(), (GetScreenWidth() / 2) - MeasureTextRHDI(scoreCommaFormatter(score).c_str())/2, 120, Color{107, 161, 222,255});
				// DrawTextRubik(TextFormat("%s", starsDisplay), (GetScreenWidth() / 2 - MeasureTextRubik(TextFormat("%s", starsDisplay), 24) / 2), 160, 24, goldStars ? GOLD : WHITE);
				DrawTextRubik(TextFormat("Perfect Notes : %01i/%02i", perfectHit, songList.songs[curPlayingSong].parts[instrument]->charts[diff].notes.size()), (GetScreenWidth() / 2 - MeasureTextRubik(TextFormat("Perfect Notes: %01i/%02i", perfectHit, songList.songs[curPlayingSong].parts[instrument]->charts[diff].notes.size()), 24) / 2), 192, 24, WHITE);
				DrawTextRubik(TextFormat("Good Notes : %01i/%02i", notesHit - perfectHit, songList.songs[curPlayingSong].parts[instrument]->charts[diff].notes.size()), (GetScreenWidth() / 2 - MeasureTextRubik(TextFormat("Good Notes: %01i/%02i", notesHit - perfectHit, songList.songs[curPlayingSong].parts[instrument]->charts[diff].notes.size()), 24) / 2), 224, 24, WHITE);
				DrawTextRubik(TextFormat("Missed Notes: %01i/%02i", notesMissed, songList.songs[curPlayingSong].parts[instrument]->charts[diff].notes.size()), (GetScreenWidth() / 2 - MeasureTextRubik(TextFormat("Missed Notes: %01i/%02i", notesMissed, songList.songs[curPlayingSong].parts[instrument]->charts[diff].notes.size()), 24) / 2), 256, 24, WHITE);
				DrawTextRubik(TextFormat("Strikes: %01i", playerOverhits), (GetScreenWidth() / 2 - MeasureTextRubik(TextFormat("Strikes: %01i", playerOverhits), 24) / 2), 288, 24, WHITE);
				DrawTextRubik(TextFormat("Longest Streak: %01i", maxCombo), (GetScreenWidth() / 2 - MeasureTextRubik(TextFormat("Longest Streak: %01i", maxCombo), 24) / 2), 320, 24, WHITE);
				if (GuiButton({ 0,0,60,60 }, "<")) {
					SwitchScreen(SONG_SELECT);
				}
				break;
			}
            default: {
                SwitchScreen(MENU);
            }
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

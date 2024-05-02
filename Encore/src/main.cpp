
#if defined(WIN32) && defined(NDEBUG)
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif
#include "game/arguments.h"
#include "game/gameplay.h"

Assets assets;


vector<std::string> ArgumentList::arguments;

static bool compareNotes(const Note& a, const Note& b) {
	return a.time < b.time;
}


Scenes scenes;
RhythmLogic rhythmLogic;
Gameplay gameplay;
Player player;
SongList songList;

Vector2 viewScroll = { 0,0 };
Rectangle view = { 0 };


std::string trackSpeedButton;

double lastAxesTime = 0.0;

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




	std::filesystem::path executablePath(GetApplicationDirectory());

	std::filesystem::path directory = executablePath.parent_path();


    if (std::filesystem::exists(directory / "keybinds.json")) {
        Settings::migrateSettings((const std::filesystem::path)(directory / "keybinds.json"), (const std::filesystem::path)(directory / "Settings::json"));
    }

    Settings::loadSettings(directory / "Settings::json");
	trackSpeedButton = "Track Speed " + truncateFloatString(Settings::trackSpeedOptions[Settings::trackSpeed]) + "x";


	ChangeDirectory(GetApplicationDirectory());
	Assets::loadAssets(directory);
    SetWindowIcon(icon);

	glfwSetKeyCallback(glfwGetCurrentContext(), RhythmLogic::origKeyCallback);
	glfwSetGamepadStateCallback(RhythmLogic::origGamepadCallback);
    // GuiLoadStyle((directory / "Assets/ui/encore.rgs").string().c_str());

    GuiSetStyle(BUTTON, BASE, 0x181827FF);
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt(ColorBrightness(Player::accentColor, -0.5)));
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, ColorToInt(ColorBrightness(Player::accentColor, -0.25)));
    GuiSetStyle(BUTTON, BORDER, 0xFFFFFFFF);
    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, 0xFFFFFFFF);
    GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, 0xFFFFFFFF);
	GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0x505050ff);
    GuiSetStyle(DEFAULT, TEXT, 0xcbcbcbFF);
    GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, 0xFFFFFFFF);
    GuiSetStyle(BUTTON, TEXT_COLOR_PRESSED, 0xFFFFFFFF);
	GuiSetStyle(DEFAULT, TEXT_SIZE, 28);

	GuiSetStyle(TOGGLE, BASE, 0x181827FF);
	GuiSetStyle(TOGGLE, BASE_COLOR_FOCUSED, ColorToInt(ColorBrightness(Player::accentColor, -0.5)));
	GuiSetStyle(TOGGLE, BASE_COLOR_PRESSED, ColorToInt(ColorBrightness(Player::accentColor, -0.25)));
	GuiSetStyle(TOGGLE, BORDER, 0xFFFFFFFF);
	GuiSetStyle(TOGGLE, BORDER_COLOR_FOCUSED, 0xFFFFFFFF);
	GuiSetStyle(TOGGLE, BORDER_COLOR_PRESSED, 0xFFFFFFFF);
	GuiSetStyle(TOGGLE, TEXT_COLOR_FOCUSED, 0xFFFFFFFF);
	GuiSetStyle(TOGGLE, TEXT_COLOR_PRESSED, 0xFFFFFFFF);
	GuiSetFont(rubik32);


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

		BeginDrawing();

		ClearBackground(DARKGRAY);

		lerpCtrl.updateStates();

		switch (Scenes::currentScreen) {
            case SONG_LOADING_SCREEN : {
                if (!RhythmLogic::songsLoaded) {
                    Assets::DrawTextRHDI(TextFormat("Songs Loaded: %01i", songList.songCount), (float) GetScreenWidth() / 2 -
                                                 Assets::MeasureTextRHDI(TextFormat(
                                                                                               "Songs Loaded: %01i",
                                                                                               songList.songCount)) / 2,
                                 (float) GetScreenHeight() / 2, WHITE);
                    Assets::DrawTextRHDI(TextFormat("Bad Songs: %01i", songList.badSongCount), (float) GetScreenWidth() / 2 -
                                                 Assets::MeasureTextRHDI(TextFormat(
                                                                                               "Bad Songs: %01i",
                                                                                               songList.badSongCount)) /
                                                                                       2,
                                 (float) GetScreenHeight() / 2 + 50, WHITE);
                    Assets::DrawTextRHDI(TextFormat("Folders: %01i", songList.directoryCount), (float) GetScreenWidth() / 2 -
                                                 Assets::MeasureTextRHDI(TextFormat(
                                                                                               "Folders: %01i",
                                                                                               songList.directoryCount)) /
                                                                                       2,
                                 (float) GetScreenHeight() / 2 - 50, WHITE);

                    LoadSongs(Settings::songPaths);

                    RhythmLogic::songsLoaded = true;
                }
                for (Song& song : songList.songs) {
                    song.titleScrollTime = GetTime();
                    song.titleTextWidth = Assets::MeasureTextRubik(song.title.c_str(), 24);
                    song.artistScrollTime = GetTime();
                    song.artistTextWidth = Assets::MeasureTextRubik(song.artist.c_str(), 20);
                }

                Scenes::currentScreen = SONG_SELECT;
                break;
            }
			case MENU: {
				lerpCtrl.createLerp("MENU_LOGO", EaseOutCubic, 1.5f);
                DrawTextureEx(encoreWhiteLogo, {(float)GetScreenWidth()/2 - encoreWhiteLogo.width/4, (float)lerpCtrl.getState("MENU_LOGO").value * ((float)GetScreenHeight()/5 - encoreWhiteLogo.height/4)},0,0.5, WHITE);

                if (GuiButton({ ((float)GetScreenWidth() / 2) - 100,((float)GetScreenHeight() / 2) - 120,200, 60 }, "Play")) {
					Scenes::SwitchScreen(SONG_LOADING_SCREEN);
				}
				if (GuiButton({ ((float)GetScreenWidth() / 2) - 100,((float)GetScreenHeight() / 2) - 30,200, 60 }, "Options")) {
					glfwSetGamepadStateCallback(RhythmLogic::gamepadStateCallbackSetControls);
                    Scenes::SwitchScreen(SETTINGS);
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
                    RhythmLogic::songsLoaded = false;
                }
                DrawTextureEx(github, {(float)GetScreenWidth()-54, (float)GetScreenHeight()-54}, 0, 0.2, WHITE);
                DrawTextureEx(discord, {(float)GetScreenWidth()-113, (float)GetScreenHeight()-48}, 0, 0.075, WHITE);
				break;
			}
			case SETTINGS: {
				if (Settings::controllerType == -1 && RhythmLogic::controllerID != -1) {
					std::string gamepadName = std::string(glfwGetGamepadName(RhythmLogic::controllerID));
					Settings::controllerType = Keybinds::getControllerType(gamepadName);
				}
				if (GuiButton({ ((float)GetScreenWidth() / 2) - 350,((float)GetScreenHeight() - 60),100,60 }, "Cancel") && !(RhythmLogic::changingKey || RhythmLogic::changingOverdrive)) {
					glfwSetGamepadStateCallback(RhythmLogic::origGamepadCallback);
                    Settings::keybinds4K = Settings::prev4K;
                    Settings::keybinds5K = Settings::prev5K;
                    Settings::keybinds4KAlt = Settings::prev4KAlt;
                    Settings::keybinds5KAlt = Settings::prev5KAlt;
					Settings::keybindOverdrive = Settings::prevOverdrive;
					Settings::keybindOverdriveAlt = Settings::prevOverdriveAlt;

					Settings::controller4K = Settings::prevController4K;
					Settings::controller4KAxisDirection = Settings::prevController4KAxisDirection;
					Settings::controller5K = Settings::prevController5K;
					Settings::controller5KAxisDirection = Settings::prevController5KAxisDirection;
					Settings::controllerOverdrive = Settings::prevControllerOverdrive;
					Settings::controllerOverdriveAxisDirection = Settings::prevControllerOverdriveAxisDirection;
					Settings::controllerType = Settings::prevControllerType;

                    Settings::highwayLengthMult = Settings::prevHighwayLengthMult;
					Settings::trackSpeed = Settings::prevTrackSpeed;
					Settings::inputOffsetMS = Settings::prevInputOffsetMS;
					Settings::avOffsetMS = Settings::prevAvOffsetMS;
					Settings::missHighwayDefault = Settings::prevMissHighwayColor;
					Settings::mirrorMode = Settings::prevMirrorMode;

					Settings::saveSettings(directory / "Settings::json");

					Scenes::SwitchScreen(MENU);
				}
				if (GuiButton({ ((float)GetScreenWidth() / 2) + 250,((float)GetScreenHeight() - 60),100,60 }, "Apply") && !(RhythmLogic::changingKey || RhythmLogic::changingOverdrive)) {
					glfwSetGamepadStateCallback(RhythmLogic::origGamepadCallback);
					Settings::prev4K = Settings::keybinds4K;
					Settings::prev5K = Settings::keybinds5K;
					Settings::prev4KAlt = Settings::keybinds4KAlt;
					Settings::prev5KAlt = Settings::keybinds5KAlt;
					Settings::prevOverdrive = Settings::keybindOverdrive;
					Settings::prevOverdriveAlt = Settings::keybindOverdriveAlt;

					Settings::prevController4K = Settings::controller4K;
					Settings::prevController4KAxisDirection = Settings::controller4KAxisDirection;
					Settings::prevController5K = Settings::controller5K;
					Settings::prevController5KAxisDirection = Settings::controller5KAxisDirection;
					Settings::prevControllerOverdrive = Settings::controllerOverdrive;
					Settings::prevControllerOverdriveAxisDirection = Settings::controllerOverdriveAxisDirection;
					Settings::prevControllerType = Settings::controllerType;

                    Settings::prevHighwayLengthMult = Settings::highwayLengthMult;
					Settings::prevTrackSpeed = Settings::trackSpeed;
					Settings::prevInputOffsetMS = Settings::inputOffsetMS;
					Settings::prevAvOffsetMS = Settings::avOffsetMS;
					Settings::prevMissHighwayColor = Settings::missHighwayDefault;
					Settings::prevMirrorMode = Settings::mirrorMode;

					Settings::saveSettings(directory / "Settings::json");

					Scenes::SwitchScreen(MENU);
				}
				static int selectedTab = 0;
				GuiToggleGroup({ 0,0,(float)GetScreenWidth() / 3,60 }, "Main;Keyboard Controls;Gamepad Controls", &selectedTab);
				if (selectedTab == 0) { //Main settings tab
					if (GuiButton({ (float)GetScreenWidth() / 2 - 125,(float)GetScreenHeight() / 2 - 320,250,60 }, "")) {
						if (Settings::trackSpeed == Settings::trackSpeedOptions.size() - 1) Settings::trackSpeed = 0; else Settings::trackSpeed++;
						trackSpeedButton = "Track Speed " + truncateFloatString(Settings::trackSpeedOptions[Settings::trackSpeed]) + "x";
					}
					Assets::DrawTextRubik(trackSpeedButton.c_str(), (float)GetScreenWidth() / 2 - Assets::MeasureTextRubik(trackSpeedButton.c_str(), 20) / 2, (float)GetScreenHeight() / 2 - 300, 20, WHITE);

					GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
					float avOffsetFloat = (float)Settings::avOffsetMS;
                    float lengthSetting = Settings::highwayLengthMult;
                    Assets::DrawTextRubik("A/V Offset", (float)GetScreenWidth() / 2 - Assets::MeasureTextRubik("A/V Offset", 20) / 2, (float)GetScreenHeight() / 2 - 240, 20, WHITE);
                    Assets::DrawTextRubik(" -500 ", (float)GetScreenWidth() / 2 - 125 - Assets::MeasureTextRubik(" -500 ", 20), (float)GetScreenHeight() / 2 - 210, 20, WHITE);
                    Assets::DrawTextRubik(" 500 ", (float)GetScreenWidth() / 2 + 125, (float)GetScreenHeight() / 2 - 210, 20, WHITE);
					if (GuiSliderBar({ (float)GetScreenWidth() / 2 - 125,(float)GetScreenHeight() / 2 - 220,250,40 }, "", "", &avOffsetFloat, -500.0f, 500.0f)) {
						Settings::avOffsetMS = (int)avOffsetFloat;
					}

					if (GuiButton({ (float)GetScreenWidth() / 2 - 125 - Assets::MeasureTextRubik(" -500 ", 20) - 60,(float)GetScreenHeight() / 2 - 230,60,60 }, "-1")) {
						Settings::avOffsetMS--;
					}
					if (GuiButton({ (float)GetScreenWidth() / 2 + 125 + Assets::MeasureTextRubik(" -500 ", 20) ,(float)GetScreenHeight() / 2 - 230,60,60 }, "+1")) {
						Settings::avOffsetMS++;
					}
                    Assets::DrawTextRubik(TextFormat("%01i ms",Settings::avOffsetMS), (float)GetScreenWidth() / 2 - (Assets::MeasureTextRubik(TextFormat("%01i ms",Settings::avOffsetMS), 20) / 2), (float)GetScreenHeight() / 2 - 210, 20, BLACK);


                    float lengthHeight = ((float)GetScreenHeight() / 2 )- 60;
                    Assets::DrawTextRubik("Highway Length", (float)GetScreenWidth() / 2 - Assets::MeasureTextRubik("Highway Length", 20) / 2, lengthHeight - 20, 20, WHITE);
                    Assets::DrawTextRubik(" 0.25 ", (float)GetScreenWidth() / 2 - 125 - Assets::MeasureTextRubik(" 0.25 ", 20), lengthHeight+10, 20, WHITE);
                    Assets::DrawTextRubik(" 2.50 ", (float)GetScreenWidth() / 2 + 125, lengthHeight+10, 20, WHITE);
                    if (GuiSliderBar({ (float)GetScreenWidth() / 2 - 125,lengthHeight,250,40 }, "", "", &lengthSetting, 0.25f, 2.5f)) {
                        Settings::highwayLengthMult = lengthSetting;
                    }
                    if (GuiButton({ (float)GetScreenWidth() / 2 - 125 - Assets::MeasureTextRubik(" 0.25 ", 20) - 60,lengthHeight-10,60,60 }, "-0.25")) {
                        Settings::highwayLengthMult-= 0.25;
                    }
                    if (GuiButton({ (float)GetScreenWidth() / 2 + 125 + Assets::MeasureTextRubik(" 2.50 ", 20) ,lengthHeight-10,60,60 }, "+0.25")) {
                        Settings::highwayLengthMult+=0.25;
                    }
                    Assets::DrawTextRubik(TextFormat("%1.2fx",Settings::highwayLengthMult), (float)GetScreenWidth() / 2 - (Assets::MeasureTextRubik(TextFormat("%1.2f",Settings::highwayLengthMult), 20) / 2), lengthHeight+10, 20, BLACK);


                    if (GuiButton({ (float)GetScreenWidth() / 2 - 125, (float)GetScreenHeight() / 2,250,60 }, TextFormat("Miss Highway Color: %s", Player::MissHighwayColor ? "True" : "False"))) {
						Settings::missHighwayDefault = !Settings::missHighwayDefault;
                        Player::MissHighwayColor = Settings::missHighwayDefault;
                    }
					if (GuiButton({ (float)GetScreenWidth() / 2 - 125, (float)GetScreenHeight() / 2 + 90,250,60 }, TextFormat("Mirror mode: %s", Settings::mirrorMode ? "True" : "False"))) {
						Settings::mirrorMode = !Settings::mirrorMode;
					}
					float inputOffsetFloat = (float)Settings::inputOffsetMS;
                    Assets::DrawTextRubik("Input Offset", (float)GetScreenWidth() / 2 - Assets::MeasureTextRubik("Input Offset", 20) / 2, (float)GetScreenHeight() / 2 - 160, 20, WHITE);
                    Assets::DrawTextRubik(" -500 ", (float)GetScreenWidth() / 2 - 125 - Assets::MeasureTextRubik(" -500 ", 20), (float)GetScreenHeight() / 2 - 130, 20, WHITE);
                    Assets::DrawTextRubik(" 500 ", (float)GetScreenWidth() / 2 + 125, (float)GetScreenHeight() / 2 - 130, 20, WHITE);
					if (GuiSliderBar({ (float)GetScreenWidth() / 2 - 125,(float)GetScreenHeight() / 2 - 140,250,40 }, "", "", &inputOffsetFloat, -500.0f, 500.0f)) {
						Settings::inputOffsetMS = (int)inputOffsetFloat;
					}
					if (GuiButton({ (float)GetScreenWidth() / 2 - 125 - Assets::MeasureTextRubik(" -500 ", 20) - 60,(float)GetScreenHeight() / 2 - 150,60,60 }, "-1")) {
						Settings::inputOffsetMS--;
					}
					if (GuiButton({ (float)GetScreenWidth() / 2 + 125 + Assets::MeasureTextRubik(" -500 ", 20),(float)GetScreenHeight() / 2 - 150,60,60 }, "+1")) {
						Settings::inputOffsetMS++;
					}
                    Assets::DrawTextRubik(TextFormat("%01i ms",Settings::inputOffsetMS), (float)GetScreenWidth() / 2 - (Assets::MeasureTextRubik(TextFormat("%01i ms",Settings::inputOffsetMS), 20) / 2), (float)GetScreenHeight() / 2 - 130, 20, BLACK);
					GuiSetStyle(DEFAULT, TEXT_SIZE, 28);
				}
				else if (selectedTab == 1) { //Keyboard bindings tab
					GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
					for (int i = 0; i < 5; i++) {
						float j = i - 2.0f;
						if (GuiButton({ ((float)GetScreenWidth() / 2) - 40 + (80 * j),120,80,60 }, Keybinds::getKeyStr(Settings::keybinds5K[i]).c_str())) {
							Settings::changing4k = false;
							Settings::changingAlt = false;
							RhythmLogic::selLane = i;
							RhythmLogic::changingKey = true;
						}
						if (GuiButton({ ((float)GetScreenWidth() / 2) - 40 + (80 * j),180,80,60 }, Keybinds::getKeyStr(Settings::keybinds5KAlt[i]).c_str())) {
							Settings::changing4k = false;
							Settings::changingAlt = true;
							RhythmLogic::selLane = i;
							RhythmLogic::changingKey = true;
						}
					}
					for (int i = 0; i < 4; i++) {
						float j = i - 1.5f;
						if (GuiButton({ ((float)GetScreenWidth() / 2) - 40 + (80 * j),300,80,60 }, Keybinds::getKeyStr(Settings::keybinds4K[i]).c_str())) {
							Settings::changingAlt = false;
							Settings::changing4k = true;
							RhythmLogic::selLane = i;
							RhythmLogic::changingKey = true;
						}
						if (GuiButton({ ((float)GetScreenWidth() / 2) - 40 + (80 * j),360,80,60 }, Keybinds::getKeyStr(Settings::keybinds4KAlt[i]).c_str())) {
							Settings::changingAlt = true;
							Settings::changing4k = true;
							RhythmLogic::selLane = i;
							RhythmLogic::changingKey = true;
						}
					}
					if (GuiButton({ ((float)GetScreenWidth() / 2) - 100,480,80,60 }, Keybinds::getKeyStr(Settings::keybindOverdrive).c_str())) {
						Settings::changingAlt = false;
						RhythmLogic::changingKey = false;
						RhythmLogic::changingOverdrive = true;
					}
					if (GuiButton({ ((float)GetScreenWidth() / 2) + 20,480,80,60 }, Keybinds::getKeyStr(Settings::keybindOverdriveAlt).c_str())) {
						Settings::changingAlt = true;
						RhythmLogic::changingKey = false;
						RhythmLogic::changingOverdrive = true;
					}
					if (RhythmLogic::changingKey) {
						std::vector<int>& bindsToChange = Settings::changingAlt ? (Settings::changing4k ? Settings::keybinds4KAlt : Settings::keybinds5KAlt) : (Settings::changing4k ? Settings::keybinds4K : Settings::keybinds5K);
						DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), { 0,0,0,200 });
						std::string keyString = (Settings::changing4k ? "4k" : "5k");
						std::string altString = (Settings::changingAlt ? " alt" : "");
						std::string changeString = "Press a key for " + keyString + altString + " lane ";
                        Assets::DrawTextRubik(changeString.c_str(), (GetScreenWidth() - Assets::MeasureTextRubik(changeString.c_str(), 20)) / 2, GetScreenHeight() / 2 - 30, 20, WHITE);
                        Assets::DrawTextRubik("Or press escape to remove bound key", (GetScreenWidth() - Assets::MeasureTextRubik("Or press escape to remove bound key", 20)) / 2, GetScreenHeight() / 2 + 30, 20, WHITE);
						int pressedKey = GetKeyPressed();
						if (pressedKey != 0) {
							if (pressedKey == KEY_ESCAPE)
								pressedKey = -1;
							bindsToChange[RhythmLogic::selLane] = pressedKey;
							RhythmLogic::selLane = 0;
							RhythmLogic::changingKey = false;
						}
					}
					if (RhythmLogic::changingOverdrive) {
						DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), { 0,0,0,200 });
						std::string altString = (Settings::changingAlt ? " alt" : "");
						std::string changeString = "Press a key for " + altString + " overdrive";
                        Assets::DrawTextRubik(changeString.c_str(), (GetScreenWidth() - Assets::MeasureTextRubik(changeString.c_str(), 20)) / 2, GetScreenHeight() / 2 - 30, 20, WHITE);
                        Assets::DrawTextRubik("Or press escape to remove bound key", (GetScreenWidth() - Assets::MeasureTextRubik("Or press escape to remove bound key", 20)) / 2, GetScreenHeight() / 2 + 30, 20, WHITE);
						int pressedKey = GetKeyPressed();
						if (pressedKey != 0) {
							if (pressedKey == KEY_ESCAPE)
								pressedKey = -1;
							if (Settings::changingAlt)
								Settings::keybindOverdriveAlt = pressedKey;
							else
								Settings::keybindOverdrive = pressedKey;
							RhythmLogic::changingOverdrive = false;
						}
					}
					GuiSetStyle(DEFAULT, TEXT_SIZE, 28);
				}
				else if (selectedTab == 2) { //Controller bindings tab
					GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
					for (int i = 0; i < 5; i++) {
						float j = i - 2.0f;
						if (GuiButton({ ((float)GetScreenWidth() / 2) - 40 + (80 * j),240,80,60 }, Keybinds::getControllerStr(RhythmLogic::controllerID, Settings::controller5K[i], Settings::controllerType, Settings::controller5KAxisDirection[i]).c_str())) {
							Settings::changing4k = false;
							RhythmLogic::selLane = i;
							RhythmLogic::changingKey = true;
						}
					}
					for (int i = 0; i < 4; i++) {
						float j = i - 1.5f;
						if (GuiButton({ ((float)GetScreenWidth() / 2) - 40 + (80 * j),360,80,60 }, Keybinds::getControllerStr(RhythmLogic::controllerID, Settings::controller4K[i], Settings::controllerType, Settings::controller4KAxisDirection[i]).c_str())) {
							Settings::changing4k = true;
							RhythmLogic::selLane = i;
							RhythmLogic::changingKey = true;
						}
					}
					if (GuiButton({ ((float)GetScreenWidth() / 2) - 40,480,80,60 }, Keybinds::getControllerStr(RhythmLogic::controllerID, Settings::controllerOverdrive, Settings::controllerType, Settings::controllerOverdriveAxisDirection).c_str())) {
						RhythmLogic::changingKey = false;
						RhythmLogic::changingOverdrive = true;
					}
					if (RhythmLogic::changingKey) {
						std::vector<int>& bindsToChange = (Settings::changing4k ? Settings::controller4K : Settings::controller5K);
						std::vector<int>& directionToChange = (Settings::changing4k ? Settings::controller4KAxisDirection : Settings::controller5KAxisDirection);
						DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), { 0,0,0,200 });
						std::string keyString = (Settings::changing4k ? "4k" : "5k");
						std::string changeString = "Press a button/axis for controller " + keyString + " lane " + std::to_string(RhythmLogic::selLane + 1);
                        Assets::DrawTextRubik(changeString.c_str(), (GetScreenWidth() - Assets::MeasureTextRubik(changeString.c_str(), 20)) / 2, GetScreenHeight() / 2 - 30, 20, WHITE);
                        Assets::DrawTextRubik("Or press escape to cancel", (GetScreenWidth() - Assets::MeasureTextRubik("Or press escape to cancel", 20)) / 2, GetScreenHeight() / 2 + 30, 20, WHITE);
						if (RhythmLogic::pressedGamepadInput != -999) {
							bindsToChange[RhythmLogic::selLane] = RhythmLogic::pressedGamepadInput;
							if (RhythmLogic::pressedGamepadInput < 0) {
								directionToChange[RhythmLogic::selLane] = RhythmLogic::axisDirection;
							}
                            RhythmLogic::selLane = 0;
                            RhythmLogic::changingKey = false;
                            RhythmLogic::pressedGamepadInput = -999;
						}
					}
					if (RhythmLogic::changingOverdrive) {
						DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), { 0,0,0,200 });
						std::string changeString = "Press a button/axis for controller overdrive";
                        Assets::DrawTextRubik(changeString.c_str(), (GetScreenWidth() - Assets::MeasureTextRubik(changeString.c_str(), 20)) / 2, GetScreenHeight() / 2 - 30, 20, WHITE);
                        Assets::DrawTextRubik("Or press escape to cancel", (GetScreenWidth() - Assets::MeasureTextRubik("Or press escape to cancel", 20)) / 2, GetScreenHeight() / 2 + 30, 20, WHITE);

						if (RhythmLogic::pressedGamepadInput != -999) {
							Settings::controllerOverdrive = RhythmLogic::pressedGamepadInput;
							if (RhythmLogic::pressedGamepadInput < 0) {
								Settings::controllerOverdriveAxisDirection = RhythmLogic::axisDirection;
							}
                            RhythmLogic::changingOverdrive = false;
                            RhythmLogic::pressedGamepadInput = -999;
						}
					}
					if (IsKeyPressed(KEY_ESCAPE)) {
                        RhythmLogic::pressedGamepadInput = -999;
                        RhythmLogic::changingKey = false;
                        RhythmLogic::changingOverdrive = false;
					}
					GuiSetStyle(DEFAULT, TEXT_SIZE, 28);
				}
				break;
			}
			case SONG_SELECT: {
                RhythmLogic::streamsLoaded = false;
                RhythmLogic::midiLoaded = false;
                RhythmLogic::isPlaying = false;
                Player::overdrive = false;
                RhythmLogic::curNoteIdx = { 0,0,0,0,0 };
                RhythmLogic::curODPhrase = 0;
                RhythmLogic::curBeatLine = 0;
                RhythmLogic::curBPM = 0;
                Player::instrument = 0;
                Player::diff = 0;
                int randSong = rand()%(songList.songCount-0+1)+0;
                int selectedSongInt = RhythmLogic::curPlayingSong;
                Song selectedSong = songList.songs[selectedSongInt];

                SetTextureWrap(selectedSong.albumArtBlur, TEXTURE_WRAP_REPEAT);
                SetTextureFilter(selectedSong.albumArtBlur, TEXTURE_FILTER_ANISOTROPIC_16X);
                if (RhythmLogic::selSong){
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
				if (RhythmLogic::songSelectOffset <= songList.songs.size() + 2 - (GetScreenHeight() / 60) && RhythmLogic::songSelectOffset >= 0) {
                    RhythmLogic::songSelectOffset -= mouseWheel.y;
				}
				if (RhythmLogic::songSelectOffset < 0) RhythmLogic::songSelectOffset = 0;
				if (RhythmLogic::songSelectOffset > songList.songCount + 2 - (GetScreenHeight() / 60)) RhythmLogic::songSelectOffset = songList.songs.size() + 2 - (GetScreenHeight() / 60);
				if (RhythmLogic::songSelectOffset >= songList.songCount) RhythmLogic::songSelectOffset = songList.songCount - 1;


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
                DrawTextEx(redHatDisplayLarge, "Song Select", {TextPlacementLR, TextPlacementTB}, 100,1, WHITE);

                if (GuiButton({ 0,0,60,60 }, "<")) {
                    for (Song& song : songList.songs) {
                        song.titleXOffset = 0;
                        song.artistXOffset = 0;
                    }
                    Scenes::SwitchScreen(MENU);
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
                if (RhythmLogic::selSong) {
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

                for (int i = RhythmLogic::songSelectOffset; i < RhythmLogic::songSelectOffset + (GetScreenHeight() / 50) - 2; i++) {
					if (songList.songCount-1 < i)
						break;

					Song& song = songList.songs[i];
                    float buttonX = ((float)GetScreenWidth()/2)-(((float)GetScreenWidth()*0.86f)/2);
					//LerpState state = lerpCtrl.createLerp("SONGSELECT_LERP_" + std::to_string(i), EaseOutCirc, 0.4f);
					float songXPos = LeftSide;//state.value * 500;
					float songYPos = (TopOvershell+6) + (45 * (i - RhythmLogic::songSelectOffset));

					if (GuiButton(Rectangle{ songXPos, songYPos,(AlbumArtLeft-LeftSide)-6, 45 }, "")) {
                        RhythmLogic::curPlayingSong = i;
                        RhythmLogic::selSong = true;

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
					DrawTextEx(rubikBold32,song.title.c_str(), {songXPos + 15 + song.titleXOffset, songYPos + 10}, 24,1, i == RhythmLogic::curPlayingSong && RhythmLogic::selSong ? WHITE : LightText);
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
                    Assets::DrawTextRubik(song.artist.c_str(), songXPos + 30 + songTitleWidth + song.artistXOffset, songYPos + 12, 20, i == RhythmLogic::curPlayingSong && RhythmLogic::selSong ? WHITE : LightText);
					EndScissorMode();
				}
                // hehe
                DrawBottomOvershell();
                float BottomOvershell = GetScreenHeight() - 120;
                if (RhythmLogic::selSong) {
                    if (GuiButton(Rectangle{LeftSide, BottomOvershell, 250, 34}, "Play Song")) {
                        RhythmLogic::curPlayingSong = selectedSongInt;
                        Scenes::SwitchScreen(INSTRUMENT_SELECT);

                    }
                }
                DrawBottomBottomOvershell();
                break;
			}
			case INSTRUMENT_SELECT: {
                RhythmLogic::selSong = false;
                Song selectedSong = songList.songs[RhythmLogic::curPlayingSong];
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
                DrawTextEx(redHatDisplayLarge, songList.songs[RhythmLogic::curPlayingSong].title.c_str(), {TextPlacementLR, TextPlacementTB-5}, 72,1, WHITE);
                DrawTextEx(rubikBoldItalic32, selectedSong.artist.c_str(), {TextPlacementLR, TextPlacementTB+60}, 40,1,LIGHTGRAY);
				if (!RhythmLogic::midiLoaded) {
					if (!songList.songs[RhythmLogic::curPlayingSong].midiParsed) {
						smf::MidiFile midiFile;
						midiFile.read(songList.songs[RhythmLogic::curPlayingSong].midiPath.string());
						songList.songs[RhythmLogic::curPlayingSong].getTiming(midiFile, 0, midiFile[0]);
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
											songList.songs[RhythmLogic::curPlayingSong].parseBeatLines(midiFile, i, midiFile[i]);
										else if (trackName == "EVENTS") {
											songList.songs[RhythmLogic::curPlayingSong].getStartEnd(midiFile, i, midiFile[i]);
										}
										else {
											if (songPart != SongParts::Invalid) {
												for (int loadDiff = 0; loadDiff < 4; loadDiff++) {
													Chart newChart;
													newChart.parseNotes(midiFile, i, midiFile[i], loadDiff, (int)songPart);
													if (!newChart.notes.empty()) {
														songList.songs[RhythmLogic::curPlayingSong].parts[(int)songPart]->hasPart = true;
													}
													std::sort(newChart.notes.begin(), newChart.notes.end(), compareNotes);
													int noteIdx = 0;
													for (Note& note : newChart.notes) {
														newChart.notes_perlane[note.lane].push_back(noteIdx);
														noteIdx++;
													}
													songList.songs[RhythmLogic::curPlayingSong].parts[(int)songPart]->charts.push_back(newChart);
												}
											}
										}
									}
								}
							}
						}
						songList.songs[RhythmLogic::curPlayingSong].midiParsed = true;
					}
                    RhythmLogic::midiLoaded = true;
				}
				else {
					if (GuiButton({ 0,0,60,60 }, "<")) {
                        RhythmLogic::midiLoaded = false;
                        Scenes::SwitchScreen(SONG_SELECT);
					}
                    // DrawTextRHDI(TextFormat("%s - %s", songList.songs[curPlayingSong].title.c_str(), songList.songs[curPlayingSong].artist.c_str()), 70,7, WHITE);
					for (int i = 0; i < 4; i++) {
						if (songList.songs[RhythmLogic::curPlayingSong].parts[i]->hasPart) {
							if (GuiButton({ LeftSide,BottomOvershell - 60 - (60 * (float)i),300,60 }, "")) {
                                Player::instrument = i;
								int isBassOrVocal = 0;
								if (Player::instrument == 1 || Player::instrument == 3) {
									isBassOrVocal = 1;
								}
								SetShaderValue(odMultShader, isBassOrVocalLoc, &isBassOrVocal, SHADER_UNIFORM_INT);
                                Scenes::SwitchScreen(DIFFICULTY_SELECT);
							}

                            Assets::DrawTextRubik(songPartsList[i].c_str(), LeftSide + 20, BottomOvershell - 45 - (60 * (float)i), 30, WHITE);
                            Assets::DrawTextRubik((std::to_string(songList.songs[RhythmLogic::curPlayingSong].parts[i]->diff + 1) + "/7").c_str(), LeftSide + 220, BottomOvershell - 45 - (60 * (float)i), 30, WHITE);
						}
					}
				}
                DrawBottomOvershell();
                DrawBottomBottomOvershell();
				break;
			}
			case DIFFICULTY_SELECT: {
                Song selectedSong = songList.songs[RhythmLogic::curPlayingSong];
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
                DrawTextEx(redHatDisplayLarge, songList.songs[RhythmLogic::curPlayingSong].title.c_str(), {TextPlacementLR, TextPlacementTB-5}, 72,1, WHITE);
                DrawTextEx(rubikBoldItalic32, selectedSong.artist.c_str(), {TextPlacementLR, TextPlacementTB+60}, 40,1,LIGHTGRAY);
				for (int i = 0; i < 4; i++) {
					if (GuiButton({ 0,0,60,60 }, "<")) {
                        Scenes::SwitchScreen(INSTRUMENT_SELECT);
					}
                    // DrawTextRHDI(TextFormat("%s - %s", songList.songs[curPlayingSong].title.c_str(), songList.songs[curPlayingSong].artist.c_str()), 70,7, WHITE);
					if (!songList.songs[RhythmLogic::curPlayingSong].parts[Player::instrument]->charts[i].notes.empty()) {
						if (GuiButton({ LeftSide,BottomOvershell - 60 - (60 * (float)i),300,60 }, "")) {
							Player::diff = i;
                            Scenes::SwitchScreen(GAMEPLAY);
							RhythmLogic::isPlaying = true;
							RhythmLogic::startedPlayingSong = GetTime();
							glfwSetKeyCallback(glfwGetCurrentContext(), RhythmLogic::keyCallback);
							glfwSetGamepadStateCallback(RhythmLogic::gamepadStateCallback);
						}
                        Assets::DrawTextRubik(diffList[i].c_str(), LeftSide + 150 - (Assets::MeasureTextRubik(diffList[i].c_str(), 30) / 2), BottomOvershell - 45 - (60 * (float)i), 30, WHITE);
					}
				}
                DrawBottomOvershell();
                DrawBottomBottomOvershell();
				break;
			}
			case GAMEPLAY: {
                gameplay.gameplay();
			}
			case RESULTS: {
				int starsval = Player::stars(songList.songs[RhythmLogic::curPlayingSong].parts[Player::instrument]->charts[Player::diff].baseScore,Player::diff);
                for (int i = 0; i < starsval; i++) {
                    DrawTextureEx(Player::goldStars? goldStar : star, {((float)GetScreenWidth()/2)+(i*40)-100,84},0,0.15f,WHITE);
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
                Assets::DrawTextRHDI("Results", 70,7, WHITE);
                Assets::DrawTextRubik((songList.songs[RhythmLogic::curPlayingSong].artist + " - " + songList.songs[RhythmLogic::curPlayingSong].title).c_str(), GetScreenWidth() / 2 - (Assets::MeasureTextRubik((songList.songs[RhythmLogic::curPlayingSong].artist + " - " + songList.songs[RhythmLogic::curPlayingSong].title).c_str(), 24) / 2), 48, 24, WHITE);
				if (Player::FC) {
                    Assets::DrawTextRubik("Flawless!", GetScreenWidth() / 2 - (Assets::MeasureTextRubik("Flawless!", 24) / 2), 7, 24, GOLD);
				}

				Assets::DrawTextRHDI(Gameplay::scoreCommaFormatter(Player::score).c_str(), (GetScreenWidth() / 2) - Assets::MeasureTextRHDI(Gameplay::scoreCommaFormatter(Player::score).c_str())/2, 120, Color{107, 161, 222,255});
				// DrawTextRubik(TextFormat("%s", starsDisplay), (GetScreenWidth() / 2 - MeasureTextRubik(TextFormat("%s", starsDisplay), 24) / 2), 160, 24, goldStars ? GOLD : WHITE);
                Assets::DrawTextRubik(TextFormat("Perfect Notes : %01i/%02i", Player::perfectHit, songList.songs[RhythmLogic::curPlayingSong].parts[Player::instrument]->charts[Player::diff].notes.size()), (GetScreenWidth() / 2 - Assets::MeasureTextRubik(TextFormat("Perfect Notes: %01i/%02i", Player::perfectHit, songList.songs[RhythmLogic::curPlayingSong].parts[Player::instrument]->charts[Player::diff].notes.size()), 24) / 2), 192, 24, WHITE);
                Assets::DrawTextRubik(TextFormat("Good Notes : %01i/%02i", Player::notesHit - Player::perfectHit, songList.songs[RhythmLogic::curPlayingSong].parts[Player::instrument]->charts[Player::diff].notes.size()), (GetScreenWidth() / 2 - Assets::MeasureTextRubik(TextFormat("Good Notes: %01i/%02i", Player::notesHit - Player::perfectHit, songList.songs[RhythmLogic::curPlayingSong].parts[Player::instrument]->charts[Player::diff].notes.size()), 24) / 2), 224, 24, WHITE);
                Assets::DrawTextRubik(TextFormat("Missed Notes: %01i/%02i", Player::notesMissed, songList.songs[RhythmLogic::curPlayingSong].parts[Player::instrument]->charts[Player::diff].notes.size()), (GetScreenWidth() / 2 - Assets::MeasureTextRubik(TextFormat("Missed Notes: %01i/%02i", Player::notesMissed, songList.songs[RhythmLogic::curPlayingSong].parts[Player::instrument]->charts[Player::diff].notes.size()), 24) / 2), 256, 24, WHITE);
                Assets::DrawTextRubik(TextFormat("Strikes: %01i", Player::playerOverhits), (GetScreenWidth() / 2 - Assets::MeasureTextRubik(TextFormat("Strikes: %01i", Player::playerOverhits), 24) / 2), 288, 24, WHITE);
                Assets::DrawTextRubik(TextFormat("Longest Streak: %01i", Player::maxCombo), (GetScreenWidth() / 2 - Assets::MeasureTextRubik(TextFormat("Longest Streak: %01i", Player::maxCombo), 24) / 2), 320, 24, WHITE);
				if (GuiButton({ 0,0,60,60 }, "<")) {
                    Scenes::SwitchScreen(SONG_SELECT);
				}
				break;
			}
            default: {
                Scenes::SwitchScreen(MENU);
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

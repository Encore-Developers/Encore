#define JSON_DIAGNOSTICS 1
#include "menus/GameplayMenu.h"
#include "song/songlist.h"
#include "users/playerManager.h"
#include "menus/menu.h"
#include "menus/OvershellMenu.h"
#include "menus/sndTestMenu.h"
#include "menus/cacheLoadingScreen.h"
#include "menus/resultsMenu.h"
#include "util/discord.h"
#include "util/enclog.h"
#include "gameplay/enctime.h"

#include "util/json-helper.h"
#include "song/song.h"

#define RAYGUI_IMPLEMENTATION

/* not needed for debug purposes
#if defined(WIN32) && defined(NDEBUG)
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif
*/
#if defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#endif

#include <filesystem>
#include <iostream>
#include <vector>
#include <thread>
#include <condition_variable>
#include <thread>
#include <atomic>

#include "raylib.h"
#include "raygui.h"
#include "raymath.h"
#include "GLFW/glfw3.h"

#include "arguments.h"
#include "assets.h"
#include "song/audio.h"
#include "gameplay/gameplayRenderer.h"
#include "keybinds.h"
#include "old/lerp.h"
#include "menus/gameMenu.h"
#include "menus/overshellRenderer.h"

#include "menus/settingsOptionRenderer.h"

#include "menus/uiUnits.h"

#include "settings-old.h"
#include "settings.h"
#include "timingvalues.h"
#include "gameplay/InputHandler.h"
#include "gameplay/inputCallbacks.h"
#include "inih/INIReader.h"
#include "menus/ChartLoadingMenu.h"
#include "menus/ReadyUpMenu.h"
#include "menus/SettingsMenu.h"
#include "menus/SongSelectMenu.h"

#include "menus/styles.h"

#include <menus/MenuManager.h>
#include <nlohmann/json_fwd.hpp>

MenuManager TheMenuManager;
gameplayRenderer TheGameRenderer;
SongList TheSongList;
PlayerManager ThePlayerManager;
SettingsOld &settingsMain = SettingsOld::getInstance();
AudioManager &audioManager = AudioManager::getInstance();
Encore::Settings TheGameSettings;

// OvershellRenderer overshellRenderer;

Assets &assets = Assets::getInstance();

vector<std::string> ArgumentList::arguments;

#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH
#endif

#ifndef ENCORE_VERSION
#define ENCORE_VERSION
#endif

// calibration
bool isCalibrating = false;
double calibrationStartTime = 0.0;
double lastClickTime = 0.0;
std::vector<double> tapTimes;
const int clickInterval = 1;

bool showInputFeedback = false;
double inputFeedbackStartTime = 0.0;
const double inputFeedbackDuration = 0.6;
float inputFeedbackAlpha = 1.0f;
// end calibration

std::string encoreVersion = ENCORE_VERSION;
std::string commitHash = GIT_COMMIT_HASH;

bool Menu::onNewMenu = false;

/*
std::string scoreCommaFormatter(int value) {
    std::stringstream ss;
    ss.imbue(std::locale(std::cout.getloc(), new Separators<char>()));
    ss << std::fixed << value;
    return ss.str();
}
*/

int minWidth = 640;
int minHeight = 480;

Menu *ActiveMenu = nullptr;

int main(int argc, char *argv[]) {
    SetTraceLogCallback(Encore::EncoreLog);
    Units u = Units::getInstance();
    commitHash.erase(7);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    SetWindowState(FLAG_MSAA_4X_HINT);
    bool windowToggle = true;
    ArgumentList::InitArguments(argc, argv);

    std::string FPSCapStringVal = ArgumentList::GetArgValue("fpscap");
    std::string vSyncOn = ArgumentList::GetArgValue("vsync");
    int targetFPSArg = -1;
    int vsyncArg = 1;

    if (!vSyncOn.empty()) {
        vsyncArg = strtol(vSyncOn.c_str(), NULL, 10);
        Encore::EncoreLog(
            LOG_INFO, TextFormat("Vertical sync argument toggled: %d", vsyncArg)
        );
    }
    if (vsyncArg == 1) {
        SetConfigFlags(FLAG_VSYNC_HINT);
    }

    std::filesystem::path executablePath(GetApplicationDirectory());

    std::filesystem::path directory = executablePath.parent_path();

#ifdef __APPLE__
    CFBundleRef bundle = CFBundleGetMainBundle();
    if (bundle != NULL) {
        // get the Resources directory for our binary for the Assets handling
        CFURLRef resourceURL = CFBundleCopyResourcesDirectoryURL(bundle);
        if (resourceURL != NULL) {
            char resourcePath[PATH_MAX];
            if (CFURLGetFileSystemRepresentation(
                    resourceURL, true, (UInt8 *)resourcePath, PATH_MAX
                ))
                assets.setDirectory(resourcePath);
            CFRelease(resourceURL);
        }
        // do the next step manually (settings/config handling)
        // "directory" is our executable directory here, hop up to the external dir
        if (directory.filename().compare("MacOS") == 0)
            directory = directory.parent_path().parent_path().parent_path(); // hops
        // "MacOS",
        // "Contents",
        // "Encore.app"
        // into
        // containing
        // folder

        CFRelease(bundle);
    }
#endif
    // todo: move to Encore::SettingsHelper
    TheGameSettings.SongPaths = { directory / "Songs" };
    if (exists(directory/"settings.json")
        && !exists(directory/"settings-old.json")) {
        rename(directory/"settings.json", directory/"settings-old.json");
    }
    // check to see if settings exists
    // todo: move to own init helper
    if (exists((directory / "settings.json"))) {
        nlohmann::json SettingsFile;
        std::ifstream f(directory / "settings.json");
        SettingsFile = nlohmann::json::parse(f);
        f.close();
        Encore::from_json(SettingsFile, TheGameSettings);

    } else {
        // void WriteSettingsFile(filesystem::path settingsDir, const Settings&
        // TheGameSettings)
        nlohmann::json SettingsFile = TheGameSettings;
        Encore::WriteJsonFile(directory / "settings.json", SettingsFile);
    }

    settingsMain.setDirectory(directory);
    ThePlayerManager.SetPlayerListSaveFileLocation(directory / "players.json");
    if (std::filesystem::exists(directory / "keybinds.json")) {
        settingsMain.migrateSettings(
            directory / "keybinds.json", directory / "settings-old.json"
        );
    }
    settingsMain.loadSettings(directory / "settings-old.json");
    ThePlayerManager.LoadPlayerList();

    bool removeFPSLimit = false;
    int menuFPS = GetMonitorRefreshRate(GetCurrentMonitor()) / 2;

    // https://www.raylib.com/examples/core/loader.html?name=core_custom_frame_control

    double previousTime = GetTime();
    double currentTime = 0.0;
    double updateDrawTime = 0.0;
    double waitTime = 0.0;
    float deltaTime = 0.0;

    if (!TheGameSettings.Fullscreen) {
        InitWindow(
            GetMonitorWidth(GetCurrentMonitor()) * 0.75f,
            GetMonitorHeight(GetCurrentMonitor()) * 0.75f,
            "Encore"
        );
        SET_WINDOW_WINDOWED();
    } else {
        InitWindow(
            GetMonitorWidth(GetCurrentMonitor()),
            GetMonitorHeight(GetCurrentMonitor()),
            "Encore"
        );
        SET_WINDOW_FULLSCREEN_BORDERLESS();
    }

    audioManager.Init();
    SetExitKey(0);
    audioManager.loadSample("Assets/combobreak.mp3", "miss");

    ChangeDirectory(GetApplicationDirectory());

    GLFWkeyfun origKeyCallback = glfwSetKeyCallback(glfwGetCurrentContext(), keyCallback);
    GLFWgamepadstatefun origGamepadCallback =
        glfwSetGamepadStateCallback(gamepadStateCallback);
    glfwSetKeyCallback(glfwGetCurrentContext(), origKeyCallback);
    glfwSetGamepadStateCallback(origGamepadCallback);

    SETDEFAULTSTYLE();

    SetRandomSeed(std::chrono::system_clock::now().time_since_epoch().count());
    assets.FirstAssets();
    SetWindowIcon(assets.icon);
    GuiSetFont(assets.rubik);
    assets.LoadAssets();
    TheMenuManager.currentScreen = CACHE_LOADING_SCREEN;
    Menu::onNewMenu = true;
    TheSongTime.SetOffset(TheGameSettings.AudioOffset / 1000.0);

    if (TheGameSettings.Framerate > 0)
        Encore::EncoreLog(
            LOG_INFO, TextFormat("Target FPS: %d", TheGameSettings.Framerate)
        );
    else {
        Encore::EncoreLog(LOG_INFO, TextFormat("Unlocked framerate."));
        removeFPSLimit = true;
    }

    // audioManager.loadSample("Assets/highway/clap.mp3", "clap");
    while (!WindowShouldClose()) {
        u.calcUnits();
        double curTime = GetTime();
        float bgTime = curTime / 5.0f;
        SetShaderValue(assets.bgShader, assets.bgTimeLoc, &bgTime, SHADER_UNIFORM_FLOAT);
        if (IsKeyPressed(KEY_F11)
            || (IsKeyPressed(KEY_LEFT_ALT) && IsKeyPressed(KEY_ENTER))) {
            TheGameSettings.Fullscreen = !TheGameSettings.Fullscreen;
            if (!TheGameSettings.Fullscreen) {
                SET_WINDOW_WINDOWED();
            } else {
                SET_WINDOW_FULLSCREEN_BORDERLESS();
            }
        }
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

        if (TheMenuManager.onNewMenu) {
            TheMenuManager.onNewMenu = false;
            delete ActiveMenu;
            ActiveMenu = NULL;
            // this is for dropping out
            glfwSetKeyCallback(glfwGetCurrentContext(), origKeyCallback);
            glfwSetGamepadStateCallback(origGamepadCallback);
            switch (TheMenuManager.currentScreen) { // NOTE: when adding a new Menu
                                                    // derivative, you
                // must put its enum value in Screens, and its
                // assignment in this switch/case. You will also
                // add its case to the `ActiveMenu->Draw();`
                // cases.
            case MAIN_MENU: {
                TheGameRPC.DiscordUpdatePresence("Main menu", "In the menus");
                ActiveMenu = new MainMenu;
                ActiveMenu->Load();
                break;
            }
            case SETTINGS: {
                TheGameRPC.DiscordUpdatePresence("Configuring", "In the menus");
                ActiveMenu = new SettingsMenu;
                ActiveMenu->Load();
                break;
            }
            case RESULTS: {
                TheGameRPC.DiscordUpdatePresence("Viewing results", "In the menus");
                ActiveMenu = new resultsMenu;
                ActiveMenu->Load();
                break;
            }
            case SONG_SELECT: {
                glfwSetGamepadStateCallback(gamepadStateCallback);
                TheGameRPC.DiscordUpdatePresence("Viewing songs", "In the menus");
                ActiveMenu = new SongSelectMenu;
                ActiveMenu->Load();
                break;
            }
            case READY_UP: {
                TheGameRPC.DiscordUpdatePresence("Readying up", "In the menus");
                ActiveMenu = new ReadyUpMenu;
                ActiveMenu->Load();
                break;
            }
            case SOUND_TEST: {
                ActiveMenu = new SoundTestMenu;
                ActiveMenu->Load();
                break;
            }
            case CACHE_LOADING_SCREEN: {
                TheGameRPC.DiscordUpdatePresence("Loading game", "In the menus");
                ActiveMenu = new cacheLoadingScreen;
                ActiveMenu->Load();
                break;
            }
            case CHART_LOADING_SCREEN: {
                TheGameRPC.DiscordUpdatePresence("Loading a song", "In the menus");
                ActiveMenu = new ChartLoadingMenu;
                ActiveMenu->Load();
                break;
            }
            case GAMEPLAY: {
                TheGameRPC.DiscordUpdatePresenceSong(
                    "Playing a song",
                    TheSongList.curSong->title + " - " + TheSongList.curSong->artist,
                    ThePlayerManager.GetActivePlayer(0).Instrument
                );
                glfwSetKeyCallback(glfwGetCurrentContext(), keyCallback);
                glfwSetGamepadStateCallback(gamepadStateCallback);
                ActiveMenu = new GameplayMenu;
                ActiveMenu->Load();
                break;
            }
            default:;
            }
        }

        switch (TheMenuManager.currentScreen) {
        case CALIBRATION: {
            static bool sampleLoaded = false;
            if (!sampleLoaded) {
                audioManager.loadSample("Assets/kick.wav", "click");
                sampleLoaded = true;
            }

            if (GuiButton(
                    { (float)GetScreenWidth() / 2 - 250,
                      (float)GetScreenHeight() - 120,
                      200,
                      60 },
                    "Start Calibration"
                )) {
                isCalibrating = true;
                calibrationStartTime = GetTime();
                lastClickTime = calibrationStartTime;
                tapTimes.clear();
            }
            if (GuiButton(
                    { (float)GetScreenWidth() / 2 + 50,
                      (float)GetScreenHeight() - 120,
                      200,
                      60 },
                    "Stop Calibration"
                )) {
                isCalibrating = false;

                if (tapTimes.size() > 1) {
                    double totalDifference = 0.0;
                    for (double tapTime : tapTimes) {
                        double expectedClickTime =
                            round((tapTime - calibrationStartTime) / clickInterval)
                                * clickInterval
                            + calibrationStartTime;
                        totalDifference += (tapTime - expectedClickTime);
                    }
                    settingsMain.avOffsetMS =
                        static_cast<int>((totalDifference / tapTimes.size()) * 1000);
                    // Convert to milliseconds
                    settingsMain.inputOffsetMS = settingsMain.avOffsetMS;
                    std::cout
                        << static_cast<int>((totalDifference / tapTimes.size()) * 1000)
                        << "ms of latency detected" << std::endl;
                }
                std::cout << "Stopped Calibration" << std::endl;
                tapTimes.clear();
            }

            if (isCalibrating) {
                double currentTime = GetTime();
                double elapsedTime = currentTime - lastClickTime;

                if (elapsedTime >= clickInterval) {
                    audioManager.playSample("click", 1);
                    lastClickTime += clickInterval;
                    // Increment by the interval to avoid missing clicks
                    std::cout << "Click" << std::endl;
                }

                if (IsKeyPressed(settingsMain.keybindOverdrive)) {
                    tapTimes.push_back(currentTime);
                    std::cout << "Input Registered" << std::endl;

                    showInputFeedback = true;
                    inputFeedbackStartTime = currentTime;
                    inputFeedbackAlpha = 1.0f;
                }
            }

            if (showInputFeedback) {
                double currentTime = GetTime();
                double timeSinceInput = currentTime - inputFeedbackStartTime;
                if (timeSinceInput > inputFeedbackDuration) {
                    showInputFeedback = false;
                } else {
                    inputFeedbackAlpha = 1.0f - (timeSinceInput / inputFeedbackDuration);
                }
            }

            if (showInputFeedback) {
                Color feedbackColor = {
                    0, 255, 0, static_cast<unsigned char>(inputFeedbackAlpha * 255)
                };
                DrawTextEx(
                    assets.rubikBold,
                    "Input Registered",
                    { static_cast<float>((GetScreenWidth() - u.hinpct(0.35f)) / 2),
                      static_cast<float>(GetScreenHeight() / 2) },
                    u.hinpct(0.05f),
                    0,
                    feedbackColor
                );
            }

            if (GuiButton(
                    { ((float)GetScreenWidth() / 2) - 350,
                      ((float)GetScreenHeight() - 60),
                      100,
                      60 },
                    "Cancel"
                )) {
                isCalibrating = false;
                settingsMain.avOffsetMS = settingsMain.prevAvOffsetMS;
                settingsMain.inputOffsetMS = settingsMain.prevInputOffsetMS;
                tapTimes.clear();

                settingsMain.saveSettings(directory / "settings.json");
                TheMenuManager.SwitchScreen(SETTINGS);
            }

            if (GuiButton(
                    { ((float)GetScreenWidth() / 2) + 250,
                      ((float)GetScreenHeight() - 60),
                      100,
                      60 },
                    "Apply"
                )) {
                isCalibrating = false;
                settingsMain.prevAvOffsetMS = settingsMain.avOffsetMS;
                settingsMain.prevInputOffsetMS = settingsMain.inputOffsetMS;
                tapTimes.clear();

                settingsMain.saveSettings(directory / "settings.json");
                TheMenuManager.SwitchScreen(SETTINGS);
            }

            break;
        }
        case MAIN_MENU:
        case SETTINGS:
        case SONG_SELECT:
        case READY_UP:
        case GAMEPLAY:
        case RESULTS:
        case CHART_LOADING_SCREEN:
        case SOUND_TEST:
        case CACHE_LOADING_SCREEN: {
            ActiveMenu->Draw();
            break;
        }
        }
        EndDrawing();

        if (!removeFPSLimit) {
            currentTime = GetTime();
            updateDrawTime = currentTime - previousTime;
            int Target = TheGameSettings.Framerate;
            if (TheMenuManager.currentScreen != GAMEPLAY)
                Target = menuFPS;

            if (Target > 0) {
                waitTime = (1.0f / (float)Target) - updateDrawTime;
                if (waitTime > 0.0) {
                    WaitTime((float)waitTime);
                    currentTime = GetTime();
                    deltaTime = (float)(currentTime - previousTime);
                }
            } else
                deltaTime = (float)updateDrawTime;

            previousTime = currentTime;
        }
    }
    CloseWindow();
    return 0;
}

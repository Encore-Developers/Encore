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
#include "gameplay/GameplayInputHandler.h"
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
Assets &assets = Assets::getInstance();
Encore::AudioManager TheAudioManager;
Encore::Settings TheGameSettings;
Encore::Discord TheGameRPC;

// OvershellRenderer overshellRenderer;

vector<std::string> ArgumentList::arguments;

#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH
#endif

#ifndef ENCORE_VERSION
#define ENCORE_VERSION
#endif

std::string encoreVersion = ENCORE_VERSION;
std::string commitHash = GIT_COMMIT_HASH;

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
    if (exists(directory / "settings.json") && !exists(directory / "settings-old.json")) {
        rename(directory / "settings.json", directory / "settings-old.json");
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


    if (TheGameSettings.VerticalSync) {

        SetConfigFlags(FLAG_VSYNC_HINT);
    }
    Encore::EncoreLog(
           LOG_INFO, TextFormat("Vertical sync: %d", vsyncArg)
       );
    if (!TheGameSettings.Fullscreen) {
        InitWindow(
            GetMonitorWidth(GetCurrentMonitor()) * 0.75f,
            GetMonitorHeight(GetCurrentMonitor()) * 0.75f,
            "Encore"
        );
        SET_WINDOW_WINDOWED();
        MaximizeWindow();
    } else {
        InitWindow(
            GetMonitorWidth(GetCurrentMonitor()),
            GetMonitorHeight(GetCurrentMonitor()),
            "Encore"
        );
        SET_WINDOW_FULLSCREEN_BORDERLESS();
    }

    TheAudioManager.Init();
    SetExitKey(0);
    TheAudioManager.loadSample("Assets/combobreak.mp3", "miss");

    ChangeDirectory(GetApplicationDirectory());

    SETDEFAULTSTYLE();

    SetRandomSeed(std::chrono::system_clock::now().time_since_epoch().count());
    assets.FirstAssets();
    SetWindowIcon(assets.icon);
    GuiSetFont(assets.rubik);
    assets.LoadAssets();
    TheMenuManager.currentScreen = CACHE_LOADING_SCREEN;
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
            TheMenuManager.LoadMenu();
        }

        TheMenuManager.DrawMenu();
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

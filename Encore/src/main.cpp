#define JSON_DIAGNOSTICS 1
#include "songlist.h"
#include "users/playerManager.h"
#include "menus/menu.h"
#include "gameplay/enctime.h"
#include <cassert>
#define assertm(exp, msg) assert((void(msg), exp))

#define RAYGUI_IMPLEMENTATION

#if defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#endif

#include <filesystem>
#include <vector>
#include <thread>

#include "raylib.h"
#include "raygui.h"

#include "arguments.h"
#include "../include/assets.h"
#include "song/audio.h"
#include "gameplay/gameplayRenderer.h"

#include "menus/uiUnits.h"

#include "settings.h"
#include "menus/SettingsAudioVideo.h"
#include "menus/SettingsController.h"
#include "menus/SettingsCredits.h"
#include "menus/SettingsGameplay.h"
#include "menus/SettingsKeyboard.h"

#include "menus/styles.h"
#include "util/frame-manager.h"
#include "util/settings-helper.h"

#include <menus/MenuManager.h>

MenuManager TheMenuManager;
gameplayRenderer TheGameRenderer;
SongList TheSongList;
PlayerManager ThePlayerManager;
Assets &assets = Assets::getInstance();
Encore::AudioManager TheAudioManager;
Encore::Settings TheGameSettings;
Encore::SettingsGameplay TheGameplaySettings;
Encore::SettingsAudioVideo TheAudioVideoSettings;
Encore::SettingsController TheControllerSettings;
Encore::SettingsKeyboard TheKeyboardSettings;
Encore::SettingsCredits TheCredits;
Encore::SettingsInit TheSettingsInitializer;
Encore::FrameManager TheFrameManager;

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

int minWidth = 640;
int minHeight = 480;

int main(int argc, char *argv[]) {
    SetTraceLogCallback(Encore::EncoreLog);
    Units u = Units::getInstance();
    commitHash.erase(7);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

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
    TheSettingsInitializer.InitSettings(directory);
    ThePlayerManager.SetPlayerListSaveFileLocation(directory / "players.json");
    ThePlayerManager.LoadPlayerList();

    if (TheGameSettings.VerticalSync) {
        SetConfigFlags(FLAG_VSYNC_HINT);
    }
    Encore::EncoreLog(LOG_INFO, TextFormat("Vertical sync: %d", vsyncArg));
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
    bool AudioInitSuccessful = TheAudioManager.Init();
    assert(AudioInitSuccessful == true);
    Encore::EncoreLog(LOG_INFO, "Audio successfully initialized");

    SetExitKey(0);
    TheAudioManager.loadSample("Assets/combobreak.mp3", "miss");
    TheFrameManager.InitFrameManager();
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
        TheFrameManager.removeFPSLimit = true;
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
        TheFrameManager.WaitForFrame();
    }
    CloseWindow();
    return 0;
}

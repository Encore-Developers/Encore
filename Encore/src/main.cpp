#define JSON_DIAGNOSTICS 1
#include "song/songlist.h"
#include "users/playerManager.h"
#include "menus/menu.h"
#include "util/discord.h"
#include "util/enclog.h"
#include "gameplay/enctime.h"


#include <cassert>

#include "settings/keybinds.h"
#include "song/cacheload.h"
#define assertm(exp, msg) assert((void(msg), exp))

#define RAYGUI_IMPLEMENTATION

/* not needed for debug purposes, change in release
#if defined(WIN32) && defined(NDEBUG)
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif
*/
#if defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#endif

#define JSON_DIAGNOSTICS 1

#include <filesystem>
#include <vector>
#include <thread>

#include "raylib.h"
#include "raygui.h"
#include "GLFW/glfw3.h"

#include "arguments.h"
#include "assets.h"
#include "song/audio.h"
#include "gameplay/gameplayRenderer.h"

#include "menus/uiUnits.h"

#include "settings/settings.h"
// #include "settings-old.h"
#include "menus/SettingsAudioVideo.h"
#include "menus/SettingsController.h"
#include "menus/SettingsCredits.h"
#include "menus/SettingsGameplay.h"
#include "menus/SettingsKeyboard.h"

#include "menus/styles.h"
#include "util/frame-manager.h"

#include <menus/MenuManager.h>

MenuManager TheMenuManager;
// gameplayRenderer TheGameRenderer;
SongList TheSongList;
PlayerManager ThePlayerManager;
Assets &assets = Assets::getInstance();
Encore::AudioManager TheAudioManager;
Encore::Settings TheGameSettings;
Encore::Keybinds TheGameKeybinds;
Encore::SettingsGameplay TheGameplaySettings;
Encore::SettingsAudioVideo TheAudioVideoSettings;
Encore::SettingsController TheControllerSettings;
Encore::SettingsKeyboard TheKeyboardSettings;
Encore::SettingsCredits TheCredits;
Encore::Discord TheGameRPC;
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

void DrawLoadingScreen(unsigned char alpha, float progress) {
    Texture icon = ASSET(faviconTex);
    float fade = (1.0 - (alpha/255.0));
    fade *= fade;
    unsigned char quadAlpha = 255*(1.0-fade);
    float scale = (GetScreenHeight() / 1080.0f) * 0.3;
    float iconScale = scale + fade * 0.5;
    int iconSize = (icon.height * iconScale)/2; // Dividing by 2 for centering
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0, 0, 0, quadAlpha});

    Vector2 screenCenter = {GetScreenWidth()/2.0f, GetScreenHeight()/2.0f};
    DrawRing(screenCenter, scale*300.0f, scale*320.0f, -90, 360-90, 64, {255, 255, 255, (unsigned char)(alpha/3)});
    DrawRing(screenCenter, scale*300.0f, scale*320.0f, -90, 360*progress-90, 64, {255, 255, 255, alpha});
    DrawTextureEx(icon, {GetScreenWidth()/2.0f-iconSize, GetScreenHeight()/2.0f-iconSize}, 0, iconScale, {255, 255, 255, alpha});
}

int main(int argc, char *argv[]) {
    TheGameRPC.Initialize();
    SetTraceLogCallback(Encore::EncoreLog);
    Units u = Units::getInstance();
    commitHash.erase(7);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    glfwWindowHint(GLFW_SAMPLES, 4);

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
    CacheLoad::StartLoad();
    ThePlayerManager.SetPlayerListSaveFileLocation(directory / "players.json");

    ThePlayerManager.LoadPlayerList();

    if (TheGameSettings.VerticalSync) {
        SetConfigFlags(FLAG_VSYNC_HINT);
    }
    Encore::EncoreLog(LOG_INFO, TextFormat("Vertical sync: %d", vsyncArg));
    InitWindow(800, 600, "Encore");
    if (!TheGameSettings.Fullscreen) {
        int x, y, width, height;
        glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), &x, &y, &width, &height);
        Encore::EncoreLog(LOG_INFO, TextFormat("Workarea of monitor %s: %i %i %i %i", glfwGetMonitorName(glfwGetPrimaryMonitor()), x, y, width, height));
        int windowWidth = width * 0.75;
        int windowHeight = height * 0.75;
        SetWindowPosition(width/2 - windowWidth/2 + x, height/2 - windowHeight/2 + y);
        SetWindowSize(windowWidth, windowHeight);
        ClearWindowState(FLAG_WINDOW_UNDECORATED);
        MaximizeWindow();
    } else {
        SetWindowSize(GetMonitorWidth(0), GetMonitorHeight(0));
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
    initialSet.StartLoad();
    mainMenuSet.StartLoad();
    AssetSet({ASSETPTR(favicon), ASSETPTR(faviconTex)}).BlockUntilLoaded();
    SetWindowIcon(LoadImageFromMemory(".png", ASSET(favicon), ASSET(favicon).GetFileSize()));
    if (!CacheLoad::finished) {
        TheMenuManager.currentScreen = CACHE_LOADING_SCREEN;
    } else {
        TheMenuManager.currentScreen = MAIN_MENU;
    }




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
        SetShaderValue(ASSET(bgShader), ASSET(bgShader).GetUniformLoc("time"), &bgTime, SHADER_UNIFORM_FLOAT);
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
        static bool showLoading = true;
        if (showLoading && !initialSet.PollLoaded(true)) {
            DrawLoadingScreen(255, initialSet.GetProgress());
        } else {
            static float loadingScreenFade = 1.0f;
            showLoading = false;
            if (TheMenuManager.onNewMenu) {
                TheMenuManager.LoadMenu();
            }
            TheGameRPC.Update();
            TheMenuManager.DrawMenu();
            if (loadingScreenFade > 0) {
                DrawLoadingScreen(255*loadingScreenFade, 1);
                float frameTime = GetFrameTime();
                // Cap the frame time here so the fade animation doesn't get skipped by the stutter caused by loading the song preview
                if (frameTime > 0.032) {
                    frameTime = 0.032;
                }
                loadingScreenFade -= frameTime*5.0f;
            }
        }
        EndDrawing();

        TheFrameManager.WaitForFrame();
    }
    CloseWindow();
    return 0;
}

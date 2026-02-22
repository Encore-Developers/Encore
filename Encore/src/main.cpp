#define JSON_DIAGNOSTICS 1
#include "song/songlist.h"
#include "users/playerManager.h"
#include "menus/menu.h"
#include "util/discord.h"
#include "util/enclog.h"
#include "gameplay/enctime.h"
#include "rlImGui.h"
#include "imgui.h"
#include "imgui_internal.h"

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
#include "menus/SettingsAudioVideo.h"
#include "menus/SettingsController.h"
#include "menus/SettingsCredits.h"
#include "menus/SettingsGameplay.h"
#include "menus/SettingsKeyboard.h"

#include "menus/styles.h"
#include "util/frame-manager.h"

#include <menus/MenuManager.h>
#include "debug/EncoreDebug.h"

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
    float scale = (GetRenderHeight() / 1080.0f) * 0.3;
    float iconScale = scale + fade * 0.5;
    int iconSize = (icon.height * iconScale)/2; // Dividing by 2 for centering
    DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), {0, 0, 0, quadAlpha});

    Vector2 screenCenter = {GetRenderWidth()/2.0f, GetRenderHeight()/2.0f};
    DrawRing(screenCenter, scale*300.0f, scale*320.0f, -90, 360-90, 64, {255, 255, 255, (unsigned char)(alpha/3)});
    DrawRing(screenCenter, scale*300.0f, scale*320.0f, -90, 360*progress-90, 64, {255, 255, 255, alpha});
    DrawTextureEx(icon, {GetRenderWidth()/2.0f-iconSize, GetRenderHeight()/2.0f-iconSize}, 0, iconScale, {255, 255, 255, alpha});
}

void SetImGuiTheme() {
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_FrameBg]                = ImVec4(0.41f, 0.16f, 0.48f, 0.54f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.72f, 0.26f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.67f, 0.26f, 0.98f, 0.67f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.35f, 0.16f, 0.48f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.67f, 0.26f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.63f, 0.24f, 0.88f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.67f, 0.26f, 0.98f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.63f, 0.26f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.72f, 0.26f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.62f, 0.06f, 0.98f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.72f, 0.26f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.65f, 0.26f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.70f, 0.26f, 0.98f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.54f, 0.10f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.47f, 0.10f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.65f, 0.26f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.65f, 0.26f, 0.98f, 0.95f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.67f, 0.26f, 0.98f, 0.80f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.44f, 0.18f, 0.58f, 0.86f);
    colors[ImGuiCol_TabSelected]            = ImVec4(0.51f, 0.20f, 0.68f, 1.00f);
    colors[ImGuiCol_TabSelectedOverline]    = ImVec4(0.67f, 0.26f, 0.98f, 1.00f);
    colors[ImGuiCol_TabDimmedSelected]      = ImVec4(0.32f, 0.14f, 0.42f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.68f, 0.26f, 0.98f, 0.20f);


}

bool imGuiLoaded = false;
ImFont *proggyForever;

int main(int argc, char *argv[]) {
    TheGameRPC.Initialize();
    SetTraceLogCallback(Encore::EncoreLog);
    Units u = Units::getInstance();
    commitHash.erase(7);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, true);

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
    TheGameSettings.UpdateFullscreen();
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
    TheAssets.AddRingsAndInstruments();
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
        glfwSwapInterval(TheGameSettings.VerticalSync ? 1 : 0);
        u.calcUnits();

        if (GetRenderWidth() < minWidth) {
            if (GetRenderHeight() < minHeight)
                SetWindowSize(minWidth, minHeight);
            else
                SetWindowSize(minWidth, GetRenderHeight());
        }
        if (GetRenderHeight() < minHeight) {
            if (GetRenderWidth() < minWidth)
                SetWindowSize(minWidth, minHeight);
            else
                SetWindowSize(GetRenderWidth(), minHeight);
        }

        if (!imGuiLoaded) {
            rlImGuiSetup(true);
            ImGui::GetCurrentContext()->FontSizeBase = 16;
            ImGui::GetStyle().FontSizeBase = 16;
            auto io = ImGui::GetIO();
            proggyForever = io.Fonts->AddFontDefaultVector();
            io.FontDefault = proggyForever;
            SetImGuiTheme();
            imGuiLoaded = true;
        }
        BeginDrawing();
        rlImGuiBegin();
        ImGui::PushFont(proggyForever, ImGui::GetStyle().FontSizeBase);
        ClearBackground(DARKGRAY);
        static bool showLoading = true;
        if (showLoading && !initialSet.PollLoaded(true)) {
            DrawLoadingScreen(255, initialSet.GetProgress());
        } else {
            double curTime = GetTime();
            float bgTime = curTime / 5.0f;
            SetShaderValue(ASSET(bgShader), ASSET(bgShader).GetUniformLoc("time"), &bgTime, SHADER_UNIFORM_FLOAT);
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

        if (EncoreDebug::showDebug) {
            EncoreDebug::DrawDebug();
        }

        ImGui::PopFont();
        rlImGuiEnd();
        EndDrawing();

        TheFrameManager.WaitForFrame();
    }
    CloseWindow();
    return 0;
}

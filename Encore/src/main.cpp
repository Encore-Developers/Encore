#define JSON_DIAGNOSTICS 1
#include "song/songlist.h"
#include "users/playerManager.h"
#include "util/discord.h"
#include "util/enclog.h"
#include "gameplay/enctime.h"
#include "rlImGui.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "tracy/Tracy.hpp"

#include <cassert>

#include "gameplay/inputCallbacks.h"
#include "SDL3/SDL.h"

#include "settings/keybinds.h"
#include "song/cacheload.h"

#ifdef STEAM
#include "steam_api.h"
#endif
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
std::filesystem::path prefsPath;

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

ImVec4 HueShiftColor(ImVec4 color, float shift) {
    Color rlCol = {(unsigned char)(color.x * 255),  (unsigned char)(color.y * 255), (unsigned char)(color.z * 255), (unsigned char)(color.w * 255)};
    auto hsv = ColorToHSV(rlCol);
    hsv.x += shift;
    rlCol = ColorFromHSV(hsv.x, hsv.y, hsv.z);
    return {rlCol.r / 255.0f, rlCol.g / 255.0f, rlCol.b / 255.0f, color.w};
}

void SetImGuiTheme() {
    ImVec4* colors = ImGui::GetStyle().Colors;
    const float shift = 60;
    std::vector<int> colorsToShift = {
        ImGuiCol_FrameBg,
        ImGuiCol_FrameBgHovered,
        ImGuiCol_FrameBgActive,
        ImGuiCol_TitleBgActive,
        ImGuiCol_CheckMark,
        ImGuiCol_SliderGrab,
        ImGuiCol_SliderGrabActive,
        ImGuiCol_Button,
        ImGuiCol_ButtonHovered,
        ImGuiCol_ButtonActive,
        ImGuiCol_Header,
        ImGuiCol_HeaderHovered,
        ImGuiCol_HeaderActive,
        ImGuiCol_SeparatorHovered,
        ImGuiCol_SeparatorActive,
        ImGuiCol_ResizeGripHovered,
        ImGuiCol_ResizeGripActive,
        ImGuiCol_TabHovered,
        ImGuiCol_Tab,
        ImGuiCol_TabSelected,
        ImGuiCol_TabSelectedOverline,
        ImGuiCol_TabDimmedSelected,
        ImGuiCol_ResizeGrip
    };
    for (auto col : colorsToShift) {
        colors[col] = HueShiftColor(colors[col], shift);
    }
}

void LocateDevAssets() {
    auto execPath = std::filesystem::path(GetApplicationDirectory());
    for (int i = 0; i < 5; i++) {
        execPath /= "..";
        execPath = std::filesystem::canonical(execPath);
        //Encore::EncoreLog(LOG_INFO, TextFormat("Scanning: %s", execPath.c_str()));
        if (std::filesystem::exists(execPath / "CMakeLists.txt")) {
            execPath = std::filesystem::canonical(execPath / "Encore/Assets/");
            Encore::EncoreLog(LOG_INFO, TextFormat("Found dev directory: %s", execPath.c_str()));
            TheAssets.setDirectory(execPath);
            break;
        }
    }
}

bool imGuiLoaded = false;
ImFont *imGuiFont;

int main(int argc, char *argv[]) {

#ifdef STEAM
    if (SteamAPI_RestartAppIfNecessary(4691230)) {
        return 1;
    }

    if (!SteamAPI_Init()) {
        printf("This is a Steam build of Encore - Steam must be running\n");
        return 1;
    }
#endif
    LocateDevAssets();

    ArgumentList::InitArguments(argc, argv);
    initialSet.StartLoad();
    AssetSet({ASSETPTR(favicon), ASSETPTR(faviconTex)}).StartLoad();

    std::string discordOff = ArgumentList::GetArgValue("discord");
    TheGameRPC.Initialize(discordOff);
    SetTraceLogCallback(Encore::EncoreLog);
    Units u = Units::getInstance();
    commitHash.erase(7);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, true);

    bool windowToggle = true;

    std::string FPSCapStringVal = ArgumentList::GetArgValue("fpscap");
    std::string vSyncOn = ArgumentList::GetArgValue("vsync");
    int targetFPSArg = -1;
    int vsyncArg = 1;

    std::filesystem::path executablePath(GetApplicationDirectory());
    // Bump only for completely breaking changes to player/settings format
    // Do not bump for cache format changes
    prefsPath = SDL_GetPrefPath("Encore", "v0.2.0");
    std::filesystem::path directory = prefsPath;
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
                assets.setDirectory(std::filesystem::path(resourcePath) / "Assets");
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
    {
        ZoneScopedN("Window Init")
        InitWindow(800, 600, "Encore");
    }
    TheGameSettings.UpdateFullscreen();
    bool AudioInitSuccessful = TheAudioManager.Init();
    assert(AudioInitSuccessful == true);
    Encore::EncoreLog(LOG_INFO, "Audio successfully initialized");

    SetExitKey(0);
    TheFrameManager.InitFrameManager();
    ChangeDirectory(GetApplicationDirectory());

    SETDEFAULTSTYLE();

    SetRandomSeed(std::chrono::system_clock::now().time_since_epoch().count());
    TheAssets.AddRingsAndInstruments();
    mainMenuSet.StartLoad();
    SetWindowIcon(LoadImageFromMemory(".png", ASSET(favicon), ASSET(favicon).GetFileSize()));
    if (!CacheLoad::finished) {
        TheMenuManager.currentScreen = CACHE_LOADING_SCREEN;
    } else {
        TheMenuManager.currentScreen = MAIN_MENU;
    }

    ControllerPoller poller;


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
        ZoneScopedN("Main Loop")
        glfwSwapInterval(TheGameSettings.VerticalSync ? 1 : 0);
        u.calcUnits();

        PollQueuedInputs(poller);

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
            ImGui::GetCurrentContext()->FontSizeBase = 20;
            ImGui::GetStyle().FontSizeBase = 20;
            SetImGuiTheme();
            imGuiLoaded = true;
        }
        BeginDrawing();
        rlImGuiBegin();
        if (ImGui::GetIO().WantCaptureMouse) {
            GuiLock();
        } else {
            GuiUnlock();
        }
        static bool imGuiFontLoaded = false;

        if (!imGuiFontLoaded && ASSET(JetBrainsMono).state == LOADED) {
            auto io = ImGui::GetIO();
            imGuiFont = io.Fonts->AddFontFromMemoryTTF(ASSET(JetBrainsMono).RawData(), ASSET(JetBrainsMono).RawDataSize());
            io.FontDefault = imGuiFont;
            imGuiFontLoaded = true;
        }

        if (imGuiFontLoaded) {
            ImGui::PushFont(imGuiFont, ImGui::GetStyle().FontSizeBase);
        }

        ClearBackground(DARKGRAY);
        static bool showLoading = true;
        static float loadingScreenFade = 1.0f;
        if (showLoading && !initialSet.PollLoaded(true)) {
            DrawLoadingScreen(255, initialSet.GetProgress());
        } else {
            double curTime = GetTime();
            float bgTime = curTime / 5.0f;
            SetShaderValue(ASSET(bgShader), ASSET(bgShader).GetUniformLoc("time"), &bgTime, SHADER_UNIFORM_FLOAT);
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

        if (imGuiFontLoaded) {
            ImGui::PopFont();
        }
        {
            ZoneScopedN("End Drawing")
            rlImGuiEnd();
            EndDrawing();
        }

        if (EncoreDebug::reloadQueued) {
            EncoreDebug::StartReloadAssets();
            showLoading = true;
            loadingScreenFade = 1.0f;
        }
        TheFrameManager.WaitForFrame();
        FrameMark;
    }
    poller.active = false;
    CloseWindow();
    return 0;
}

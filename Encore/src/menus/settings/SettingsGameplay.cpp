//
// Created by Jaydenz on 04/29/2025.
//

#include "SettingsGameplay.h"

#include "../MenuManager.h"
#include "../main/MainMenu.h"
#include "raygui.h"
#include "assets.h"
#include "settings/settings.h"
#include "settingsOptionRenderer.h"
#include "../uiUnits.h"
#include "gameplay/enctime.h"
#include "../overshell/OvershellMenu.h"
#include "imgui.h"
#include "SDL3/SDL_dialog.h"
#include "gameplay/inputCallbacks.h"
#include "misc/imgui_stdlib.h"
#include "util/settings-text.h"
#include "../overshell/OvershellHelper.h"
#include "song/ArtLoader.h"

bool ShowGameplaySettings = true;
bool AwesomenessDetection = false;
void SettingsGameplay::Draw() {
    if (!IsWindowReady()) {
        return;
    }
    // null checks not needed, references are never null + all of these things WILL be initialized on run
    Units& u = Units::getInstance();
    //if (&u == nullptr) {
    //    return;
    //}

    Assets& assets = Assets::getInstance();
    //if (&assets == nullptr) {
    //    return;
    //}

    //SettingsOld& settingsMain = SettingsOld::getInstance();
    //if (&settingsMain == nullptr) {
    //    return;
    //}

    SongTime& enctime = TheSongTime;
    // if (&enctime == nullptr) {
    // }

    settingsOptionRenderer sor;
    const float boxWidthPct = 0.55f;

    if (IsTextureValid(TheArtLoader.loadedArtBlur)) {
        GameMenu::DrawAlbumArtBackground();
    } else {
        DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), BLACK);
    }
    DrawRectangle(u.LeftSide, 0, u.winpct(1.0f), GetRenderHeight(), Color{0, 0, 0});

    float SidebarLeft = u.LeftSide + u.winpct(0.70f);
    float SidebarWidth = u.wpct(0.235f);
    float SidebarTop = u.hinpct(0.15f);
    float SidebarHeight = u.hpct(0.85f);
    float SidebarHeaderHeight = u.hinpct(0.10f);
    float borderWidth = u.winpct(0.002f);
    float innerTop = SidebarTop + borderWidth;

    DrawLineEx({SidebarLeft - borderWidth, SidebarTop}, {SidebarLeft - borderWidth, SidebarTop + SidebarHeight}, borderWidth, WHITE);
    DrawLineEx({SidebarLeft + SidebarWidth, SidebarTop}, {SidebarLeft + SidebarWidth, SidebarTop + SidebarHeight}, borderWidth, WHITE);
    DrawLineEx({SidebarLeft - borderWidth, SidebarTop + SidebarHeight}, {SidebarLeft + SidebarWidth + borderWidth, SidebarTop + SidebarHeight}, borderWidth, WHITE);
    DrawRectangle(SidebarLeft, SidebarTop, SidebarWidth, SidebarHeight, Color{31, 31, 50, 255});

    struct SidebarContent {
        const char* header;
        const char* body;
    };
    SidebarContent sidebarContents[] = {
        // sidebar text
        // fullscreen
        {
            "Fullscreen",
            "TBD"
        },
        {
            "Awesomeness Detection",
            "Lets Marie know that you are awesome!"
        },
        // scan Songs
        {
            "Scan Songs",
            "TBD"
        }
    };

    static int selectedIndex = 0;
    Vector2 mousePos = GetMousePosition();
    bool isHovering = false;

    const char* headerText = sidebarContents[selectedIndex].header;
    const char* sidebarBodyText = sidebarContents[selectedIndex].body;
    float headerFontSize = u.hinpct(0.030f);
    float headerLineSpacing = headerFontSize * 1.2f;
    std::vector<std::string> headerLines = split(headerText, "\n");
    float maxHeaderWidth = 0;
    for (const std::string& line : headerLines) {
        if (IsFontValid(assets.rubikBold)) {
            Vector2 lineSize = MeasureTextEx(assets.rubikBold, line.c_str(), headerFontSize, 0);
            if (lineSize.x > maxHeaderWidth) {
                maxHeaderWidth = lineSize.x;
            }
        }
    }
    float currentHeaderY = innerTop + u.hinpct(0.02f);
    for (const std::string& line : headerLines) {
        float lineX = SidebarLeft + (SidebarWidth - maxHeaderWidth) / 2;
        if (IsFontValid(assets.rubikBold)) {
            DrawTextEx(assets.rubikBold, line.c_str(), {lineX, currentHeaderY}, headerFontSize, 0, WHITE);
        }
        currentHeaderY += headerLineSpacing;
    }
    float bodyFontSize = u.hinpct(0.030f);
    float lineSpacing = bodyFontSize * 1.2f;
    std::vector<std::string> lines = split(sidebarBodyText, "\n");
    float currentY = SidebarTop + SidebarHeaderHeight + u.hinpct(0.02f);
    for (const std::string& line : lines) {
        if (IsFontValid(assets.rubik)) {
            Vector2 lineSize = MeasureTextEx(assets.rubik, line.c_str(), bodyFontSize, 0);
            float lineX = SidebarLeft + (SidebarWidth - lineSize.x) / 2;
            DrawTextEx(assets.rubik, line.c_str(), {lineX, currentY}, bodyFontSize, 0, WHITE);
            currentY += lineSpacing;
        }
    }

    encOS::DrawTopOvershell(0.15f);
    GameMenu::DrawVersion();
    GameMenu::DrawBottomOvershell();
    DrawOvershell();

    DrawTextEx(assets.rubik, "Main > Gameplay", {u.LeftSide, u.hpct(0.027f)}, u.hinpct(0.042f), 0, LIGHTGRAY);
    GameMenu::mhDrawText(assets.redHatDisplayBlack, "SETTINGS", {u.LeftSide, u.hpct(0.05f)}, u.hinpct(0.125f), WHITE, assets.sdfShader, LEFT);

    float EntryHeight = u.hinpct(0.05f);
    float EntryTop = u.hpct(0.15f);
    float EntryLeft = u.LeftSide;
    float EntryWidth = u.winpct(0.7);
    Color glowColor = Color{142, 13, 148, 220};
    float highlightBorderWidth = 4.0f;

    settings.Draw();

    float scanSongsTop = EntryTop + (EntryHeight) * (settings.settingsArray.size());
    Rectangle wholeBoxRect = {EntryLeft, scanSongsTop, EntryWidth, EntryHeight};

    ImGui::SetNextWindowPos({wholeBoxRect.x, wholeBoxRect.y+wholeBoxRect.height}, ImGuiCond_Always);
    ImGui::SetNextWindowSize({wholeBoxRect.width, wholeBoxRect.height*3});
    if (ImGui::Begin("Song Paths", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        const std::filesystem::path* toDelete = nullptr;
        for (const auto& path : TheGameSettings.SongPaths) {
            ImGui::PushID((void*)&path);
            if (ImGui::Button("Remove")) {
                toDelete = &path;
            }
            ImGui::SameLine();
            ImGui::Text("%s", path.generic_string().c_str());
            ImGui::PopID();
        }
        if (ImGui::Button("Add...")) {
            // lambdas upon lambdas :smile:
            ControllerPoller::instance->CallFuncOnSDLThread([] {SDL_ShowOpenFolderDialog([](void*, const char* const* filelist, int filter) {
                if (filelist) {
                    while (*filelist) {
                        TheGameSettings.SongPaths.push_back(*filelist);
                        filelist++;
                    }
                }
            }, nullptr, nullptr, nullptr, false);});
        }

        // note: removing this until i can figure out why it keeps fucking doing that
        // starting to really hate std::vector erase
        // if (!TheGameSettings.SongPaths.empty()) {
        //    for (auto it = TheGameSettings.SongPaths.begin(); it != TheGameSettings.SongPaths.end(); ++it) {
        //        if (&*it == toDelete) {
        //            TheGameSettings.SongPaths.erase(it);
        //            break;
        //        }
        //    }
        //}
    }
    ImGui::End();

    if (!isHovering) {
        selectedIndex = 0;
    }
}

#include <raylib.h>

void SettingsGameplay::KeyboardInputCallback(int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        Save();
        TheMenuManager.SwitchScreen(SETTINGS);
    }
}

void SettingsGameplay::ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event) {
    if (event.action == Encore::RhythmEngine::Action::PRESS) {
        switch (event.channel) {
        case Encore::RhythmEngine::InputChannel::LANE_2: {
            Save();
            TheMenuManager.SwitchScreen(SETTINGS);
            break;
        }
        }
    }
    //if (state.buttons[GLFW_GAMEPAD_BUTTON_B] == GLFW_PRESS) {
    //    Save();
    //    TheMenuManager.SwitchScreen(SETTINGS);
    //}
}

void SettingsGameplay::Load() {

    buttReg.buttMap.clear();
    NEWBUTTONACTION2(buttReg, STRUM_UP, "UP", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        settings.IncrementIndex(1);
    }, false)
    NEWBUTTONACTION2(buttReg, STRUM_DOWN, "DOWN", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        settings.IncrementIndex(-1);
    }, false)
    NEWBUTTONACTION2(buttReg, LANE_1, "Select", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
    })
    NEWBUTTONACTION2(buttReg, LANE_2, "Back", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        Save();
        TheMenuManager.SwitchScreen(SETTINGS);
    })
    NEWBUTTONACTION2(buttReg, INPUT_LEFT, "Lower", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        settings.Action(true);
    }, false)
    NEWBUTTONACTION2(buttReg, INPUT_RIGHT, "Raise", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        settings.Action(false);
    }, false)

    ScanSongsFunc = [this]() {
        if (TheGameSettings.SongPaths.empty()) {
            TraceLog(LOG_ERROR, "SongPaths is empty. Cannot scan songs.");
        } else {
            for (const auto& path : TheGameSettings.SongPaths) {
                TraceLog(LOG_INFO, "Scanning path: %s", path.string().c_str());
            }
            TheSongList.ScanSongs(TheGameSettings.SongPaths);
        }
    };
    TheGameSettings.LoadFromFile((TheGameSettings.directory / "settings.json").string());
    settings.Add(new Encore::SettingDoohickey::boolSettingObject("Fullscreen", &TheGameSettings.Fullscreen));
    settings.Add(new Encore::SettingDoohickey::boolSettingObject("Awesomeness Detection", &ad));
    // settings.Add(new Encore::SettingDoohickey::buttonSettingObject("Scan Songs", &ScanSongsFunc));




}

void SettingsGameplay::Save() {
    // SettingsOld& settingsMain = SettingsOld::getInstance();
    TheGameSettings.SaveToFile((TheGameSettings.directory / "settings.json").string());
}
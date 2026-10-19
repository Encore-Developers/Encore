//
// Created by Jaydenz on 04/29/2025.
//

#include "SettingsGameplay.h"

#include "SettingsMenu.h"
#include "../MenuManager.h"
#include "raygui.h"
#include "assets.h"
#include "settings/settings.h"
#include "settingsOptionRenderer.h"
#include "../util/uiUnits.h"
#include "../overshell/OvershellMenu.h"
#include "imgui.h"
#include "SDL3/SDL_dialog.h"
#include "gameplay/inputCallbacks.h"
#include "menus/util/GameMenu.h"
#include "menus/util/TextDisplay.h"
#include "menus/util/locale/Locale.h"
#include "song/ArtLoader.h"

using namespace Encore;

bool ShowGameplaySettings = true;
bool AwesomenessDetection = false;
void SettingsGameplay::Draw() {
    Assets& assets = Assets::getInstance();

    GameMenu::DrawAlbumArtBackground();
    DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), Color { 0, 0, 0, 128 });

    GameMenu::DrawTopOvershell(0.15f);
    GameMenu::DrawTopBarText();

    float TextPlacementTB = Units::hpct(0.05f);
    Text::lDrawText(assets.rubik, "settings.header.gameplay", {Units::LeftSide, Units::hpct(0.027f)}, Units::hinpct(0.042f), LIGHTGRAY, LEFT);
    Text::lDrawText(assets.redHatDisplayBlack, "settings.header.main", {Units::LeftSide, TextPlacementTB}, Units::hinpct(0.125f), WHITE, LEFT);
    settings.isOSOpen = isOSOpen();
    settings.Draw();
    float EntryHeight = Units::hinpct(0.05f);
    float EntryWidth = Units::winpct(0.7);
    float EntryLeft = ((Units::winpct(1.0) - EntryWidth) / 2) + Units::LeftSide;
    float EntryTop = Units::hpct(0.15f) + (EntryHeight * settings.settingsArray.size());
    Rectangle rect = {EntryLeft, EntryTop, EntryWidth, EntryHeight   };
    float scale = SDL_GetWindowDisplayScale(GetSDLWindow());
    ImGui::SetNextWindowPos({rect.x/scale, rect.y/scale}, ImGuiCond_Always);
    ImGui::SetNextWindowSize({rect.width/scale, rect.height*2.5f/scale});
    if (ImGui::Begin("Song Paths", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
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
            SDL_ShowOpenFolderDialog([](void*, const char* const* filelist, int filter) {
                if (filelist) {
                    while (*filelist) {
                        TheGameSettings.SongPaths.push_back(*filelist);
                        filelist++;
                    }
                }
            }, nullptr, nullptr, nullptr, false);
        }
        // starting to really hate std::vector erase
        for (auto it = TheGameSettings.SongPaths.begin(); it != TheGameSettings.SongPaths.end(); ++it) {
            if (&*it == toDelete) {
                TheGameSettings.SongPaths.erase(it);
                break;
            }
        }
    }
    ImGui::End();



    GameMenu::DrawBottomOvershell();
    buttReg.DrawPrompts(isOSOpen());
    DrawOvershell();
}

void SettingsGameplay::KeyboardInputCallback(SDL_KeyboardEvent* event) {
}

void SettingsGameplay::ControllerInputCallback(ControllerEvent event) {
    buttReg.HandleInput(event);
}

void SettingsGameplay::Load() {
    buttReg.buttMap.clear();
    NEWBUTTONACTION2(buttReg, STRUM_UP, "generic.up", {
        if (_action != Encore::Action::PRESS) return;
        if (ScanningSongs) return;
        settings.IncrementSelected(true);
    }, false)
    NEWBUTTONACTION2(buttReg, STRUM_DOWN, "generic.down", {
        if (_action != Encore::Action::PRESS) return;
        if (ScanningSongs) return;
        settings.IncrementSelected(false);
    }, false)
    NEWBUTTONACTION2(buttReg, LANE_1, "generic.select", {
        if (_action != Encore::Action::PRESS) return;
        if (ScanningSongs) return;
        auto type = settings.settingsArray.at(settings.selectedIndex)->GetType();
        if (type == SettingDoohickey::settingType::BUTTON_SETTING ||
            type == SettingDoohickey::settingType::BOOL_SETTING)
            settings.Action(false);
    })
    NEWBUTTONACTION2(buttReg, LANE_2, "settings.prompt.exit", {
        if (_action != Encore::Action::PRESS) return;
        if (ScanningSongs) return;
        Save();
        TheMenuManager.CreateAndSwitchMenu<SettingsMenu>();
    })
    // might as well take advantage of this copying
    NEWBUTTONACTION2(buttReg, LANE_3, "settings.prompt.exitWithoutSaving", {
        if (_action != Encore::Action::PRESS) return;
        if (ScanningSongs) return;
        TheMenuManager.CreateAndSwitchMenu<SettingsMenu>();
    })
    NEWBUTTONACTION2(buttReg, INPUT_LEFT, "Lower", {
        if (_action != Encore::Action::PRESS) return;
        if (ScanningSongs) return;
        settings.Action(true);
    }, false)
    NEWBUTTONACTION2(buttReg, INPUT_RIGHT, "Raise", {
        if (_action != Encore::Action::PRESS) return;
        if (ScanningSongs) return;
        settings.Action(false);
    }, false)

    Framerate = TheGameSettings.Framerate;
    VerticalSync = TheGameSettings.VerticalSync;
    Fullscreen = TheGameSettings.Fullscreen;
    AudioOffset = TheGameSettings.AudioOffset;
    VideoOffset = TheGameSettings.VideoOffset;
    ShowCalibrationInfo = TheGameSettings.ShowCalibrationInfo;

    scanSongsFunc = [this] {
        ScanSongs();
    };

    settings.Add(new SettingDoohickey::separatorObject("settings.separator.calibration"));
    settings.Add(new SettingDoohickey::intSettingObject("settings.audioOffset", &AudioOffset, -100, 400, 5));
    settings.Add(new SettingDoohickey::intSettingObject("settings.videoOffset", &VideoOffset, -100, 400, 5));
    settings.Add(new SettingDoohickey::boolSettingObject("settings.calibrationInfo", &ShowCalibrationInfo, 0, 1, 1));

    settings.Add(new SettingDoohickey::separatorObject("settings.separator.video"));
    settings.Add(new SettingDoohickey::intSettingObject("settings.framerate", &Framerate, 5, 2000, 5));
    settings.Add(new SettingDoohickey::boolSettingObject("settings.verticalSync", &VerticalSync, 0, 1, 1));
    settings.Add(new SettingDoohickey::boolSettingObject("settings.fullscreen", &Fullscreen, 0, 1, 1));
    settings.Add(new SettingDoohickey::boolSettingObject("settings.awesomenessDetection", &ad, 0, 1, 1));

    settings.Add(new SettingDoohickey::separatorObject("settings.separator.songs"));
    settings.Add(new SettingDoohickey::buttonSettingObject("settings.scanSongs", &scanSongsFunc));
}

void SettingsGameplay::Save() {
    TheGameSettings.AudioOffset = AudioOffset;
    TheGameSettings.VideoOffset = VideoOffset;
    TheGameSettings.Framerate = Framerate;
    TheGameSettings.VerticalSync = VerticalSync;
    TheGameSettings.Fullscreen = Fullscreen;
    TheGameSettings.ShowCalibrationInfo = ShowCalibrationInfo;
    TheGameSettings.SaveToFile((TheGameSettings.directory / "settings.json").string());
}
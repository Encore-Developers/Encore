//
// Created by Jaydenz on 04/29/2025.
//

#include "SettingsGameplay.h"

#include "SettingsMenu.h"
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
#include "menus/locale/Locale.h"
#include "song/ArtLoader.h"

using namespace Encore;

bool ShowGameplaySettings = true;
bool AwesomenessDetection = false;
void SettingsGameplay::Draw() {
    Units& u = Units::getInstance();
    Assets& assets = Assets::getInstance();

    GameMenu::DrawAlbumArtBackground();
    DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), Color { 0, 0, 0, 128 });

    encOS::DrawTopOvershell(0.15f);
    GameMenu::DrawVersion();

    float TextPlacementTB = u.hpct(0.05f);
    GameMenu::mhDrawText(assets.rubik, LOCALISE("settings.header.gameplay"), {u.LeftSide, u.hpct(0.027f)}, u.hinpct(0.042f), LIGHTGRAY, ASSET(sdfShader), LEFT);
    GameMenu::mhDrawText(assets.redHatDisplayBlack, LOCALISE("settings.header.main"), {u.LeftSide, TextPlacementTB}, u.hinpct(0.125f), WHITE, assets.sdfShader, LEFT);
    settings.isOSOpen = isOSOpen();
    settings.Draw();
    float EntryHeight = u.hinpct(0.05f);
    float EntryWidth = u.winpct(0.7);
    float EntryLeft = ((u.winpct(1.0) - EntryWidth) / 2) + u.LeftSide;
    float EntryTop = u.hpct(0.15f) + (EntryHeight * settings.settingsArray.size());
    Rectangle rect = {EntryLeft, EntryTop, EntryWidth, EntryHeight   };

    ImGui::SetNextWindowPos({rect.x, rect.y}, ImGuiCond_Always);
    ImGui::SetNextWindowSize({rect.width, rect.height*4});
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

void SettingsGameplay::KeyboardInputCallback(int key, int scancode, int action, int mods) {
}

void SettingsGameplay::ControllerInputCallback(RhythmEngine::ControllerEvent event) {
    buttReg.HandleInput(event);
}

void SettingsGameplay::Load() {
    buttReg.buttMap.clear();
    NEWBUTTONACTION2(buttReg, STRUM_UP, "UP", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        if (ScanningSongs) return;
        settings.IncrementSelected(true);
    }, false)
    NEWBUTTONACTION2(buttReg, STRUM_DOWN, "DOWN", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        if (ScanningSongs) return;
        settings.IncrementSelected(false);
    }, false)
    NEWBUTTONACTION2(buttReg, LANE_1, "generic.select", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        if (ScanningSongs) return;
        auto type = settings.settingsArray.at(settings.selectedIndex)->GetType();
        if (type == SettingDoohickey::settingType::BUTTON_SETTING ||
            type == SettingDoohickey::settingType::BOOL_SETTING)
            settings.Action(false);
    })
    NEWBUTTONACTION2(buttReg, LANE_2, "settings.prompt.exit", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        if (ScanningSongs) return;
        Save();
        TheMenuManager.CreateAndSwitchMenu<SettingsMenu>();
    })
    // might as well take advantage of this copying
    NEWBUTTONACTION2(buttReg, LANE_3, "settings.prompt.exitWithoutSaving", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        if (ScanningSongs) return;
        TheMenuManager.CreateAndSwitchMenu<SettingsMenu>();
    })
    NEWBUTTONACTION2(buttReg, INPUT_LEFT, "Lower", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        if (ScanningSongs) return;
        settings.Action(true);
    }, false)
    NEWBUTTONACTION2(buttReg, INPUT_RIGHT, "Raise", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        if (ScanningSongs) return;
        settings.Action(false);
    }, false)

    Framerate = TheGameSettings.Framerate;
    VerticalSync = TheGameSettings.VerticalSync;
    Fullscreen = TheGameSettings.Fullscreen;
    AudioOffset = TheGameSettings.AudioOffset;
    VideoOffset = TheGameSettings.VideoOffset;

    scanSongsFunc = [this] {
        ScanSongs();
    };

    settings.Add(new SettingDoohickey::separatorObject("settings.separator.calibration"));
    settings.Add(new SettingDoohickey::intSettingObject("settings.audioOffset", &AudioOffset, -100, 400, 5));
    settings.Add(new SettingDoohickey::intSettingObject("settings.videoOffset", &VideoOffset, -100, 400, 5));

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
    TheGameSettings.SaveToFile((TheGameSettings.directory / "settings.json").string());
}
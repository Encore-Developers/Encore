//
// Created by Jaydenz on 04/29/2025.
//

#include "SettingsAudioVideo.h"

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
#include "util/settings-text.h"
#include "../overshell/OvershellHelper.h"
#include "menus/locale/Locale.h"

bool ShowAudioVisualSettings = true;
bool showVolumeSettings = false;

// settings variables
int AudioOffset = 0;
int VideoOffset = 0;
int Framerate = 0;
float avMainVolume = 0.0f;
float avActiveInstrumentVolume = 0.0f;
float avInactiveInstrumentVolume = 0.0f;
float avMuteVolume = 0.0f;
float avMenuMusicVolume = 0.0f;
float avSoundEffectVolume = 0.0f;
bool BackgroundBeatFlash = false;
bool VerticalSync = false;

using namespace Encore;
SettingsAudioVideo::~SettingsAudioVideo() {};

void SettingsAudioVideo::Draw() {
    Units &u = Units::getInstance();
    Assets &assets = Assets::getInstance();

    GameMenu::DrawAlbumArtBackground();
    DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), Color { 0, 0, 0, 128 });

    encOS::DrawTopOvershell(0.15f);
    GameMenu::DrawVersion();
    GameMenu::DrawBottomOvershell();

    float TextPlacementTB = u.hpct(0.05f);
    Text::lDrawText(assets.rubik, "settings.header.audioVisual", {u.LeftSide, u.hpct(0.027f)}, u.hinpct(0.042f), LIGHTGRAY, LEFT);
    Text::lDrawText(assets.redHatDisplayBlack, "settings.header.main", {u.LeftSide, TextPlacementTB}, u.hinpct(0.125f), WHITE, LEFT);

    settings.isOSOpen = isOSOpen();
    settings.Draw();

    GameMenu::DrawBottomOvershell();
    buttReg.DrawPrompts(isOSOpen());
    DrawOvershell();
}


void SettingsAudioVideo::KeyboardInputCallback(int key, int scancode, int action, int mods) {
}

void SettingsAudioVideo::ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event) {
    buttReg.HandleInput(event);
    // if (state.buttons[GLFW_GAMEPAD_BUTTON_B] == GLFW_PRESS) {
    //    Save();
    //    TheMenuManager.SwitchScreen(SETTINGS);
    //}
}

void SettingsAudioVideo::Load() {
    buttReg.buttMap.clear();
    NEWBUTTONACTION2(buttReg, STRUM_UP, "UP", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        settings.IncrementSelected(true);
    }, false)
    NEWBUTTONACTION2(buttReg, STRUM_DOWN, "DOWN", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        settings.IncrementSelected(false);
    }, false)
    NEWBUTTONACTION2(buttReg, LANE_1, "generic.select", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
    })
    NEWBUTTONACTION2(buttReg, LANE_2, "settings.prompt.exit", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        Save();
        TheMenuManager.CreateAndSwitchMenu<SettingsMenu>();
    })
    // might as well take advantage of this copying
    NEWBUTTONACTION2(buttReg, LANE_3, "settings.prompt.exitWithoutSaving", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        TheMenuManager.CreateAndSwitchMenu<SettingsMenu>();
    })
    NEWBUTTONACTION2(buttReg, INPUT_LEFT, "Lower", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        settings.Action(true);
    }, false)
    NEWBUTTONACTION2(buttReg, INPUT_RIGHT, "Raise", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        settings.Action(false);
    }, false)




    avMainVolume = TheGameSettings.avMainVolume;
    avActiveInstrumentVolume = TheGameSettings.avActiveInstrumentVolume;
    avInactiveInstrumentVolume = TheGameSettings.avInactiveInstrumentVolume;
    avMuteVolume = TheGameSettings.avMuteVolume;
    avMenuMusicVolume = TheGameSettings.avMenuMusicVolume;
    avSoundEffectVolume = TheGameSettings.avSoundEffectVolume;
    avCrowdVolume = TheGameSettings.avCrowdVolume;
    avInactiveVocalsVolume = TheGameSettings.avInactiveVocalsVolume;
    BackgroundBeatFlash = TheGameSettings.BackgroundBeatFlash;

    settings.Add(new SettingDoohickey::separatorObject("settings.separator.volume"));
    settings.Add(new SettingDoohickey::floatSettingObject("settings.volume.global", &avMainVolume, 0, 1, 0.05f));
    settings.Add(new SettingDoohickey::floatSettingObject("settings.volume.active", &avActiveInstrumentVolume, 0, 1, 0.05f));
    settings.Add(new SettingDoohickey::floatSettingObject("settings.volume.mute", &avMuteVolume, 0, 1, 0.05f));
    settings.Add(new SettingDoohickey::floatSettingObject("settings.volume.vocals", &avInactiveVocalsVolume, 0, 1, 0.05f));
    settings.Add(new SettingDoohickey::floatSettingObject("settings.volume.crowd", &avCrowdVolume, 0, 1, 0.05f));
    settings.Add(new SettingDoohickey::floatSettingObject("settings.volume.inactive", &avInactiveInstrumentVolume, 0, 1, 0.05f));
    settings.Add(new SettingDoohickey::floatSettingObject("settings.volume.sfx", &avSoundEffectVolume, 0, 1, 0.05f));
    settings.Add(new SettingDoohickey::floatSettingObject("settings.volume.menuMusic", &avMenuMusicVolume, 0, 1, 0.05f));


    TraceLog(LOG_INFO, "Loaded audio/video settings: AudioOffset=%d, VideoOffset=%d, Framerate=%d, avMainVolume=%.2f",
             AudioOffset, VideoOffset, Framerate, avMainVolume);
}

void SettingsAudioVideo::Save() {

    TheGameSettings.avMainVolume = avMainVolume;
    TheGameSettings.avActiveInstrumentVolume = avActiveInstrumentVolume;
    TheGameSettings.avInactiveInstrumentVolume = avInactiveInstrumentVolume;
    TheGameSettings.avMuteVolume = avMuteVolume;
    TheGameSettings.avMenuMusicVolume = avMenuMusicVolume;
    TheGameSettings.avSoundEffectVolume = avSoundEffectVolume;
    TheGameSettings.BackgroundBeatFlash = BackgroundBeatFlash;
    TheGameSettings.avCrowdVolume = avCrowdVolume;
    TheGameSettings.avInactiveVocalsVolume = avInactiveVocalsVolume;

    TheGameSettings.SaveToFile((TheGameSettings.directory / "settings.json").string());

    TraceLog(LOG_INFO, "Saved audio/video settings: AudioOffset=%d, VideoOffset=%d, Framerate=%d, avMainVolume=%.2f",
             AudioOffset, VideoOffset, Framerate, avMainVolume);
}
//
// Created by Jaydenz on 04/29/2025.
//

#include "SettingsAudioVideo.h"

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
int selectedIndex = 0;

using namespace Encore;
SettingsAudioVideo::~SettingsAudioVideo() {};

void SettingsAudioVideo::Draw() {
    Units &u = Units::getInstance();
    Assets &assets = Assets::getInstance();

    GameMenu::DrawAlbumArtBackground();
    DrawRectangle(u.LeftSide, 0, u.winpct(1.0f), GetRenderHeight(), Color{0, 0, 0});

    encOS::DrawTopOvershell(0.15f);
    GameMenu::DrawVersion();
    GameMenu::DrawBottomOvershell();

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
        // Audio Calibration
        {
            "Audio Calibration",
            "Increases the time between the audio \nplayback and note display to make notes\n line up with the audio.\nThe higher the number, the later the\naudio, and the closer to the screen will\nthe notes line up."
        },
        // Video Calibration
        {
            "Video Calibration",
            "Adjusts the visuals of the notes as\nwell as the hit window. The higher the\nnumber, the later the video."
        },
        // Volume
        {
            "Volume Settings",
            "Placeholder"
        },
        // Main Output
        {
            "Main Output Volume",
            "Placeholder"
        },
        // Active Instrument
        {
            "Active Instrument Volume",
            "Placeholder"
        },
        // Inactive Instrument
        {
            "Inactive Instrument Volume",
            "Placeholder"
        },
        // Mute Instrument
        {
            "Mute Instrument Volume",
            "Placeholder"
        },
        // Menu Music
        {
            "Menu Music Volume",
            "Placeholder"
        },
        // Sound Effects
        {
            "Sound Effects Volume",
            "Placeholder"
        },
        // Background Beat Flash
        {
            "Background Beat Flash",
            "Placeholder"
        },
        // Framerate
        {
            "Framerate",
            "Placeholder"
        },
        // V-Sync
        {
            "V-Sync",
            "Placeholder"
        }
    };

    const char* headerText = sidebarContents[selectedIndex].header;
    const char* sidebarBodyText = sidebarContents[selectedIndex].body;
    float headerFontSize = u.hinpct(0.030f);
    float headerLineSpacing = headerFontSize * 1.2f;
    std::vector<std::string> headerLines = split(headerText, "\n");
    float maxHeaderWidth = 0;
    for (const std::string& line : headerLines) {
        Vector2 lineSize = MeasureTextEx(assets.rubikBold, line.c_str(), headerFontSize, 0);
        if (lineSize.x > maxHeaderWidth) {
            maxHeaderWidth = lineSize.x;
        }
    }
    float currentHeaderY = innerTop + u.hinpct(0.02f);
    for (const std::string& line : headerLines) {
        float lineX = SidebarLeft + (SidebarWidth - maxHeaderWidth) / 2;
        DrawTextEx(assets.rubikBold, line.c_str(), {lineX, currentHeaderY}, headerFontSize, 0, WHITE);
        currentHeaderY += headerLineSpacing;
    }
    float bodyFontSize = u.hinpct(0.030f);
    float lineSpacing = bodyFontSize * 1.2f;
    std::vector<std::string> lines = split(sidebarBodyText, "\n");
    float currentY = SidebarTop + SidebarHeaderHeight + u.hinpct(0.02f);
    for (const std::string& line : lines) {
        Vector2 lineSize = MeasureTextEx(assets.rubik, line.c_str(), bodyFontSize, 0);
        float lineX = SidebarLeft + (SidebarWidth - lineSize.x) / 2;
        DrawTextEx(assets.rubik, line.c_str(), {lineX, currentY}, bodyFontSize, 0, WHITE);
        currentY += lineSpacing;
    }

    float TextPlacementTB = u.hpct(0.05f);
    float TextPlacementLR = u.wpct(0.05f);
    DrawTextEx(assets.rubik, "Audio/Video", {TextPlacementLR, u.hpct(0.027f)}, u.hinpct(0.042f), 0, LIGHTGRAY);
    GameMenu::mhDrawText(assets.redHatDisplayBlack, "SETTINGS", {TextPlacementLR, TextPlacementTB}, u.hinpct(0.125f), WHITE, assets.sdfShader, LEFT);

    Color sliderNormal = Color{24, 24, 39, 178};
    Color sliderHovered = Color{84, 13, 88, 200};
    Color sliderFocused = Color{142, 13, 148, 220};
    GuiSetStyle(SLIDER, BASE_COLOR_NORMAL, ColorToInt(sliderNormal));
    GuiSetStyle(SLIDER, BASE_COLOR_FOCUSED, ColorToInt(sliderHovered));
    GuiSetStyle(SLIDER, BASE_COLOR_PRESSED, ColorToInt(sliderFocused));
    GuiSetStyle(SLIDER, TEXT_COLOR_FOCUSED, ColorToInt(sliderFocused));
    GuiSetStyle(SLIDER, BORDER_COLOR_NORMAL, 0xFFFFFFFF);
    GuiSetStyle(SLIDER, BORDER_COLOR_FOCUSED, 0xFFFFFFFF);
    GuiSetStyle(SLIDER, BORDER_WIDTH, 2);
    GuiSetStyle(SLIDER, SLIDER_WIDTH, 0);
    GuiSetStyle(SLIDER, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);

    settings.Draw(selectedIndex);

    GameMenu::DrawBottomOvershell();
    buttReg.DrawPrompts(isOSOpen());
    DrawOvershell();
}


void SettingsAudioVideo::KeyboardInputCallback(int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        Save();
        TheMenuManager.SwitchScreen(SETTINGS);
    }
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
        selectedIndex -= 1;
        if (selectedIndex < 0) selectedIndex = 0;
        if (selectedIndex >= settings.settingsArray.size()) selectedIndex = settings.settingsArray.size()-1;
    }, false)
    NEWBUTTONACTION2(buttReg, STRUM_DOWN, "DOWN", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        selectedIndex += 1;
        if (selectedIndex < 0) selectedIndex = 0;
        if (selectedIndex >= settings.settingsArray.size()) selectedIndex = settings.settingsArray.size()-1;
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
        settings.Action(selectedIndex, true);
    }, false)
    NEWBUTTONACTION2(buttReg, INPUT_RIGHT, "Raise", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        settings.Action(selectedIndex, false);
    }, false)
    TheGameSettings.LoadFromFile("settings.json");


    AudioOffset = TheGameSettings.AudioOffset;
    VideoOffset = TheGameSettings.VideoOffset;
    Framerate = TheGameSettings.Framerate;
    avMainVolume = TheGameSettings.avMainVolume;
    avActiveInstrumentVolume = TheGameSettings.avActiveInstrumentVolume;
    avInactiveInstrumentVolume = TheGameSettings.avInactiveInstrumentVolume;
    avMuteVolume = TheGameSettings.avMuteVolume;
    avMenuMusicVolume = TheGameSettings.avMenuMusicVolume;
    avSoundEffectVolume = TheGameSettings.avSoundEffectVolume;
    BackgroundBeatFlash = TheGameSettings.BackgroundBeatFlash;
    VerticalSync = TheGameSettings.VerticalSync;

    settings.Add(new SettingDoohickey::intSettingObject("Audio Offset", &AudioOffset, -100, 400, 5));
    settings.Add(new SettingDoohickey::intSettingObject("Video Offset", &VideoOffset, -100, 400, 5));
    settings.Add(new SettingDoohickey::intSettingObject("Framerate", &Framerate, 5, 2000, 5));
    settings.Add(new SettingDoohickey::boolSettingObject("Vsync", &VerticalSync, 0, 1, 1));
    settings.Add(new SettingDoohickey::floatSettingObject("Main Volume", &avMainVolume, 0, 1, 0.05f));
    settings.Add(new SettingDoohickey::floatSettingObject("Active Instrument Volume", &avActiveInstrumentVolume, 0, 1, 0.05f));
    settings.Add(new SettingDoohickey::floatSettingObject("Inactive Instrument Volume", &avInactiveInstrumentVolume, 0, 1, 0.05f));
    settings.Add(new SettingDoohickey::floatSettingObject("SFX Volume", &avSoundEffectVolume, 0, 1, 0.05f));
    settings.Add(new SettingDoohickey::floatSettingObject("Mute Volume", &avMuteVolume, 0, 1, 0.05f));
    settings.Add(new SettingDoohickey::floatSettingObject("Menu Music Volume", &avMenuMusicVolume, 0, 1, 0.05f));


    TraceLog(LOG_INFO, "Loaded audio/video settings: AudioOffset=%d, VideoOffset=%d, Framerate=%d, avMainVolume=%.2f",
             AudioOffset, VideoOffset, Framerate, avMainVolume);
}

void SettingsAudioVideo::Save() {
    TheGameSettings.AudioOffset = AudioOffset;
    TheGameSettings.VideoOffset = VideoOffset;
    TheGameSettings.Framerate = Framerate;
    TheGameSettings.avMainVolume = avMainVolume;
    TheGameSettings.avActiveInstrumentVolume = avActiveInstrumentVolume;
    TheGameSettings.avInactiveInstrumentVolume = avInactiveInstrumentVolume;
    TheGameSettings.avMuteVolume = avMuteVolume;
    TheGameSettings.avMenuMusicVolume = avMenuMusicVolume;
    TheGameSettings.avSoundEffectVolume = avSoundEffectVolume;
    TheGameSettings.BackgroundBeatFlash = BackgroundBeatFlash;
    TheGameSettings.VerticalSync = VerticalSync;

    TheGameSettings.SaveToFile("settings.json");

    TraceLog(LOG_INFO, "Saved audio/video settings: AudioOffset=%d, VideoOffset=%d, Framerate=%d, avMainVolume=%.2f",
             AudioOffset, VideoOffset, Framerate, avMainVolume);
}
//
// Created by marie on 17/11/2024.
//

#include "SettingsMenu.h"

#include "MenuManager.h"
#include "gameMenu.h"
#include "raygui.h"
#include "settings.h"
#include "settingsOptionRenderer.h"
#include "styles.h"
#include "uiUnits.h"
#include "gameplay/enctime.h"
#include "util/json-helper.h"

bool changingKey = false;
bool changing4k = false;
bool changingOverdrive = false;
bool changingAlt = false;
bool changingPause = false;

bool ShowHighwaySettings = true;
bool ShowCalibrationSettings = true;
bool ShowGeneralSettings = true;

enum OptionsCategories {
    MAIN,
    VOLUME,
    KEYBOARD,
    GAMEPAD
};

enum KeybindCategories {
    kbPAD,
    kbCLASSIC,
    kbMISC,
    kbMENUS
};

enum JoybindCategories {
    gpPAD,
    gpCLASSIC,
    gpMISC,
    gpMENUS
};

void SettingsMenu::Draw() {
    Keybinds keybinds;
    Units &u = Units::getInstance();
    Assets &assets = Assets::getInstance();
    SettingsOld &settingsMain = SettingsOld::getInstance();
    SongTime &enctime = TheSongTime;
    settingsOptionRenderer sor;
    GameMenu::DrawAlbumArtBackground(TheSongList.curSong->albumArtBlur);

    // Create Default Values


    std::filesystem::path directory = settingsMain.getDirectory();
    // if (settingsMain.controllerType == -1 && controllerID != -1) {
    //	std::string gamepadName = std::string(glfwGetGamepadName(controllerID));
    //	settingsMain.controllerType = keybinds.getControllerType(gamepadName);
    //}
    float TextPlacementTB = u.hpct(0.15f) - u.hinpct(0.11f);
    float TextPlacementLR = u.wpct(0.01f);
    DrawRectangle(
        u.LeftSide, 0, u.winpct(1.0f), GetScreenHeight(), Color { 0, 0, 0, 128 }
    );
    DrawLineEx(
        { u.LeftSide + u.winpct(0.0025f), 0 },
        { u.LeftSide + u.winpct(0.0025f), (float)GetScreenHeight() },
        u.winpct(0.005f),
        WHITE
    );
    DrawLineEx(
        { u.RightSide - u.winpct(0.0025f), 0 },
        { u.RightSide - u.winpct(0.0025f), (float)GetScreenHeight() },
        u.winpct(0.005f),
        WHITE
    );

    encOS::DrawTopOvershell(0.15f);
    GameMenu::DrawVersion();
    GameMenu::DrawBottomOvershell();
    DrawTextEx(
        assets.redHatDisplayBlack,
        "Options",
        { TextPlacementLR, TextPlacementTB },
        u.hinpct(0.10f),
        0,
        WHITE
    );

    float OvershellBottom = u.hpct(0.15f);
    if (GuiButton(
            { ((float)GetScreenWidth() / 2) - 350,
              ((float)GetScreenHeight() - 60),
              100,
              60 },
            "Cancel"
        )
        && !(changingKey || changingOverdrive || changingPause)) {
        // glfwSetGamepadStateCallback(origGamepadCallback);
        enctime.SetOffset(settingsMain.avOffsetMS / 1000.0);
        TheMenuManager.SwitchScreen(MAIN_MENU);
    }
    if (GuiButton(
            { ((float)GetScreenWidth() / 2) + 250,
              ((float)GetScreenHeight() - 60),
              100,
              60 },
            "Apply"
        )
        && !(changingKey || changingOverdrive || changingPause)) {
        // glfwSetGamepadStateCallback(origGamepadCallback);
        if (Fullscreen) {
            SET_WINDOW_FULLSCREEN_BORDERLESS();
            // SetWindowState(FLAG_WINDOW_UNDECORATED);
            // SetWindowState(FLAG_MSAA_4X_HINT);
            // int CurrentMonitor = GetCurrentMonitor();
            // SetWindowPosition(0, 0);
            // SetWindowSize(GetMonitorWidth(CurrentMonitor),
            //			GetMonitorHeight(CurrentMonitor));
        } else {
            SET_WINDOW_WINDOWED();
        }
#define OPTION(type, value, default) TheGameSettings.value = value;
        SETTINGS_OPTIONS;
#undef OPTION
        // player.InputOffset = settingsMain.inputOffsetMS / 1000.0f;
        // player.VideoOffset = settingsMain.avOffsetMS / 1000.0f;
        enctime.SetOffset(AudioOffset / 1000.0);
        Encore::WriteJsonFile(directory/"settings.json", TheGameSettings);
        settingsMain.saveSettings(directory / "settings-old.json");

        TheMenuManager.SwitchScreen(MAIN_MENU);
    }
    static int selectedTab = 0;
    static int displayedTab = 0;

    static int selectedKbTab = 0;
    static int displayedKbTab = 0;

    GuiToggleGroup(
        { u.LeftSide + u.winpct(0.005f),
          OvershellBottom,
          (u.winpct(0.985f) / 3),
          u.hinpct(0.05) },
        "Main;Volume;Keyboard Controls",
        &selectedTab
    );
    if (!changingKey && !changingOverdrive && !changingPause) {
        displayedTab = selectedTab;
    } else {
        selectedTab = displayedTab;
    }
    if (!changingKey && !changingOverdrive && !changingPause) {
        displayedKbTab = selectedKbTab;
    } else {
        selectedKbTab = displayedKbTab;
    }
    float EntryFontSize = u.hinpct(0.03f);
    float EntryHeight = u.hinpct(0.05f);
    float EntryTop = OvershellBottom + u.hinpct(0.1f);
    float HeaderTextLeft = u.LeftSide + u.winpct(0.015f);
    float EntryTextLeft = u.LeftSide + u.winpct(0.025f);
    float EntryTextTop = EntryTop + u.hinpct(0.01f);
    float OptionLeft = u.LeftSide + u.winpct(0.005f) + u.winpct(0.989f) / 3;
    float OptionWidth = u.winpct(0.989f) / 3;
    float OptionRight = OptionLeft + OptionWidth;

    float underTabsHeight = OvershellBottom + u.hinpct(0.05f);

    GuiSetStyle(SLIDER, BASE_COLOR_NORMAL, 0x181827FF);
    GuiSetStyle(
        SLIDER, BASE_COLOR_PRESSED, ColorToInt(ColorBrightness(AccentColor, -0.25f))
    );
    GuiSetStyle(
        SLIDER, TEXT_COLOR_FOCUSED, ColorToInt(ColorBrightness(AccentColor, -0.5f))
    );
    GuiSetStyle(SLIDER, BORDER_COLOR_NORMAL, 0xFFFFFFFF);
    GuiSetStyle(SLIDER, BORDER_COLOR_FOCUSED, 0xFFFFFFFF);
    GuiSetStyle(SLIDER, BORDER_WIDTH, 2);

    switch (displayedTab) {
    case MAIN: {
        // Main settings tab

        float trackSpeedFloat = settingsMain.trackSpeed;

        // header 1
        // calibration header
        int calibrationMenuOffset = 0;

        DrawRectangle(
            u.wpct(0.005f),
            underTabsHeight + (EntryHeight * calibrationMenuOffset),
            OptionWidth * 2,
            EntryHeight,
            Color { 0, 0, 0, 128 }
        );
        DrawTextEx(
            assets.rubikBoldItalic,
            "Audio/Video",
            { HeaderTextLeft,
              OvershellBottom + u.hinpct(0.055f)
                  + (EntryHeight * calibrationMenuOffset) },
            u.hinpct(0.04f),
            0,
            WHITE
        );

        // av offset

        AudioOffset = sor.sliderEntry(
            AudioOffset,
            -500.0f,
            500.0f,
            calibrationMenuOffset + 1,
            "Audio Offset",
            1
        );



        // framerate
        DrawRectangle(
            u.wpct(0.005f),
            underTabsHeight + (EntryHeight * (calibrationMenuOffset + 2)),
            OptionWidth * 2,
            EntryHeight,
            Color { 0, 0, 0, 64 }
        );



        float calibrationTop = EntryTop + (EntryHeight * (calibrationMenuOffset + 1));
        float calibrationTextTop =
            EntryTextTop + (EntryHeight * (calibrationMenuOffset + 1));
        DrawTextEx(
            assets.rubikBold,
            "Automatic Calibration",
            { EntryTextLeft, calibrationTextTop },
            EntryFontSize,
            0,
            WHITE
        );
        if (GuiButton(
                { OptionLeft, calibrationTop, OptionWidth, EntryHeight },
                "Start Calibration"
            )) {
            TheMenuManager.SwitchScreen(CALIBRATION);
        }

        int generalOffset = 3;
        // general header
        DrawRectangle(
            u.wpct(0.005f),
            underTabsHeight + (EntryHeight * generalOffset),
            OptionWidth * 2,
            EntryHeight,
            Color { 0, 0, 0, 128 }
        );
        DrawTextEx(
            assets.rubikBoldItalic,
            "General",
            { HeaderTextLeft,
              OvershellBottom + u.hinpct(0.055f) + (EntryHeight * generalOffset) },
            u.hinpct(0.04f),
            0,
            WHITE
        );

        // fullscreen

        Fullscreen =
            sor.toggleEntry(Fullscreen, generalOffset + 1, "Fullscreen");

        DrawRectangle(
            u.wpct(0.005f),
            underTabsHeight + (EntryHeight * (generalOffset + 2)),
            OptionWidth * 2,
            EntryHeight,
            Color { 0, 0, 0, 64 }
        );

        Framerate = sor.sliderEntry(
                            Framerate,
                            0,
                            1500,
                            generalOffset + 2,
                            "Max Framerate",
                            5
                        );

        VerticalSync = sor.toggleEntry(VerticalSync, generalOffset + 3, "VSync");

        DrawRectangle(
                    u.wpct(0.005f),
                    underTabsHeight + (EntryHeight * (generalOffset + 4)),
                    OptionWidth * 2,
                    EntryHeight,
                    Color { 0, 0, 0, 64 }
                );
        float scanTop = EntryTop + (EntryHeight * (generalOffset + 3));
        float scanTextTop = EntryTextTop + (EntryHeight * (generalOffset + 3));
        DrawTextEx(
            assets.rubikBold,
            "Scan Songs",
            { EntryTextLeft, scanTextTop },
            EntryFontSize,
            0,
            WHITE
        );
        if (GuiButton({ OptionLeft, scanTop, OptionWidth, EntryHeight }, "Scan")) {
            TheSongList.ScanSongs(TheGameSettings.SongPaths);
        }

        break;
    }
    case VOLUME: {
        // audio tab
        DrawRectangle(
            u.wpct(0.005f),
            OvershellBottom + u.hinpct(0.05f),
            OptionWidth * 2,
            EntryHeight,
            Color { 0, 0, 0, 128 }
        );
        DrawTextEx(
            assets.rubikBoldItalic,
            "Volume",
            { HeaderTextLeft, OvershellBottom + u.hinpct(0.055f) },
            u.hinpct(0.04f),
            0,
            WHITE
        );

        avMainVolume =
            sor.sliderEntry(avMainVolume, 0, 1, 1, "Main Volume", 0.05f);

        avActiveInstrumentVolume =
            sor.sliderEntry(avActiveInstrumentVolume, 0, 1, 2, "Active Instrument Volume", 0.05f);

        avInactiveInstrumentVolume =
            sor.sliderEntry(avInactiveInstrumentVolume, 0, 1, 3, "Inactive Instrument Volume", 0.05f);

        avSoundEffectVolume =
            sor.sliderEntry(avSoundEffectVolume, 0, 1, 4, "SFX Volume", 0.05f);

        avMuteVolume =
            sor.sliderEntry(avMuteVolume, 0, 1, 5, "Miss Volume", 0.05f);

        avMenuMusicVolume =
            sor.sliderEntry(avMenuMusicVolume, 0, 1, 6, "Menu Music Volume", 0.05f);

        // player.selInstVolume = settingsMain.MainVolume *
        // settingsMain.PlayerVolume; player.otherInstVolume =
        // settingsMain.MainVolume * settingsMain.BandVolume; player.sfxVolume =
        // settingsMain.MainVolume * settingsMain.SFXVolume; player.missVolume =
        // settingsMain.MainVolume * settingsMain.MissVolume;

        break;
    }
    case KEYBOARD: {
        // Keyboard bindings tab
        GuiToggleGroup(
            { u.LeftSide + u.winpct(0.005f),
              OvershellBottom + u.hinpct(0.05f),
              (u.winpct(0.987f) / 4),
              u.hinpct(0.05) },
            "Pad Binds;Classic Binds;Misc Gameplay;Menu Binds",
            &selectedKbTab
        );
        GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
        switch (displayedKbTab) {
        case kbPAD: {
            for (int i = 0; i < 5; i++) {
                sor.keybindEntryText(i + 2, "Lane " + std::to_string(i + 1));
                sor.keybind5kEntry(
                    settingsMain.keybinds5K[i],
                    i + 2,
                    "Lane " + std::to_string(i + 1),
                    keybinds,
                    i
                );
                sor.keybind5kAltEntry(
                    settingsMain.keybinds5KAlt[i],
                    i + 2,
                    "Lane " + std::to_string(i + 1),
                    keybinds,
                    i
                );
            }
            for (int i = 0; i < 4; i++) {
                sor.keybindEntryText(i + 8, "Lane " + std::to_string(i + 1));
                sor.keybind4kEntry(
                    settingsMain.keybinds4K[i],
                    i + 8,
                    "Lane " + std::to_string(i + 1),
                    keybinds,
                    i
                );
                sor.keybind4kAltEntry(
                    settingsMain.keybinds4KAlt[i],
                    i + 8,
                    "Lane " + std::to_string(i + 1),
                    keybinds,
                    i
                );
            }
            GuiSetStyle(DEFAULT, TEXT_SIZE, 28);
            break;
        }
        case kbCLASSIC: {
            DrawRectangle(
                u.wpct(0.005f),
                underTabsHeight + (EntryHeight * 2),
                OptionWidth * 2,
                EntryHeight,
                ColorAlpha(GREEN, 0.1f)
            );
            DrawRectangle(
                u.wpct(0.005f),
                underTabsHeight + (EntryHeight * 3),
                OptionWidth * 2,
                EntryHeight,
                ColorAlpha(RED, 0.1f)
            );
            DrawRectangle(
                u.wpct(0.005f),
                underTabsHeight + (EntryHeight * 4),
                OptionWidth * 2,
                EntryHeight,
                ColorAlpha(YELLOW, 0.1f)
            );
            DrawRectangle(
                u.wpct(0.005f),
                underTabsHeight + (EntryHeight * 5),
                OptionWidth * 2,
                EntryHeight,
                ColorAlpha(BLUE, 0.1f)
            );
            DrawRectangle(
                u.wpct(0.005f),
                underTabsHeight + (EntryHeight * 6),
                OptionWidth * 2,
                EntryHeight,
                ColorAlpha(ORANGE, 0.1f)
            );
            for (int i = 0; i < 5; i++) {
                sor.keybindEntryText(i + 2, "Lane " + std::to_string(i + 1));
                sor.keybind5kEntry(
                    settingsMain.keybinds5K[i],
                    i + 2,
                    "Lane " + std::to_string(i + 1),
                    keybinds,
                    i
                );
                sor.keybind5kAltEntry(
                    settingsMain.keybinds5KAlt[i],
                    i + 2,
                    "Lane " + std::to_string(i + 1),
                    keybinds,
                    i
                );
            }
            sor.keybindEntryText(8, "Strum Up");
            sor.keybindStrumEntry(0, 8, settingsMain.keybindStrumUp, keybinds);

            sor.keybindEntryText(9, "Strum Down");
            sor.keybindStrumEntry(1, 9, settingsMain.keybindStrumDown, keybinds);

            break;
        }
        case kbMISC: {
            sor.keybindEntryText(2, "Overdrive");
            sor.keybindOdAltEntry(
                settingsMain.keybindOverdriveAlt, 2, "Overdrive Alt", keybinds
            );
            sor.keybindOdEntry(settingsMain.keybindOverdrive, 2, "Overdrive", keybinds);

            sor.keybindEntryText(3, "Pause Song");
            sor.keybindPauseEntry(settingsMain.keybindPause, 3, "Pause", keybinds);
            break;
        }
        case kbMENUS: {
            break;
        }
        }
        if (sor.changingKey) {
            std::vector<int> &bindsToChange = sor.changingAlt
                ? (sor.changing4k ? settingsMain.keybinds4KAlt
                                  : settingsMain.keybinds5KAlt)
                : (sor.changing4k ? settingsMain.keybinds4K : settingsMain.keybinds5K);
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), { 0, 0, 0, 200 });
            std::string keyString = (sor.changing4k ? "4k" : "5k");
            std::string altString = (sor.changingAlt ? " alt" : "");
            std::string changeString =
                "Press a key for " + keyString + altString + " lane ";
            GameMenu::mhDrawText(
                assets.rubik,
                changeString.c_str(),
                { ((float)GetScreenWidth()) / 2, (float)GetScreenHeight() / 2 - 30 },
                20,
                WHITE,
                assets.sdfShader,
                CENTER
            );
            int pressedKey = GetKeyPressed();
            if (GuiButton(
                    { ((float)GetScreenWidth() / 2) - 130,
                      GetScreenHeight() - 60.0f,
                      120,
                      40 },
                    "Unbind Key"
                )) {
                pressedKey = -2;
            }
            if (GuiButton(
                    { ((float)GetScreenWidth() / 2) + 10,
                      GetScreenHeight() - 60.0f,
                      120,
                      40 },
                    "Cancel"
                )) {
                sor.selLane = 0;
                sor.changingKey = false;
            }
            if (pressedKey != 0) {
                bindsToChange[sor.selLane] = pressedKey;
                sor.selLane = 0;
                sor.changingKey = false;
            }
        }
        if (sor.changingOverdrive) {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), { 0, 0, 0, 200 });
            std::string altString = (sor.changingAlt ? " alt" : "");
            std::string changeString = "Press a key for " + altString + " overdrive";
            GameMenu::mhDrawText(
                assets.rubik,
                changeString.c_str(),
                { ((float)GetScreenWidth()) / 2, (float)GetScreenHeight() / 2 - 30 },
                20,
                WHITE,
                assets.sdfShader,
                CENTER
            );
            int pressedKey = GetKeyPressed();
            if (GuiButton(
                    { ((float)GetScreenWidth() / 2) - 130,
                      GetScreenHeight() - 60.0f,
                      120,
                      40 },
                    "Unbind Key"
                )) {
                pressedKey = -2;
            }
            if (GuiButton(
                    { ((float)GetScreenWidth() / 2) + 10,
                      GetScreenHeight() - 60.0f,
                      120,
                      40 },
                    "Cancel"
                )) {
                sor.changingAlt = false;
                sor.changingOverdrive = false;
            }
            if (pressedKey != 0) {
                if (sor.changingAlt)
                    settingsMain.keybindOverdriveAlt = pressedKey;
                else
                    settingsMain.keybindOverdrive = pressedKey;
                sor.changingOverdrive = false;
            }
        }
        if (sor.changingPause) {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), { 0, 0, 0, 200 });
            GameMenu::mhDrawText(
                assets.rubik,
                "Press a key for Pause",
                { ((float)GetScreenWidth()) / 2, (float)GetScreenHeight() / 2 - 30 },
                20,
                WHITE,
                assets.sdfShader,
                CENTER
            );
            int pressedKey = GetKeyPressed();
            if (GuiButton(
                    { ((float)GetScreenWidth() / 2) - 130,
                      GetScreenHeight() - 60.0f,
                      120,
                      40 },
                    "Unbind Key"
                )) {
                pressedKey = -2;
            }
            if (GuiButton(
                    { ((float)GetScreenWidth() / 2) + 10,
                      GetScreenHeight() - 60.0f,
                      120,
                      40 },
                    "Cancel"
                )) {
                sor.changingAlt = false;
                sor.changingPause = false;
            }
            if (pressedKey != 0) {
                settingsMain.keybindPause = pressedKey;
                sor.changingPause = false;
            }
        }
        if (sor.changingStrumUp) {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), { 0, 0, 0, 200 });
            GameMenu::mhDrawText(
                assets.rubik,
                "Press a key for Strum Up",
                { ((float)GetScreenWidth()) / 2, (float)GetScreenHeight() / 2 - 30 },
                20,
                WHITE,
                assets.sdfShader,
                CENTER
            );
            int pressedKey = GetKeyPressed();
            if (GuiButton(
                    { ((float)GetScreenWidth() / 2) - 130,
                      GetScreenHeight() - 60.0f,
                      120,
                      40 },
                    "Unbind Key"
                )) {
                pressedKey = -2;
            }
            if (GuiButton(
                    { ((float)GetScreenWidth() / 2) + 10,
                      GetScreenHeight() - 60.0f,
                      120,
                      40 },
                    "Cancel"
                )) {
                sor.changingAlt = false;
                sor.changingStrumUp = false;
            }
            if (pressedKey != 0) {
                settingsMain.keybindStrumUp = pressedKey;
                sor.changingStrumUp = false;
            }
        }
        if (sor.changingStrumDown) {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), { 0, 0, 0, 200 });
            GameMenu::mhDrawText(
                assets.rubik,
                "Press a key for Strum Down",
                { ((float)GetScreenWidth()) / 2, (float)GetScreenHeight() / 2 - 30 },
                20,
                WHITE,
                assets.sdfShader,
                CENTER
            );
            int pressedKey = GetKeyPressed();
            if (GuiButton(
                    { ((float)GetScreenWidth() / 2) - 130,
                      GetScreenHeight() - 60.0f,
                      120,
                      40 },
                    "Unbind Key"
                )) {
                pressedKey = -2;
            }
            if (GuiButton(
                    { ((float)GetScreenWidth() / 2) + 10,
                      GetScreenHeight() - 60.0f,
                      120,
                      40 },
                    "Cancel"
                )) {
                sor.changingAlt = false;
                sor.changingStrumDown = false;
            }
            if (pressedKey != 0) {
                settingsMain.keybindStrumDown = pressedKey;
                sor.changingStrumDown = false;
            }
        }
        break;
    }
    case GAMEPAD: { /*
         //Controller bindings tab
         GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
         for (int i = 0; i < 5; i++) {
             float j = (float) i - 2.0f;
             if (GuiButton({
                             ((float) GetScreenWidth() / 2) - 40 + (
                                 80 * j),
                             240, 80, 60
                         },
                         keybinds.getControllerStr(
                             controllerID,
                             settingsMain.controller5K[i],
                             settingsMain.controllerType,
                             settingsMain.controller5KAxisDirection[
                                 i]).c_str())) {
                 changing4k = false;
                 selLane = i;
                 changingKey = true;
             }
         }
         for (int i = 0; i < 4; i++) {
             float j = (float) i - 1.5f;
             if (GuiButton({
                             ((float) GetScreenWidth() / 2) - 40 + (
                                 80 * j),
                             360, 80, 60
                         },
                         keybinds.getControllerStr(
                             controllerID,
                             settingsMain.controller4K[i],
                             settingsMain.controllerType,
                             settingsMain.controller4KAxisDirection[
                                 i]).c_str())) {
                 changing4k = true;
                 selLane = i;
                 changingKey = true;
             }
         }
         if (GuiButton({((float) GetScreenWidth() / 2) - 40, 480, 80, 60},
                     keybinds.getControllerStr(
                         controllerID, settingsMain.controllerOverdrive,
                         settingsMain.controllerType,
                         settingsMain.controllerOverdriveAxisDirection).
                     c_str())) {
             changingKey = false;
             changingOverdrive = true;
         }
         if (GuiButton({((float) GetScreenWidth() / 2) - 40, 560, 80, 60},
                     keybinds.getControllerStr(
                         controllerID, settingsMain.controllerPause,
                         settingsMain.controllerType,
                         settingsMain.controllerPauseAxisDirection).
                     c_str())) {
             changingKey = false;
             changingOverdrive = true;
         }
         if (changingKey) {
             std::vector<int> &bindsToChange = (changing4k
                                                     ? settingsMain.
                                                     controller4K
                                                     : settingsMain.
                                                     controller5K);
             std::vector<int> &directionToChange = (changing4k
                                                         ? settingsMain.
                                                         controller4KAxisDirection
                                                         : settingsMain.
                                                         controller5KAxisDirection);
             DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                         {0, 0, 0, 200});
             std::string keyString = (changing4k ? "4k" : "5k");
             std::string changeString =
                     "Press a button/axis for controller " +
                     keyString + " lane " +
                     std::to_string(selLane + 1);
             DrawTextRubik(changeString.c_str(),
                         ((float) GetScreenWidth() - MeasureTextRubik(
                             changeString.c_str(), 20)) / 2,
                         GetScreenHeight() / 2 - 30, 20, WHITE);
             if (GuiButton({
                             ((float) GetScreenWidth() / 2) - 60,
                             GetScreenHeight() - 60.0f, 120, 40
                         },
                         "Cancel")) {
                 changingKey = false;
             }
             if (pressedGamepadInput != -999) {
                 bindsToChange[selLane] = pressedGamepadInput;
                 if (pressedGamepadInput < 0) {
                     directionToChange[selLane] = axisDirection;
                 }
                 selLane = 0;
                 changingKey = false;
                 pressedGamepadInput = -999;
             }
         }
         if (changingOverdrive) {
             DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                         {0, 0, 0, 200});
             std::string changeString =
                     "Press a button/axis for controller overdrive";
             DrawTextRubik(changeString.c_str(),
                         ((float) GetScreenWidth() - MeasureTextRubik(
                             changeString.c_str(), 20)) / 2,
                         GetScreenHeight() / 2 - 30, 20, WHITE);
             if (GuiButton({
                             ((float) GetScreenWidth() / 2) - 60,
                             GetScreenHeight() - 60.0f, 120, 40
                         },
                         "Cancel")) {
                 changingOverdrive = false;
             }
             if (pressedGamepadInput != -999) {
                 settingsMain.controllerOverdrive = pressedGamepadInput;
                 if (pressedGamepadInput < 0) {
                     settingsMain.controllerOverdriveAxisDirection =
                             axisDirection;
                 }
                 changingOverdrive = false;
                 pressedGamepadInput = -999;
             }
         }
         if (changingPause) {
             DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                         {0, 0, 0, 200});
             std::string changeString =
                     "Press a button/axis for controller pause";
             DrawTextRubik(changeString.c_str(),
                         ((float) GetScreenWidth() - MeasureTextRubik(
                             changeString.c_str(), 20)) / 2,
                         GetScreenHeight() / 2 - 30, 20, WHITE);
             if (GuiButton({
                             ((float) GetScreenWidth() / 2) - 60,
                             GetScreenHeight() - 60.0f, 120, 40
                         },
                         "Cancel")) {
                 changingPause = false;
             }
             if (pressedGamepadInput != -999) {
                 settingsMain.controllerPause = pressedGamepadInput;
                 if (pressedGamepadInput < 0) {
                     settingsMain.controllerPauseAxisDirection =
                             axisDirection;
                 }
                 changingPause = false;
                 pressedGamepadInput = -999;
             }
         }
         GuiSetStyle(DEFAULT, TEXT_SIZE, 28);
         break;*/
    }
    }
}
void SettingsMenu::Load() {
#define OPTION(type, value, default) value = TheGameSettings.value;
    SETTINGS_OPTIONS;
#undef OPTION
}
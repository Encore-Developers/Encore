//
// Created by Jaydenz on 27/04/2025.
//

#include "SettingsMenu.h"

#include "MenuManager.h"
#include "gameMenu.h"
#include "raygui.h"
#include "settings.h"
#include "settingsOptionRenderer.h"
#include "uiUnits.h"
#include "gameplay/enctime.h"
#include "util/json-helper.h"
#include "assets.h"
#include "OvershellMenu.h"

bool changingKey = false;
bool changing4k = false;
bool changingOverdrive = false;
bool changingAlt = false;
bool changingPause = false;

bool ShowHighwaySettings = true;
bool ShowCalibrationSettings = true;
bool ShowGeneralSettings = true;

enum OptionsCategories {
    AUDIO_VISUAL,
    GAMEPLAY_SETTINGS,
    CONTROLLER_BINDINGS,
    KEYBOARD_BINDINGS,
    CREDITS
};

std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0, end = 0;
    while ((end = str.find(delimiter, start)) != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
    }
    tokens.push_back(str.substr(start));
    return tokens;
}

void SettingsMenu::Draw() {
    Units &u = Units::getInstance();
    Assets &assets = Assets::getInstance();
    SettingsOld &settingsMain = SettingsOld::getInstance();
    SongTime &enctime = TheSongTime;
    settingsOptionRenderer sor;
    encOS::DrawTopOvershell(0.15f);
    GameMenu::DrawAlbumArtBackground(TheSongList.curSong->albumArtBlur);
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{50, 0, 0, 200});

    float TextPlacementTB = u.hpct(0.05f);
    float TextPlacementLR = u.wpct(0.05f);
    float EntryFontSize = u.hinpct(0.075f);
    float EntryHeight = u.hinpct(0.07f);
    float EntryTop = u.hpct(0.30f);
    float TextLineTop = u.hpct(0.36f);
    float TextTop = u.hpct(0.31f);
    float EntryTextLeft = u.wpct(0.05f);
    float SidebarLeft = u.wpct(0.75f);
    float SidebarWidth = u.wpct(0.16f);
    float SidebarTop = u.hinpct(0.10f);
    float SidebarHeight = u.hpct(0.85f);
    float SidebarHeaderHeight = u.hinpct(0.14f);
    float borderWidth = u.winpct(0.05f);
    float innerTop = SidebarTop + borderWidth;


    DrawRectangle(SidebarLeft - u.winpct(0.004f), SidebarTop- u.hinpct(0.08f) - u.winpct(0.004f),
    SidebarWidth + u.winpct(0.008f), SidebarHeight + u.winpct(0.02f), WHITE);
    DrawRectangle(SidebarLeft, SidebarTop - u.hinpct(0.02f), SidebarWidth, SidebarHeight, Color{31,31,50,255});

    const char *headerText = "Configure audio and video \n settings";
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
    float currentHeaderY = innerTop;
    for (const std::string& line : headerLines) {
        float lineX = SidebarLeft + (SidebarWidth - maxHeaderWidth) / 2;
        DrawTextEx(assets.rubikBold, line.c_str(), {lineX, currentHeaderY}, headerFontSize, 0, WHITE);
        currentHeaderY += headerLineSpacing;
    }

    const char *sidebarBodyText = "Configurable settings: \n- Audio calibration\n- Game volume\n- Background beat flash\n- Framerate\n- V-Sync\n";
    float bodyFontSize = u.hinpct(0.030f);
    float lineSpacing = bodyFontSize * 1.2f;

    std::vector<std::string> lines = split(sidebarBodyText, "\n");

    float currentY = SidebarTop + SidebarHeaderHeight + u.hinpct(0.05f);
    for (const std::string& line : lines) {
        Vector2 lineSize = MeasureTextEx(assets.rubik, line.c_str(), bodyFontSize, 0);
        float lineX = SidebarLeft + (SidebarWidth - lineSize.x) / 2;
        DrawTextEx(assets.rubik, line.c_str(), {lineX, currentY}, bodyFontSize, 0, WHITE);
        currentY += lineSpacing;
    }
    encOS::DrawTopOvershell(0.15f);

    DrawTextEx(assets.rubik, "Main Menu",
    {TextPlacementLR, u.hpct(0.027f)}, u.hinpct(0.042f), 0, LIGHTGRAY);
    GameMenu::mhDrawText(
    assets.redHatDisplayBlack,
    "SETTINGS",
    { TextPlacementLR, TextPlacementTB },
    u.hinpct(0.125f),
    WHITE,
    assets.sdfShader,
    LEFT
);
    float fontSize = u.hinpct(0.029f);
    DrawTextEx(assets.josefinSansItalic, "Select an option", {EntryTextLeft, TextTop}, fontSize, 0, Color{136, 136, 136, 255});

    DrawLineEx({EntryTextLeft, TextLineTop}, {EntryTextLeft + u.wpct(0.42f), TextLineTop}, u.winpct(0.001f), WHITE);

    // menu options
    const char *menuItems[] = {
        "Audio / Visual",
        "Gameplay",
        "Controller Bindings",
        "Keyboard Bindings",
        "Credits"
    };
    int menuItemCount = 5;
    static int selectedIndex = -1; // -1 means no item is selected
    static float clickFeedbackTimer = 0.0f;
    static int clickedIndex = -1;
    if (clickFeedbackTimer > 0) {
        clickFeedbackTimer -= GetFrameTime();
        if (clickFeedbackTimer <= 0) {
            clickedIndex = -1;
        }
    }
    // mouse controls
    Vector2 mousePos = GetMousePosition();
    for (int i = 0; i < menuItemCount; i++) {
        float yPos = EntryTop + (EntryHeight * (i + 1));
        Rectangle itemRect = {
            EntryTextLeft - u.wpct(0.01f),
            yPos,
            u.wpct(0.32f),
            EntryHeight
        };
        if (CheckCollisionPointRec(mousePos, itemRect)) {
            selectedIndex = i;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                clickFeedbackTimer = 0.2f;
                clickedIndex = i;
                switch (i) {
                case AUDIO_VISUAL:
                    TheMenuManager.SwitchScreen(CALIBRATION);
                    break;
                case GAMEPLAY_SETTINGS:
                    break;
                case CONTROLLER_BINDINGS:
                    break;
                case KEYBOARD_BINDINGS:
                    break;
                case CREDITS:
                    break;
                }
                printf("Clicked: %s\n", menuItems[i]);
            }
        }
        // menu select icon
        if (i == selectedIndex) {
            Color textColor = (i == clickedIndex) ? GRAY : WHITE;
            std::string modifiedText = "> " + std::string(menuItems[i]);
            assets.DrawTextRHDI(modifiedText.c_str(), EntryTextLeft, yPos, EntryFontSize, textColor);
        } else {
            assets.DrawTextRHDI(menuItems[i], EntryTextLeft, yPos, EntryFontSize, Color{136, 136, 136, 255});
        }
    }
    Units::getInstance();
    GameMenu::DrawVersion();
    GameMenu::DrawBottomOvershell();
    DrawOvershell();
    bool isHovering = false;
    for (int i = 0; i < menuItemCount; i++) {
        float yPos = EntryTop + (EntryHeight * (i + 0.3));
        Rectangle itemRect = {
            EntryTextLeft - u.wpct(0.001f),
            yPos,
            u.wpct(0.32f),
            EntryHeight
        };
        if (CheckCollisionPointRec(mousePos, itemRect)) {
            isHovering = true;
            break;
        }
    }
    if (!isHovering) {
        selectedIndex = -1;

    }
}


void SettingsMenu::KeyboardInputCallback(int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        TheMenuManager.SwitchScreen(MAIN_MENU);
    }
}

void SettingsMenu::ControllerInputCallback(int joypadID, GLFWgamepadstate state) {
}

void SettingsMenu::Load() {
#define OPTION(type, value, default) value = TheGameSettings.value;
    SETTINGS_OPTIONS;
#undef OPTION
}
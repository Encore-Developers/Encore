//
// Created by Jaydenz on 27/04/2025.
//

#include "SettingsMenu.h"

#include "SettingsAudioVideo.h"
#include "SettingsController.h"
#include "SettingsGameplay.h"
#include "SettingsKeyboard.h"
#include "../MenuManager.h"
#include "../main/MainMenu.h"
#include "raygui.h"
#include "settings/settings.h"
#include "../uiUnits.h"
#include "assets.h"
#include "util/settings-text.h"
#include "../overshell/OvershellHelper.h"
#include "menus/locale/Locale.h"

static int selectedIndex = 0;

enum OptionsCategories {
    AUDIO_VISUAL,
    GAMEPLAY_SETTINGS,
    CONTROLLER_BINDINGS,
    KEYBOARD_BINDINGS,
    CREDITS
};

void SettingsMenu::Draw() {
    Units &u = Units::getInstance();
    Assets &assets = Assets::getInstance();
    encOS::DrawTopOvershell(0.15f);
    GameMenu::DrawAlbumArtBackground();
    DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), Color { 0, 0, 0, 128 });

    float TextPlacementTB = u.hpct(0.05f);
    float TextPlacementLR = u.wpct(0.05f);
    float EntryFontSize = u.hinpct(0.075f);
    float EntryHeight = u.hinpct(0.075f);
    float EntryTop = u.hpct(0.30f);
    float TextLineTop = u.hpct(0.36f);
    float TextTop = u.hpct(0.31f);
    float EntryTextLeft = u.wpct(0.03f);

    encOS::DrawTopOvershell(0.15f);

    // find a better name for this than Main Menu
    GameMenu::lDrawText(assets.rubik, "settings.header.categorySel",
                         {u.LeftSide, u.hpct(0.027f)}, u.hinpct(0.042f), LIGHTGRAY, LEFT);
    GameMenu::lDrawText(
        assets.redHatDisplayBlack,
        "settings.header.main",
        {u.LeftSide, TextPlacementTB},
        u.hinpct(0.125f),
        WHITE,
        LEFT
    );
    float fontSize = u.hinpct(0.029f);
    GameMenu::lDrawText(assets.josefinSansItalic, "settings.subtitle", {EntryTextLeft, TextTop}, fontSize, Color{136, 136, 136, 255}, LEFT);

    DrawLineEx({EntryTextLeft, TextLineTop}, {EntryTextLeft + u.wpct(0.42f), TextLineTop}, u.winpct(0.001f), WHITE);


    int menuItemCount = 5;
    static float clickFeedbackTimer = 0.0f;
    static int clickedIndex = -1;
    if (clickFeedbackTimer > 0) {
        clickFeedbackTimer -= GetFrameTime();
        if (clickFeedbackTimer <= 0) {
            clickedIndex = -1;
        }
    }
    auto DrawButtonGradient =  [](Rectangle _pos, Color _color) {
        DrawRectangle(0, _pos.y, _pos.x, _pos.height, _color);
        DrawRectangleGradientH(
            _pos.x,
            _pos.y,
            _pos.width,
            _pos.height,
            _color,
            Color { 0, 0, 0, 0 }
            );
    };
    Vector2 mousePos = GetMousePosition();
    for (int i = 0; i < menuItemCount; i++) {
        float yPos = EntryTop + (EntryHeight * (i + 1));
        Rectangle itemRect = {
            EntryTextLeft,
            yPos,
            u.wpct(0.32f),
            EntryHeight
        };
        if (CheckCollisionPointRec(mousePos, itemRect)) {
            selectedIndex = i;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !isOSOpen()) {
                clickFeedbackTimer = 0.2f;
                clickedIndex = i;
                switch (i) {
                case AUDIO_VISUAL:
                    TheMenuManager.CreateAndSwitchMenu<Encore::SettingsAudioVideo>();
                    break;
                case GAMEPLAY_SETTINGS:
                    TheMenuManager.CreateAndSwitchMenu<Encore::SettingsGameplay>();
                    break;
                case CONTROLLER_BINDINGS:
                    TheMenuManager.CreateAndSwitchMenu<SettingsController>();
                    break;
                case KEYBOARD_BINDINGS:
                    TheMenuManager.CreateAndSwitchMenu<SettingsKeyboard>();
                    break;
                case CREDITS:
                    break;
                }
                printf("Clicked: %s\n", menuItems[i].c_str());
            }
        }
        if (i == selectedIndex) {
            Color textColor = (i == clickedIndex) ? GRAY : WHITE;
            DrawButtonGradient(itemRect, ColorBrightness(ColorContrast(Color { 255, 0, 255, 128 }, -0.125f), -0.25f));
            GameMenu::lDrawText(ASSET(redHatDisplayBlack), menuItems[i], {itemRect.x, itemRect.y}, EntryFontSize, textColor, LEFT);
        } else {
            GameMenu::lDrawText(ASSET(redHatDisplayBlack), menuItems[i], {itemRect.x, itemRect.y}, EntryFontSize, Color{136, 136, 136, 255}, LEFT);
        }
    }

    Units::getInstance();
    GameMenu::DrawVersion();
    GameMenu::DrawBottomOvershell();
    buttReg.DrawPrompts(isOSOpen());
    DrawOvershell();
}

void SettingsMenu::KeyboardInputCallback(int key, int scancode, int action, int mods) {
}

void SettingsMenu::ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event) {
    buttReg.HandleInput(event);
}

void SettingsMenu::Load() {
    buttReg.buttMap.clear();
    NEWBUTTONACTION2(buttReg, STRUM_UP, "UP", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        selectedIndex -= 1;
        if (selectedIndex < 0) selectedIndex = 0;
        if (selectedIndex >= menuItems.size()) selectedIndex = menuItems.size() - 1;
    }, false)
    NEWBUTTONACTION2(buttReg, STRUM_DOWN, "DOWN", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        selectedIndex += 1;
        if (selectedIndex < 0) selectedIndex = 0;
        if (selectedIndex >= menuItems.size()) selectedIndex = menuItems.size() - 1;
    }, false)
    NEWBUTTONACTION2(buttReg, LANE_1, "generic.select", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        switch (selectedIndex) {
        case AUDIO_VISUAL:
            TheMenuManager.CreateAndSwitchMenu<Encore::SettingsAudioVideo>();
            break;
        case GAMEPLAY_SETTINGS:
            TheMenuManager.CreateAndSwitchMenu<Encore::SettingsGameplay>();
            break;
        case CONTROLLER_BINDINGS:
            TheMenuManager.CreateAndSwitchMenu<SettingsController>();
            break;
        case KEYBOARD_BINDINGS:
            TheMenuManager.CreateAndSwitchMenu<SettingsKeyboard>();
            break;
        case CREDITS:
            break;
        }
    })
    NEWBUTTONACTION2(buttReg, LANE_2, "settings.prompt.exit", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        TheMenuManager.CreateAndSwitchMenu<MainMenu>();
    })
}
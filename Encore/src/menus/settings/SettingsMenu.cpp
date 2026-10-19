//
// Created by Jaydenz on 27/04/2025.
//

#include "SettingsMenu.h"

#include "SettingsAudioVideo.h"
#include "SettingsController.h"
#include "SettingsGameplay.h"
#include "SettingsKeyboard.h"
#include "../MenuManager.h"
#include "menus/main/MainMenu.h"
#include "raygui.h"
#include "settings/settings.h"
#include "../util/uiUnits.h"
#include "assets.h"
#include "SettingsCredits.h"
#include "util/settings-text.h"
#include "../overshell/OvershellHelper.h"
#include "menus/util/GameMenu.h"
#include "menus/util/TextDisplay.h"
#include "menus/util/locale/Locale.h"

static int selectedIndex = 0;

enum OptionsCategories {
    AUDIO_VISUAL,
    GAMEPLAY_SETTINGS,
    CREDITS
};

void SettingsMenu::Draw() {
    Assets &assets = Assets::getInstance();
    GameMenu::DrawAlbumArtBackground();
    DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), Color { 0, 0, 0, 128 });

    float TextPlacementTB = Units::hpct(0.05f);
    float TextPlacementLR = Units::wpct(0.05f);
    float EntryFontSize = Units::hinpct(0.075f);
    float EntryHeight = Units::hinpct(0.075f);
    float EntryTop = Units::hpct(0.30f);
    float TextLineTop = Units::hpct(0.36f);
    float TextTop = Units::hpct(0.31f);
    float EntryTextLeft = Units::wpct(0.03f);

    GameMenu::DrawTopOvershell(0.15f);

    // find a better name for this than Main Menu
    Encore::Text::lDrawText(assets.rubik, "settings.header.categorySel",
                         {Units::LeftSide, Units::hpct(0.027f)}, Units::hinpct(0.042f), LIGHTGRAY, LEFT);
    Encore::Text::lDrawText(
        assets.redHatDisplayBlack,
        "settings.header.main",
        {Units::LeftSide, TextPlacementTB},
        Units::hinpct(0.125f),
        WHITE,
        LEFT
    );
    float fontSize = Units::hinpct(0.029f);
    Encore::Text::lDrawText(assets.josefinSansItalic, "settings.subtitle", {EntryTextLeft, TextTop}, fontSize, Color{136, 136, 136, 255}, LEFT);

    DrawLineEx({EntryTextLeft, TextLineTop}, {EntryTextLeft + Units::wpct(0.42f), TextLineTop}, Units::winpct(0.001f), WHITE);


    int menuItemCount = 3;
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
            Units::wpct(0.32f),
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
                case CREDITS:
                    TheMenuManager.CreateAndSwitchMenu<SettingsCredits>();
                    break;
                }
                Encore::Log::Trace("Clicked: {}", menuItems[i]);
            }
        }
        if (i == selectedIndex) {
            Color textColor = (i == clickedIndex) ? GRAY : WHITE;
            DrawButtonGradient(itemRect, ColorBrightness(ColorContrast(Color { 255, 0, 255, 128 }, -0.125f), -0.25f));
            Encore::Text::lDrawText(ASSET(redHatDisplayBlack), menuItems[i], {itemRect.x, itemRect.y}, EntryFontSize, textColor, LEFT);
        } else {
            Encore::Text::lDrawText(ASSET(redHatDisplayBlack), menuItems[i], {itemRect.x, itemRect.y}, EntryFontSize, Color{136, 136, 136, 255}, LEFT);
        }
    }

    GameMenu::DrawTopBarText();
    GameMenu::DrawBottomOvershell();
    buttReg.DrawPrompts(isOSOpen());
    DrawOvershell();
}

void SettingsMenu::KeyboardInputCallback(SDL_KeyboardEvent* event) {
}

void SettingsMenu::ControllerInputCallback(Encore::ControllerEvent event) {
    buttReg.HandleInput(event);
}

void SettingsMenu::Load() {
    buttReg.buttMap.clear();
    NEWBUTTONACTION2(buttReg, STRUM_UP, "UP", {
        if (_action != Encore::Action::PRESS) return;
        selectedIndex -= 1;
        if (selectedIndex < 0) selectedIndex = 0;
        if (selectedIndex >= menuItems.size()) selectedIndex = menuItems.size() - 1;
    }, false)
    NEWBUTTONACTION2(buttReg, STRUM_DOWN, "DOWN", {
        if (_action != Encore::Action::PRESS) return;
        selectedIndex += 1;
        if (selectedIndex < 0) selectedIndex = 0;
        if (selectedIndex >= menuItems.size()) selectedIndex = menuItems.size() - 1;
    }, false)
    NEWBUTTONACTION2(buttReg, LANE_1, "generic.select", {
        if (_action != Encore::Action::PRESS) return;
        switch (selectedIndex) {
        case AUDIO_VISUAL:
            TheMenuManager.CreateAndSwitchMenu<Encore::SettingsAudioVideo>();
            break;
        case GAMEPLAY_SETTINGS:
            TheMenuManager.CreateAndSwitchMenu<Encore::SettingsGameplay>();
            break;
        case CREDITS:
            break;
        }
    })
    NEWBUTTONACTION2(buttReg, LANE_2, "settings.prompt.exit", {
        if (_action != Encore::Action::PRESS) return;
        TheMenuManager.CreateAndSwitchMenu<MainMenu>();
    })
}
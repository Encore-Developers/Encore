//
// Created by marie on 02/05/2024.
//


#include <functional>
#include "game/menu.h"
#include "raylib.h"
#include "raygui.h"
#include "song/songlist.h"
#include "game/lerp.h"
#include "game/settings.h"
#include "game/assets.h"


void Menu::renderStars(Player player, float xPos, float yPos) {

    Assets assets;
    int starsval = player.stars(player.score,player.diff);
    for (int i = 0; i < starsval; i++) {
        DrawTextureEx(player.goldStars? assets.goldStar : assets.star, {(xPos+(i*40)-100),yPos},0,0.15f,WHITE);
    }
};

void Menu::loadMenu(SongList songList, GLFWgamepadstatefun gamepadStateCallbackSetControls, Assets assets) {
    Lerp lerpCtrl;
    Settings settings;
        lerpCtrl.createLerp("MENU_LOGO", EaseOutCubic, 1.5f);
        DrawTextureEx(assets.encoreWhiteLogo, {(float) GetScreenWidth() / 2 - assets.encoreWhiteLogo.width / 4,
                                               (float) lerpCtrl.getState("MENU_LOGO").value *
                                               ((float) GetScreenHeight() / 5 - assets.encoreWhiteLogo.height / 4)}, 0,
                      0.5, WHITE);

        if (GuiButton({((float) GetScreenWidth() / 2) - 100, ((float) GetScreenHeight() / 2) - 120, 200, 60}, "Play")) {

            for (Song &songi: songList.songs) {
                songi.titleScrollTime = GetTime();
                songi.titleTextWidth = assets.MeasureTextRubik(songi.title.c_str(), 24);
                songi.artistScrollTime = GetTime();
                songi.artistTextWidth = assets.MeasureTextRubik(songi.artist.c_str(), 20);
            }
            Menu::SwitchScreen(SONG_SELECT);
        }
        if (GuiButton({((float) GetScreenWidth() / 2) - 100, ((float) GetScreenHeight() / 2) - 30, 200, 60},
                      "Options")) {
            glfwSetGamepadStateCallback(gamepadStateCallbackSetControls);
            Menu::SwitchScreen(SETTINGS);
        }
        if (GuiButton({((float) GetScreenWidth() / 2) - 100, ((float) GetScreenHeight() / 2) + 60, 200, 60}, "Quit")) {
            exit(0);
        }
        if (GuiButton({(float) GetScreenWidth() - 60, (float) GetScreenHeight() - 60, 60, 60}, "")) {
            OpenURL("https://github.com/Encore-Developers/Encore-Raylib");
        }

        if (GuiButton({(float) GetScreenWidth() - 120, (float) GetScreenHeight() - 60, 60, 60}, "")) {
            OpenURL("https://discord.gg/GhkgVUAC9v");
        }
        if (GuiButton({(float) GetScreenWidth() - 180, (float) GetScreenHeight() - 120, 180, 60}, "Rescan Songs")) {
            songsLoaded = false;
        }
        DrawTextureEx(assets.github, {(float) GetScreenWidth() - 54, (float) GetScreenHeight() - 54}, 0, 0.2, WHITE);
        DrawTextureEx(assets.discord, {(float) GetScreenWidth() - 113, (float) GetScreenHeight() - 48}, 0, 0.075,
                      WHITE);
}

void Menu::showResults(Player player) {
    float RightBorder = ((float)GetScreenWidth()/2)+((float)GetScreenHeight()/1.16f);
    float RightSide = RightBorder >= (float)GetScreenWidth() ? (float)GetScreenWidth() : RightBorder;


    float LeftBorder = ((float)GetScreenWidth()/2)-((float)GetScreenHeight()/1.16f);
    float LeftSide = LeftBorder <= 0 ? 0 : LeftBorder;

    float PlayerWidth = ((float)GetScreenWidth()/2)-((float)GetScreenHeight()/1.40f);

    DrawRectangle(LeftSide-4, 0, PlayerWidth+8, (float)GetScreenHeight(), WHITE);
    DrawRectangle(LeftSide, 0, PlayerWidth, (float)GetScreenHeight(), GetColor(0x181827FF));
    // DrawLine(LeftSide, 0, LeftSide, (float)GetScreenHeight(), WHITE);
    renderStars(player, 0,0);
}

void Menu::SwitchScreen(Screens screen){
    Lerp lerpCtrl;
    currentScreen = screen;
    switch (screen) {
        case MENU:
            // reset lerps
            lerpCtrl.removeLerp("MENU_LOGO");
            break;
        case SONG_SELECT:
            break;
        case INSTRUMENT_SELECT:
            break;
        case DIFFICULTY_SELECT:
            break;
        case GAMEPLAY:
            break;
        case RESULTS:
            break;
        case SETTINGS:
            break;
    }
}
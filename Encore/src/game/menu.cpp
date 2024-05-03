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

void Menu::showResults(Player player, Assets assets) {

    Song songToBeJudged = player.songToBeJudged;
    float RightBorder = ((float)GetScreenWidth()/2)+((float)GetScreenHeight()/1.20f);
    float RightSide = RightBorder >= (float)GetScreenWidth() ? (float)GetScreenWidth() : RightBorder;


    float LeftBorder = ((float)GetScreenWidth()/2)-((float)GetScreenHeight()/1.20f);
    float LeftSide = LeftBorder <= 0 ? 0 : LeftBorder;

    float ResultsWidth = (RightSide - LeftSide);
    float PlayerWidth = (ResultsWidth*0.2275f);

    float InnerMargins = (ResultsWidth*0.03f);

    float Player2 = LeftSide + PlayerWidth + InnerMargins;

    float Player4 = RightSide - PlayerWidth;
    float Player3 = Player4 - PlayerWidth - InnerMargins;

    float middle = (PlayerWidth/2) + LeftSide;


    DrawRectangle(LeftSide-4, 0, PlayerWidth+8, (float)GetScreenHeight(), WHITE);
    DrawRectangle(LeftSide, 0, PlayerWidth, (float)GetScreenHeight(), GetColor(0x181827FF));
    // renderStars(player, 0,0);

    //assets.DrawTextRHDI("Results", 70,7, WHITE);
    assets.DrawTextRubik(songToBeJudged.artist.c_str(), middle - (assets.MeasureTextRubik(songToBeJudged.artist.c_str(), 24) / 2), 48, 24, WHITE);
    // if (player.FC) {
    //    assets.DrawTextRubik("Flawless!", middle- (assets.MeasureTextRubik("Flawless!", 24) / 2), 7, 24, GOLD);
    // }
    float scorePos = middle - MeasureTextEx(assets.redHatDisplayItalic, scoreCommaFormatter(player.score).c_str(), 64, 1).x /2;
    DrawTextEx(
            assets.redHatDisplayItalic,
            scoreCommaFormatter(player.score).c_str(),
            {
                scorePos,
                (float)GetScreenHeight()/2},
            64,
            1,
            Color{
                107,
                161,
                222,
                255});

    assets.DrawTextRHDI(scoreCommaFormatter(player.score).c_str(), middle - assets.MeasureTextRHDI(scoreCommaFormatter(player.score).c_str())/2, 120, Color{107, 161, 222,255});
    assets.DrawTextRubik(TextFormat("Perfect Notes : %01i/%02i", player.perfectHit, songToBeJudged.parts[player.instrument]->charts[player.diff].notes.size()), (middle - assets.MeasureTextRubik(TextFormat("Perfect Notes: %01i/%02i", player.perfectHit, songToBeJudged.parts[player.instrument]->charts[player.diff].notes.size()), 24) / 2), 192, 24, WHITE);
    assets.DrawTextRubik(TextFormat("Good Notes : %01i/%02i", player.notesHit - player.perfectHit, songToBeJudged.parts[player.instrument]->charts[player.diff].notes.size()), (middle - assets.MeasureTextRubik(TextFormat("Good Notes: %01i/%02i", player.notesHit - player.perfectHit, songToBeJudged.parts[player.instrument]->charts[player.diff].notes.size()), 24) / 2), 224, 24, WHITE);
    assets.DrawTextRubik(TextFormat("Missed Notes: %01i/%02i", player.notesMissed, songToBeJudged.parts[player.instrument]->charts[player.diff].notes.size()), (GetScreenWidth() / 2 - assets.MeasureTextRubik(TextFormat("Missed Notes: %01i/%02i", player.notesMissed, songToBeJudged.parts[player.instrument]->charts[player.diff].notes.size()), 24) / 2), 256, 24, WHITE);
    assets.DrawTextRubik(TextFormat("Strikes: %01i", player.playerOverhits), (middle - assets.MeasureTextRubik(TextFormat("Strikes: %01i", player.playerOverhits), 24) / 2), 288, 24, WHITE);
    assets.DrawTextRubik(TextFormat("Longest Streak: %01i", player.maxCombo), (middle - assets.MeasureTextRubik(TextFormat("Longest Streak: %01i", player.maxCombo), 24) / 2), 320, 24, WHITE);
    renderStars(player, middle, 90);

    //DrawRectangle(Player2-4, 0, PlayerWidth+8, (float)GetScreenHeight(), WHITE);
    //DrawRectangle(Player2, 0, PlayerWidth, (float)GetScreenHeight(), GetColor(0x181827FF));

    //DrawRectangle(Player3-4, 0, PlayerWidth+8, (float)GetScreenHeight(), WHITE);
    //DrawRectangle(Player3, 0, PlayerWidth, (float)GetScreenHeight(), GetColor(0x181827FF));

    //DrawRectangle(Player4-4, 0, PlayerWidth+8, (float)GetScreenHeight(), WHITE);
    //DrawRectangle(Player4, 0, PlayerWidth, (float)GetScreenHeight(), GetColor(0x181827FF));
    // DrawLine(LeftSide, 0, LeftSide, (float)GetScreenHeight(), WHITE);

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
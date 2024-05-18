//
// Created by marie on 02/05/2024.
//


#include <functional>
#include <random>
#include "game/menus/gameMenu.h"
#include "raylib.h"
#include "raygui.h"
#include "song/songlist.h"
#include "game/lerp.h"
#include "game/settings.h"
#include "game/assets.h"
#include "game/menus/uiUnits.h"
#include "raymath.h"

#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH
#endif

#ifndef ENCORE_VERSION
#define ENCORE_VERSION
#endif

const float Width = (float)GetScreenWidth();
const float Height = (float)GetScreenHeight();


std::string menuCommitHash = GIT_COMMIT_HASH;
std::string menuVersion = ENCORE_VERSION;
Settings& settings = Settings::getInstance();
SongList &songListMenu = SongList::getInstance();
Units u;
std::vector<std::string> songPartsList{ "Drums","Bass","Guitar","Vocals"};
std::vector<std::string> diffList{ "Easy","Medium","Hard","Expert" };
void Menu::DrawTopOvershell(float TopOvershell) {
    DrawRectangle(0,0,(int)GetScreenWidth(), u.hpct(TopOvershell)+6,WHITE);
    DrawRectangle(0,0,(int)GetScreenWidth(), u.hpct(TopOvershell),ColorBrightness(GetColor(0x181827FF),-0.25f));
}

void Menu::DrawBottomOvershell() {
    float BottomOvershell = GetScreenHeight() - u.hpct(0.15f);
    DrawRectangle(0,BottomOvershell-6,(float)(GetScreenWidth()), (float)GetScreenHeight(),WHITE);
    DrawRectangle(0,BottomOvershell,(float)(GetScreenWidth()), (float)GetScreenHeight(),ColorBrightness(GetColor(0x181827FF),-0.5f));
}

void Menu::DrawBottomBottomOvershell() {
    float BottomBottomOvershell = GetScreenHeight() - u.hpct(0.1f);
    DrawRectangle(0,BottomBottomOvershell-6,(float)(GetScreenWidth()), (float)GetScreenHeight(),WHITE);
    DrawRectangle(0,BottomBottomOvershell,(float)(GetScreenWidth()), (float)GetScreenHeight(),ColorBrightness(GetColor(0x181827FF),-0.5f));
}

// should be reduced to just PlayerSongStats (instead of Player) eventually
void Menu::renderPlayerResults(Player player, Song song, Assets assets) {

    float cardPos = u.LeftSide + (u.winpct(0.26f) * (float)player.playerNum);


    DrawRectangle(cardPos-6, u.hpct(0.2f), u.winpct(0.22f)+12, u.hpct(0.85f), WHITE);
    DrawRectangle(cardPos, u.hpct(0.2f), u.winpct(0.22f), u.hpct(0.85f), GetColor(0x181827FF));

    DrawRectangleGradientV(cardPos,u.hpct(0.2f), u.winpct(0.22f), u.hinpct(0.2f), ColorBrightness(player.accentColor, -0.5f), GetColor(0x181827FF));

    bool rendAsFC = player.FC && !player.quit;

    if (player.quit) {
        DrawRectangleGradientV(cardPos,u.hpct(0.2f)+ u.hinpct(0.2f), u.winpct(0.22f), u.hinpct(0.63f), GetColor(0x181827FF), ColorBrightness(RED, -0.5f));
    }
    if (rendAsFC) {
        DrawRectangleGradientV(cardPos,u.hpct(0.2f)+ u.hinpct(0.2f), u.winpct(0.22f), u.hinpct(0.63f), GetColor(0x181827FF),
                               ColorContrast(ColorBrightness(GOLD, -0.5f), -0.25f));
    }
    if (player.perfectHit==song.parts[player.instrument]->charts[player.diff].notes.size() && rendAsFC) {
        DrawRectangleGradientV(cardPos,u.hpct(0.2f)+ u.hinpct(0.2f), u.winpct(0.22f), u.hinpct(0.63f), GetColor(0x181827FF),
                               ColorBrightness(WHITE, -0.5f));
    }

    DrawLine(cardPos,u.hpct(0.2f) + u.hinpct(0.2f), cardPos + u.winpct(0.22f),u.hpct(0.2f) + u.hinpct(0.2f),WHITE);
    DrawLine(cardPos,u.hpct(0.2f) + u.hinpct(0.4f), cardPos + u.winpct(0.22f),u.hpct(0.2f) + u.hinpct(0.4f),WHITE);

    float scorePos = (cardPos + u.winpct(0.11f)) - (MeasureTextEx(assets.redHatDisplayItalic, scoreCommaFormatter(player.score).c_str(), u.hinpct(0.07f), 1).x /2);
    float Percent = ((float)player.notesHit/song.parts[player.instrument]->charts[player.diff].notes.size()) * 100;

    DrawTextEx(
            assets.redHatDisplayItalic,
            scoreCommaFormatter(player.score).c_str(),
            {
                    scorePos,
                    (float)GetScreenHeight()/2},
            u.hinpct(0.065f),
            0,
            GetColor(0x00adffFF));

    renderStars(player,  (cardPos + u.winpct(0.11f)), (float)GetScreenHeight()/2 - u.hinpct(0.06f), assets, u.hinpct(0.055f),false);


    if (rendAsFC) {
        DrawTextEx(assets.redHatDisplayItalicLarge, TextFormat("%3.0f%%", Percent), {(cardPos + u.winpct(0.113f)) - (MeasureTextEx(assets.redHatDisplayItalicLarge, TextFormat("%3.0f", Percent), u.hinpct(0.1f),0).x/1.5f),u.hpct(0.243f)},u.hinpct(0.1f),0,
                   ColorBrightness(GOLD,-0.5));
        float flawlessFontSize = 0.03f;
        DrawTextEx(
                assets.rubikBoldItalic,
                "Flawless!",
                {
                        (cardPos + u.winpct(0.113f))-(MeasureTextEx(assets.rubikBoldItalic, "Flawless!", u.hinpct(flawlessFontSize), 0.0f).x/2),
                        u.hpct(0.35f)},
                u.hinpct(flawlessFontSize),
                0.0f,
                WHITE);
    } if (player.quit) {
        float flawlessFontSize = 0.05f;
        DrawTextEx(
                assets.rubikBoldItalic,
                "Quit",
                {
                        (cardPos + u.winpct(0.11f))-(MeasureTextEx(assets.rubikBoldItalic, "Quit", u.hinpct(flawlessFontSize), 0.0f).x/2),
                        u.hpct(0.335f)},
                u.hinpct(flawlessFontSize),
                0.0f,
                RED);
    }
    DrawTextEx(assets.redHatDisplayItalicLarge, TextFormat("%3.0f%%", Percent), {(cardPos + u.winpct(0.11f)) - (MeasureTextEx(assets.redHatDisplayItalicLarge, rendAsFC ? TextFormat("%3.0f", Percent) : TextFormat("%3.0f%%", Percent), u.hinpct(0.1f),0).x/(rendAsFC ? 1.5f : 2.0f)),u.hpct(0.24f)},u.hinpct(0.1f),0, rendAsFC ? YELLOW : WHITE);

    float statsHeight = u.hpct(0.2f) + u.hinpct(0.415f);
    float statsLeft = cardPos + u.winpct(0.01f);
    float statsRight = cardPos + u.winpct(0.21f);

    DrawTextEx(assets.rubik, "Perfects:", {statsLeft, statsHeight}, u.hinpct(0.03f),0,WHITE);
    DrawTextEx(assets.rubik, "Goods:", {statsLeft, statsHeight+u.hinpct(0.035f)}, u.hinpct(0.03f),0,WHITE);
    DrawTextEx(assets.rubik, "Missed:", {statsLeft, statsHeight+u.hinpct(0.07f)}, u.hinpct(0.03f),0,WHITE);
    DrawTextEx(assets.rubik, "Strikes:", {statsLeft, statsHeight+u.hinpct(0.105f)}, u.hinpct(0.03f),0,WHITE);
    DrawTextEx(assets.rubik, "Max Streak:", {statsLeft, statsHeight+u.hinpct(0.14f)}, u.hinpct(0.03f),0,WHITE);

    DrawTextEx(assets.rubikBold, TextFormat("%s %s", diffList[player.diff].c_str(), songPartsList[player.instrument].c_str()), {cardPos + u.winpct(0.11f) -
                                                                                                                                          (MeasureTextEx(assets.rubikBold, TextFormat("%s %s", diffList[player.diff].c_str(), songPartsList[player.instrument].c_str()), u.hinpct(0.04f),0).x/2), statsHeight+u.hinpct(0.175f)}, u.hinpct(0.04f),0,WHITE);


    DrawTextEx(assets.rubik, TextFormat("%01i", player.perfectHit), {statsRight - MeasureTextEx(assets.rubik, TextFormat("%01i", player.perfectHit), u.hinpct(0.03f), 0).x, statsHeight}, u.hinpct(0.03f),0,WHITE);
    DrawTextEx(assets.rubik, TextFormat("%01i", player.notesHit-player.perfectHit), {statsRight - MeasureTextEx(assets.rubik, TextFormat("%01i", player.notesHit-player.perfectHit), u.hinpct(0.03f), 0).x, statsHeight+u.hinpct(0.035f)}, u.hinpct(0.03f),0,WHITE);
    DrawTextEx(assets.rubik, TextFormat("%01i", player.notesMissed), {statsRight - MeasureTextEx(assets.rubik, TextFormat("%01i", player.notesMissed), u.hinpct(0.03f), 0).x, statsHeight+u.hinpct(0.07f)}, u.hinpct(0.03f),0,WHITE);
    DrawTextEx(assets.rubik, TextFormat("%01i", player.playerOverhits), {statsRight - MeasureTextEx(assets.rubik, TextFormat("%01i", player.playerOverhits), u.hinpct(0.03f), 0).x, statsHeight+u.hinpct(0.105f)}, u.hinpct(0.03f),0,WHITE);
    DrawTextEx(assets.rubik, TextFormat("%01i", player.maxCombo), {statsRight - MeasureTextEx(assets.rubik, TextFormat("%01i", player.maxCombo), u.hinpct(0.03f), 0).x, statsHeight+u.hinpct(0.14f)}, u.hinpct(0.03f),0,WHITE);
};

// todo: replace player with band stats
void Menu::renderStars(Player player, float xPos, float yPos, Assets assets, float scale, bool left) {
    int starsval = player.stars(player.songToBeJudged.parts[player.instrument]->charts[player.diff].baseScore,player.diff);
    float starPercent = (float)player.score/(float)player.songToBeJudged.parts[player.instrument]->charts[player.diff].baseScore;

    float starX = left ? 0 : scale*2.5f;
    for (int i = 0; i < 5; i++) {
        DrawTexturePro(assets.emptyStar, {0,0,(float)assets.emptyStar.width,(float)assets.emptyStar.height}, {(xPos+(i*scale)-starX),yPos, scale, scale},{0,0},0,WHITE);
    }
    for (int i = 0; i < starsval ; i++) {
        DrawTexturePro(player.goldStars?assets.goldStar:assets.star, {0,0,(float)assets.emptyStar.width,(float)assets.emptyStar.height}, {(xPos+(i*scale)-starX),yPos, scale, scale}, {0,0},0, WHITE);
    }
};

// todo: text box rendering for splashes, cleanup of buttons
void Menu::loadMenu(GLFWgamepadstatefun gamepadStateCallbackSetControls, Assets assets) {

    std::filesystem::path directory = GetPrevDirectoryPath(GetApplicationDirectory());

    std::ifstream splashes;
    splashes.open((directory / "Assets/ui/splashes.txt"));

    static std::string result;
    std::string line;
    if (!stringChosen) {
        std::random_device seed;
        std::mt19937 prng(seed());
        for (std::size_t n = 0; std::getline(splashes, line); n++) {
            std::uniform_int_distribution<> dist(0, n);
            if (dist(prng) < 1)
                result = line;
        }
        stringChosen = true;
    }
    if (!albumArtLoaded) {
        AlbumArtBackground = assets.highwayTexture;;
        if (!songChosen && songsLoaded) {
            std::random_device weez;
            std::mt19937 jonas(weez());
            std::uniform_int_distribution<int> nameis(0, (int) songListMenu.songs.size());

            static int my = nameis(jonas);

            Song art = songListMenu.songs[my];
            art.LoadAlbumArt(art.albumArtPath);

            AlbumArtBackground = art.albumArtBlur;

            songChosen = true;


        } else {
            AlbumArtBackground = assets.highwayTexture;
        };
        albumArtLoaded = true;
    };
    BeginShaderMode(assets.bgShader);
    DrawTexturePro(AlbumArtBackground, Rectangle{0, 0, (float) AlbumArtBackground.width,
                                                 (float) AlbumArtBackground.width},
                   Rectangle{(float) GetScreenWidth() / 2, -((float) GetScreenHeight() * 2),
                             (float) GetScreenWidth() * 2, (float) GetScreenWidth() * 2}, {0, 0}, 45,
                   WHITE);
    EndShaderMode();

    Vector2 StringBox = {u.RightSide - MeasureTextEx(assets.josefinSansItalic, result.c_str(), 32, 1).x - u.winpct(0.01f),  u.hpct(0.2f)/2 - 16};
    DrawTopOvershell(0.2f);
    DrawBottomOvershell();
    DrawBottomBottomOvershell();
    menuCommitHash.erase(7);
    DrawTextEx(assets.josefinSansItalic, TextFormat("%s-%s",menuVersion.c_str() , menuCommitHash.c_str()), {u.wpct(0), u.hpct(0)}, u.hinpct(0.025f), 0, WHITE);
    DrawTextEx(assets.josefinSansItalic, result.c_str(), StringBox, 32, 0, WHITE);
    DrawTextureEx(assets.encoreWhiteLogo, { u.LeftSide + u.winpct(0.01f), u.hpct(0.19f)-(assets.encoreWhiteLogo.height*0.5f)}, 0, 0.5, WHITE);


        if (GuiButton({((float) GetScreenWidth() / 2) - 100, ((float) GetScreenHeight() / 2) - 120, 200, 60}, "Play")) {

            for (Song &songi: songListMenu.songs) {
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
        if (GuiButton({(float) GetScreenWidth() - 60, (float) GetScreenHeight() - u.hpct(0.15f) - 60, 60, 60}, "")) {
            OpenURL("https://github.com/Encore-Developers/Encore-Raylib");
        }


        if (GuiButton({(float) GetScreenWidth() - 120, (float) GetScreenHeight() - u.hpct(0.15f) - 60, 60, 60}, "")) {
            OpenURL("https://discord.gg/GhkgVUAC9v");
        }
    if (GuiButton({(float) GetScreenWidth() - 180, (float) GetScreenHeight() - u.hpct(0.15f) - 60, 60, 60}, "")) {
        stringChosen = false;
    }
        if (GuiButton({(float) GetScreenWidth() - 180, (float) GetScreenHeight() - u.hpct(0.15f) - 120, 180, 60}, "Rescan Songs")) {
            songsLoaded = false;
            songListMenu.ScanSongs(settings.songPaths);
        }
        DrawTextureEx(assets.github, {(float) GetScreenWidth() - 54, (float) GetScreenHeight() - 54 - u.hpct(0.15f) }, 0, 0.2, WHITE);
        DrawTextureEx(assets.discord, {(float) GetScreenWidth() - 113, (float) GetScreenHeight() - 48 - u.hpct(0.15f) }, 0, 0.075,
                      WHITE);
}

void Menu::showResults(Player &player, Assets assets) {


    Song songToBeJudged = player.songToBeJudged;
    BeginShaderMode(assets.bgShader);
    DrawTexturePro(songToBeJudged.albumArtBlur, Rectangle{0, 0, (float) songToBeJudged.albumArt.width,
                                                          (float) songToBeJudged.albumArt.width},
                   Rectangle{(float) GetScreenWidth() / 2, -((float) GetScreenHeight() * 2),
                             (float) GetScreenWidth() * 2, (float) GetScreenWidth() * 2}, {0, 0}, 45,
                   WHITE);
    EndShaderMode();

    for (int i = 0; i < 4; i++) {
        renderPlayerResults(player, songToBeJudged, assets);
    }

    DrawTopOvershell(0.2f);
    DrawBottomOvershell();
    DrawBottomBottomOvershell();

    DrawTextEx(assets.josefinSansItalic, TextFormat("%s-%s",menuVersion.c_str() , menuCommitHash.c_str()), {u.wpct(0), u.hpct(0)}, u.hinpct(0.025f), 0, WHITE);

    float songNamePos = (float)GetScreenWidth()/2 - MeasureTextEx(assets.redHatDisplayBlack,player.songToBeJudged.title.c_str(), u.hinpct(0.09f), 0).x/2;
    float bigScorePos = (float)GetScreenWidth()/2 - u.winpct(0.04f) - MeasureTextEx(assets.redHatDisplayItalicLarge,scoreCommaFormatter(player.score).c_str(), u.hinpct(0.08f), 0).x;
    float bigStarPos = (float)GetScreenWidth()/2 + u.winpct(0.005f);


    DrawTextEx(assets.redHatDisplayBlack, player.songToBeJudged.title.c_str(), {songNamePos,u.hpct(0.01f)},u.hinpct(0.09f),0,WHITE);
    DrawTextEx(assets.redHatDisplayItalicLarge, scoreCommaFormatter(player.score).c_str(), {bigScorePos,u.hpct(0.1f)},u.hinpct(0.08f),0, GetColor(0x00adffFF));
    renderStars(player, bigStarPos, u.hpct(0.1125f), assets, u.hinpct(0.055f),true);
    // assets.DrawTextRHDI(player.songToBeJudged.title.c_str(),songNamePos, 50, WHITE);
}

void Menu::SwitchScreen(Screens screen){
    currentScreen = screen;
    switch (screen) {
        case MENU:
            // reset lerps
            stringChosen = false;
            break;
        case SONG_SELECT:
            break;
        case READY_UP:
            break;
        case GAMEPLAY:
            break;
        case RESULTS:
            break;
        case SETTINGS:
            break;
    }
}
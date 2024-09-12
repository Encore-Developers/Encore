//
// Created by marie on 02/05/2024.
//


#include <functional>
#include <random>
#include <raylib.h>
#include <raygui.h>
#include <raymath.h>

#include "../assets.h"
#include "../song/audio.h"
#include "../old/lerp.h"
#include "../menus/gameMenu.h"

#include <variant>

#include "../menus/menu.h"
#include "overshellRenderer.h"
#include "uiUnits.h"
#include "../settings.h"
#include "../song/songlist.h"


#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH
#endif

#ifndef ENCORE_VERSION
#define ENCORE_VERSION
#endif

const float Width = (float)GetScreenWidth();
const float Height = (float)GetScreenHeight();

bool randomSongChosen = false;

std::string menuCommitHash = GIT_COMMIT_HASH;
std::string menuVersion = ENCORE_VERSION;
std::string gitBranch = GIT_BRANCH;
Assets &menuAss = Assets::getInstance();
Settings& settings = Settings::getInstance();
SongList &songListMenu = SongList::getInstance();
Units u = Units::getInstance();

GameMenu TheGameMenu;

std::vector<std::string> songPartsList{ "Drums","Bass","Guitar","Vocals","Classic Drums", "Classic Bass", "Classic Lead", "Classic Vocals", "Keys", "Classic Keys"};
std::vector<std::string> diffList{ "Easy","Medium","Hard","Expert" };

void GameMenu::DrawTopOvershell(float TopOvershell) {
    DrawRectangleGradientV(0,u.hpct(TopOvershell)-2,GetScreenWidth(),u.hinpct(0.025f), Color{0,0,0,128}, Color{0,0,0,0});
    DrawRectangle(0,0,(int)GetScreenWidth(), u.hpct(TopOvershell),WHITE);
    DrawRectangle(0,0,(int)GetScreenWidth(), u.hpct(TopOvershell)-u.hinpct(0.005f),ColorBrightness(GetColor(0x181827FF),-0.25f));
}

void GameMenu::DrawBottomOvershell() {
    float BottomOvershell = GetScreenHeight() - u.hpct(0.15f);
    DrawRectangle(0,BottomOvershell,(float)(GetScreenWidth()), (float)GetScreenHeight(),WHITE);
    DrawRectangle(0,BottomOvershell+u.hinpct(0.005f),(float)(GetScreenWidth()), (float)GetScreenHeight(),ColorBrightness(GetColor(0x181827FF),-0.5f));
}

void GameMenu::DrawBottomBottomOvershell() {
    float BottomBottomOvershell = GetScreenHeight() - u.hpct(0.1f);
    DrawRectangle(0,BottomBottomOvershell,(float)(GetScreenWidth()), (float)GetScreenHeight(),WHITE);
    DrawRectangle(0,BottomBottomOvershell+u.hinpct(0.005f),(float)(GetScreenWidth()), (float)GetScreenHeight(),ColorBrightness(GetColor(0x181827FF),-0.5f));
}

// should be reduced to just PlayerSongStats (instead of Player) eventually
void GameMenu::renderPlayerResults(Player *player, Song song, int playerslot) {

    float cardPos = u.LeftSide + (u.winpct(0.26f) * ((float)playerslot));


    DrawRectangle(cardPos-6, u.hpct(0.2f), u.winpct(0.22f)+12, u.hpct(0.85f), WHITE);
    DrawRectangle(cardPos, u.hpct(0.2f), u.winpct(0.22f), u.hpct(0.85f), GetColor(0x181827FF));

    DrawRectangleGradientV(cardPos,u.hpct(0.2f), u.winpct(0.22f), u.hinpct(0.2f), ColorBrightness(player->AccentColor, -0.5f), GetColor(0x181827FF));

    bool rendAsFC = player->stats->FC && !player->stats->Quit && !player->Bot;
    if (player->Bot) {
        DrawRectangleGradientV(cardPos,u.hpct(0.2f)+ u.hinpct(0.2f), u.winpct(0.22f), u.hinpct(0.63f), GetColor(0x181827FF),
                               ColorContrast(ColorBrightness(SKYBLUE, -0.5f), -0.25f));
    }
    if (player->stats->Quit && !player->Bot) {
        DrawRectangleGradientV(cardPos,u.hpct(0.2f)+ u.hinpct(0.2f), u.winpct(0.22f), u.hinpct(0.63f), GetColor(0x181827FF), ColorBrightness(RED, -0.5f));
    }
    if (rendAsFC && !player->Bot) {
        DrawRectangleGradientV(cardPos,u.hpct(0.2f)+ u.hinpct(0.2f), u.winpct(0.22f), u.hinpct(0.63f), GetColor(0x181827FF),
                               ColorContrast(ColorBrightness(GOLD, -0.5f), -0.25f));
    }
    if (player->stats->PerfectHit==player->stats->Notes && rendAsFC && !player->Bot) {
        DrawRectangleGradientV(cardPos,u.hpct(0.2f)+ u.hinpct(0.2f), u.winpct(0.22f), u.hinpct(0.63f), GetColor(0x181827FF),
                               ColorBrightness(WHITE, -0.5f));
    }

    DrawLine(cardPos,u.hpct(0.2f) + u.hinpct(0.2f), cardPos + u.winpct(0.22f),u.hpct(0.2f) + u.hinpct(0.2f),WHITE);
    DrawLine(cardPos,u.hpct(0.2f) + u.hinpct(0.4f), cardPos + u.winpct(0.22f),u.hpct(0.2f) + u.hinpct(0.4f),WHITE);

    float scorePos = (cardPos + u.winpct(0.11f)) - (MeasureTextEx(menuAss.redHatDisplayItalic, scoreCommaFormatter(player->stats->Score).c_str(), u.hinpct(0.065f), 0).x /2);
    float Percent = floorf(((float)player->stats->NotesHit/ (float)player->stats->Notes) * 100.0f);

    DrawTextEx(
            menuAss.redHatDisplayItalic,
            scoreCommaFormatter(player->stats->Score).c_str(),
            {
                    scorePos,
                    (float)GetScreenHeight()/2},
            u.hinpct(0.065f),
            0,
            GetColor(0x00adffFF));

    renderStarsP(player->stats,  (cardPos + u.winpct(0.11f)), (float)GetScreenHeight()/2 - u.hinpct(0.06f), u.hinpct(0.055f),false);


    if (rendAsFC) {
        DrawTextEx(menuAss.redHatDisplayItalicLarge, TextFormat("%3.0f%%", Percent), {(cardPos + u.winpct(0.113f)) - (MeasureTextEx(menuAss.redHatDisplayItalicLarge, TextFormat("%3.0f", Percent), u.hinpct(0.1f),0).x/1.5f),u.hpct(0.243f)},u.hinpct(0.1f),0,
                   ColorBrightness(GOLD,-0.5));
        float flawlessFontSize = 0.03f;
        DrawTextEx(
                menuAss.rubikBoldItalic,
                "Flawless!",
                {
                        (cardPos + u.winpct(0.113f))-(MeasureTextEx(menuAss.rubikBoldItalic, "Flawless!", u.hinpct(flawlessFontSize), 0.0f).x/2),
                        u.hpct(0.35f)},
                u.hinpct(flawlessFontSize),
                0.0f,
                WHITE);
    }
    if (player->stats->Quit && !player->Bot) {
        float flawlessFontSize = 0.05f;
        DrawTextEx(
                menuAss.rubikBoldItalic,
                "Quit",
                {
                        (cardPos + u.winpct(0.11f))-(MeasureTextEx(menuAss.rubikBoldItalic, "Quit", u.hinpct(flawlessFontSize), 0.0f).x/2),
                        u.hpct(0.35f)},
                u.hinpct(flawlessFontSize),
                0.0f,
                RED);
    }
    if (player->Bot) {
        float flawlessFontSize = 0.05f;
        DrawTextEx(
                menuAss.rubikBoldItalic,
                "BOT",
                {
                        (cardPos + u.winpct(0.11f))-(MeasureTextEx(menuAss.rubikBoldItalic, "BOT", u.hinpct(flawlessFontSize), 0.0f).x/2),
                        u.hpct(0.35f)},
                u.hinpct(flawlessFontSize),
                0.0f,
                SKYBLUE);
    }
    DrawTextEx(menuAss.redHatDisplayItalicLarge, TextFormat("%3.0f%%", Percent), {(cardPos + u.winpct(0.11f)) - (MeasureTextEx(menuAss.redHatDisplayItalicLarge, rendAsFC ? TextFormat("%3.0f", Percent) : TextFormat("%3.0f%%", Percent), u.hinpct(0.1f),0).x/(rendAsFC ? 1.5f : 2.0f)),u.hpct(0.22f)},u.hinpct(0.1f),0, rendAsFC ? YELLOW : WHITE);

    std::string InstDiffName = TextFormat("%s %s", diffList[player->Difficulty].c_str(), songPartsList[player->Instrument].c_str());
    float InstDiffPos = MeasureTextEx(menuAss.rubikBold, InstDiffName.c_str(), u.hinpct(0.03f),0).x;
    float pctSize = MeasureTextEx(menuAss.rubikBold, TextFormat("%3.0f%%", Percent), u.hinpct(0.1f),0).y;

    DrawTextEx(menuAss.rubikBold, InstDiffName.c_str(), {cardPos + u.winpct(0.11f) - (InstDiffPos/2), u.hpct(0.24f)+(pctSize/2)}, u.hinpct(0.03f),0,WHITE);

    float statsHeight = u.hpct(0.2f) + u.hinpct(0.415f);
    float statsLeft = cardPos + u.winpct(0.01f);
    float statsRight = cardPos + u.winpct(0.21f);

    DrawTextEx(menuAss.rubik, "Perfects:", {statsLeft, statsHeight}, u.hinpct(0.03f),0,WHITE);
    DrawTextEx(menuAss.rubik, "Goods:", {statsLeft, statsHeight+u.hinpct(0.035f)}, u.hinpct(0.03f),0,WHITE);
    DrawTextEx(menuAss.rubik, "Missed:", {statsLeft, statsHeight+u.hinpct(0.07f)}, u.hinpct(0.03f),0,WHITE);
    DrawTextEx(menuAss.rubik, "Strikes:", {statsLeft, statsHeight+u.hinpct(0.105f)}, u.hinpct(0.03f),0,WHITE);
    DrawTextEx(menuAss.rubik, "Max Streak:", {statsLeft, statsHeight+u.hinpct(0.14f)}, u.hinpct(0.03f),0,WHITE);
    DrawTextEx(menuAss.rubik, "Notes:", {statsLeft, statsHeight+u.hinpct(0.175f)}, u.hinpct(0.03f),0,WHITE);

    float hitpct = ((float)player->stats->PerfectHit / (float)player->stats->Notes);
    float pHitPercent = floorf(hitpct * 100.0f);
    std::string PerfectDisplay = TextFormat("%01i (%3.0f%%)", player->stats->PerfectHit, pHitPercent);

    float gpct = ((float)(player->stats->NotesHit-player->stats->PerfectHit) / (float)player->stats->Notes);
    float gHitPercent = floorf(gpct * 100.0f);
    std::string GoodDisplay = TextFormat("%01i (%3.0f%%)", player->stats->NotesHit-player->stats->PerfectHit, gHitPercent);

    float mpct = ((float)player->stats->NotesMissed / (float)player->stats->Notes);
    float mHitPercent = floorf(mpct * 100.0f);
    std::string MissDisplay = TextFormat("%01i (%3.0f%%)", player->stats->NotesMissed, mHitPercent);

    std::string NotesDisplay = TextFormat("%01i", player->stats->Notes);

    int MaxNotes = song.parts[player->Instrument]->charts[player->Difficulty].notes.size();
    float FontSize = u.hinpct(0.03f);
    DrawTextEx(menuAss.rubik, PerfectDisplay.c_str(), {statsRight - MeasureTextEx(menuAss.rubik, PerfectDisplay.c_str(), FontSize, 0).x, statsHeight}, FontSize, 0, WHITE);
    DrawTextEx(menuAss.rubik, GoodDisplay.c_str(), {statsRight - MeasureTextEx(menuAss.rubik, GoodDisplay.c_str(), FontSize, 0).x, statsHeight+u.hinpct(0.035f)}, FontSize,0,WHITE);
    DrawTextEx(menuAss.rubik, MissDisplay.c_str(), {statsRight - MeasureTextEx(menuAss.rubik, MissDisplay.c_str(), FontSize, 0).x, statsHeight+u.hinpct(0.07f)}, FontSize,0,WHITE);
    DrawTextEx(menuAss.rubik, TextFormat("%01i",  player->stats->Overhits, player->stats->Notes), {statsRight - MeasureTextEx(menuAss.rubik, TextFormat("%01i", player->stats->Overhits), u.hinpct(0.03f), 0).x, statsHeight+u.hinpct(0.105f)}, u.hinpct(0.03f),0,WHITE);
    DrawTextEx(menuAss.rubik, TextFormat("%01i/%01i",  player->stats->MaxCombo, player->stats->Notes), {statsRight - MeasureTextEx(menuAss.rubik, TextFormat("%01i/%01i", player->stats->MaxCombo, player->stats->Notes), u.hinpct(0.03f), 0).x, statsHeight+u.hinpct(0.14f)}, u.hinpct(0.03f),0,WHITE);
    DrawTextEx(menuAss.rubik, NotesDisplay.c_str(), {statsRight - MeasureTextEx(menuAss.rubik, NotesDisplay.c_str(), u.hinpct(0.03f), 0).x, statsHeight+u.hinpct(0.17f)}, u.hinpct(0.03f),0,WHITE);
    // DrawTextEx(menuAss.rubik, TextFormat("%2.2f", player->totalOffset / player->notesHit), {statsRight - MeasureTextEx(menuAss.rubik, TextFormat("%2.2f", player->totalOffset / player->notesHit), u.hinpct(0.03f), 0).x, statsHeight+u.hinpct(0.17f)}, u.hinpct(0.03f),0,WHITE);
};

// todo: replace player with band stats

void GameMenu::renderStarsP(PlayerGameplayStats* stats, float xPos, float yPos, float scale, bool left) {
    int starsval = stats->Stars();

    float starX = left ? 0 : scale*2.5f;
    for (int i = 0; i < 5; i++) {
        DrawTexturePro(menuAss.emptyStar, {0,0,(float)menuAss.emptyStar.width,(float)menuAss.emptyStar.height}, {(xPos+(i*scale)-starX),yPos, scale, scale},{0,0},0,WHITE);
    }
    for (int i = 0; i < starsval ; i++) {
        DrawTexturePro(stats->GoldStars?menuAss.goldStar:menuAss.star, {0,0,(float)menuAss.emptyStar.width,(float)menuAss.emptyStar.height}, {(xPos+(i*scale)-starX),yPos, scale, scale}, {0,0},0, WHITE);
    }
};

void GameMenu::renderStars(BandGameplayStats* stats, float xPos, float yPos, float scale, bool left) {
    int starsval = stats->Stars();

    float starX = left ? 0 : scale*2.5f;
    for (int i = 0; i < 5; i++) {
        DrawTexturePro(menuAss.emptyStar, {0,0,(float)menuAss.emptyStar.width,(float)menuAss.emptyStar.height}, {(xPos+(i*scale)-starX),yPos, scale, scale},{0,0},0,WHITE);
    }
    for (int i = 0; i < starsval ; i++) {
        DrawTexturePro(stats->GoldStars?menuAss.goldStar:menuAss.star, {0,0,(float)menuAss.emptyStar.width,(float)menuAss.emptyStar.height}, {(xPos+(i*scale)-starX),yPos, scale, scale}, {0,0},0, WHITE);
    }
};

void GameMenu::DrawAlbumArtBackground(Texture2D song) {
    float diagonalLength = sqrtf((float)(GetScreenWidth() * GetScreenWidth()) + (float)(GetScreenHeight() * GetScreenHeight()));
    float RectXPos = GetScreenWidth() / 2;
    float RectYPos = diagonalLength / 2;

    BeginShaderMode(menuAss.bgShader);
    DrawTexturePro(song, Rectangle{0, 0, (float) song.width,
                                                 (float) song.width},
                   Rectangle{RectXPos, -RectYPos * 2,
                             diagonalLength * 2, diagonalLength * 2}, {0,0}, 45,
                   WHITE);
    EndShaderMode();
};

void GameMenu::DrawVersion() {
    DrawTextEx(menuAss.josefinSansItalic, TextFormat("%s-%s:%s",menuVersion.c_str() , gitBranch.c_str(),menuCommitHash.c_str()),
        {u.wpct(0.0025f), u.hpct(0.0025f)},
        u.hinpct(0.025f), 0, WHITE);
};

// todo: text box rendering for splashes, cleanup of buttons
void GameMenu::loadMainMenu() {
    OvershellRenderer osr;
    AudioManager& menuAudioManager = AudioManager::getInstance();
    std::filesystem::path directory = GetPrevDirectoryPath(GetApplicationDirectory());
    SongList& songList = SongList::getInstance();
    std::ifstream splashes;
    splashes.open((directory / "Assets/ui/splashes.txt"));

    static std::string result;
    std::string line;
    if (!stringChosen) {
        for (std::size_t n = 0; std::getline(splashes, line); n++) {
            int rng = GetRandomValue(0, n);
            if (rng < 1)
                result = line;
        }
        stringChosen = true;
    }
    if (std::filesystem::exists("songCache.encr") && songList.songs.size()>0) {
        if (!albumArtLoaded) {
            AlbumArtBackground = menuAss.highwayTexture;

            if (!songChosen && songsLoaded) {
                if (!randomSongChosen) {

                    int my = GetRandomValue(0, (int) songList.songs.size() - 1);
                    songList.curSong = &songList.songs[my];
                    // ChosenSongInt = my;
                    randomSongChosen = true;
                }

                songList.curSong->LoadAlbumArt();
                AlbumArtBackground = songList.curSong->albumArtBlur;
                TraceLog(LOG_INFO, songList.curSong->title.c_str());
                songChosen = true;
            } else {
                AlbumArtBackground = menuAss.highwayTexture;
            };
            albumArtLoaded = true;
        };
        if (!streamsLoaded) {
            if (songList.curSong->ini)
                songList.curSong->LoadAudioINI(songList.curSong->songDir);
            else
                songList.curSong->LoadAudio(songList.curSong->songInfoPath);
            menuAudioManager.loadStreams(songList.curSong->stemsPath);
            streamsLoaded = true;
            for (auto& stream : menuAudioManager.loadedStreams) {
                menuAudioManager.SetAudioStreamVolume(stream.handle, settings.MainVolume * 0.15f);

            }
            menuAudioManager.BeginPlayback(menuAudioManager.loadedStreams[0].handle);
        }
        DrawAlbumArtBackground(AlbumArtBackground);
    }
    float SplashFontSize = u.hinpct(0.03f);
    float SplashHeight = MeasureTextEx(menuAss.josefinSansItalic, result.c_str(), SplashFontSize, 0).y;
    float SplashWidth = MeasureTextEx(menuAss.josefinSansItalic, result.c_str(), SplashFontSize, 0).x;

    float SongFontSize = u.hinpct(0.03f);


    Vector2 StringBox = {u.wpct(0.01f),  u.hpct(0.8125f)};
    DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(), Color{0,0,0,128});
    DrawTopOvershell(0.2f);
    DrawBottomOvershell();
    DrawBottomBottomOvershell();
    menuCommitHash.erase(7);
    float logoHeight = u.hinpct(0.145f);
    DrawVersion();

    Color accentColor = ColorBrightness(ColorContrast(Color{255,0,255,128}, -0.125f),-0.25f);

    DrawRectangle(0,u.hpct(0.8f),u.LeftSide, u.hinpct(0.05f), accentColor);
    DrawRectangleGradientH(u.LeftSide,u.hpct(0.8f),SplashWidth+u.winpct(0.1f),u.hinpct(0.05f),accentColor,Color{0,0,0,0});

    DrawTextEx(menuAss.josefinSansItalic, result.c_str(), StringBox, SplashFontSize, 0, WHITE);

    Rectangle LogoRect = { u.LeftSide + u.winpct(0.01f), u.hpct(0.035f), Remap(menuAss.encoreWhiteLogo.height, 0, menuAss.encoreWhiteLogo.width / 4.25, 0, u.winpct(0.5f)), logoHeight};
    DrawTexturePro(menuAss.encoreWhiteLogo, {0,0,(float)menuAss.encoreWhiteLogo.width,(float)menuAss.encoreWhiteLogo.height}, LogoRect, {0,0}, 0, WHITE);
    GuiSetStyle(DEFAULT, TEXT_SIZE, (int)u.hinpct(0.08f));
    GuiSetFont(menuAss.redHatDisplayBlack);
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0xaaaaaaFF);
    GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, 0xFFFFFFFF);
    GuiSetStyle(BUTTON, BORDER_WIDTH, 0);
    GuiSetStyle(BUTTON, BACKGROUND_COLOR, 0);
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x00000000);
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, 0x00000000);
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, 0x00000000);
    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, 0x00000000);
    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, 0x00000000);
    GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, 0x00000000);
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0x00000000);
    if (GuiButton({0, u.hpct(0.8f), u.LeftSide+SplashWidth, u.hpct(0.05f)}, "")) {
        stringChosen = false;
    }
    if (std::filesystem::exists("songCache.encr")) {
        if (GuiButton({u.wpct(0.02f), u.hpct(0.3f), u.winpct(0.2f), u.hinpct(0.08f)}, "Play")) {
            menuAudioManager.unloadStreams();
            streamsLoaded = false;
            streamsPaused = false;
            for (Song &songi: songList.songs) {
                songi.titleScrollTime = GetTime();
                songi.titleTextWidth = menuAss.MeasureTextRubik(songi.title.c_str(), 24);
                songi.artistScrollTime = GetTime();
                songi.artistTextWidth = menuAss.MeasureTextRubik(songi.artist.c_str(), 20);
            }
            SwitchScreen(SONG_SELECT);
        }
    } else {
        GuiSetStyle(BUTTON,BASE_COLOR_NORMAL, ColorToInt(Color{128,0,0,255}));
        GuiButton({u.wpct(0.02f), u.hpct(0.3f), u.winpct(0.2f), u.hinpct(0.08f)}, "Invalid song cache!");
        songList.ScanSongs(settings.songPaths);
        songsLoaded = false;
        DrawRectanglePro({((float) GetScreenWidth() / 2) - 125, ((float) GetScreenHeight() / 2) - 120, 250, 60},{0,0},0, Color{0,0,0,64});
        GuiSetStyle(BUTTON,BASE_COLOR_NORMAL, 0x181827FF);
    }
    if (GuiButton({u.wpct(0.02f), u.hpct(0.39f), u.winpct(0.2f), u.hinpct(0.08f)},
                  "Options")) {
        // glfwSetGamepadStateCallback(gamepadStateCallbackSetControls);
        SwitchScreen(SETTINGS);
    }
    if (GuiButton({u.wpct(0.02f), u.hpct(0.48f), u.winpct(0.2f), u.hinpct(0.08f)}, "Quit")) {
        exit(0);
    }
    if (GuiButton({u.wpct(0.8f), u.hpct(0.90f), u.winpct(0.5f), u.hinpct(0.05f)}, "Sound Test")) {
        SwitchScreen(SOUND_TEST);
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt(ColorBrightness(Color{255,0,255,255}, -0.5)));
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, ColorToInt(ColorBrightness(Color{255,0,255,255}, -0.3)));
    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, 0xFFFFFFFF);
    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, 0xFFFFFFFF);
    GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, 0xFFFFFFFF);
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0x505050ff);
    GuiSetStyle(BUTTON, BORDER_WIDTH, 2);

    if (GuiButton({(float) GetScreenWidth() - 60, (float) GetScreenHeight() - u.hpct(0.15f) - 60, 60, 60}, "")) {
        OpenURL("https://github.com/Encore-Developers/Encore");
    }


    if (GuiButton({(float) GetScreenWidth() - 120, (float) GetScreenHeight() - u.hpct(0.15f) - 60, 60, 60}, "")) {
        OpenURL("https://discord.gg/GhkgVUAC9v");
    }

    GuiSetStyle(DEFAULT, TEXT_SIZE, (int)u.hinpct(0.03f));
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    GuiSetFont(menuAss.rubik);
    DrawTextureEx(menuAss.github, {(float) GetScreenWidth() - 54, (float) GetScreenHeight() - 54 - u.hpct(0.15f) }, 0, 0.2, WHITE);
    DrawTextureEx(menuAss.discord, {(float) GetScreenWidth() - 113, (float) GetScreenHeight() - 48 - u.hpct(0.15f) }, 0, 0.075,
                      WHITE);
    if (streamsLoaded) {
        Vector2 TitleSize = MeasureTextEx(menuAss.rubikBoldItalic, songList.curSong->title.c_str(), SongFontSize, 0);
        Vector2 ArtistSize = MeasureTextEx(menuAss.rubikItalic, songList.curSong->artist.c_str(), SongFontSize, 0);

        Vector2 SongTitleBox = {u.RightSide - TitleSize.x - u.winpct(0.01f),  u.hpct(0.2f) - u.hinpct(0.1f) - (TitleSize.y*1.1f)};
        Vector2 SongArtistBox = {u.RightSide - ArtistSize.x - u.winpct(0.01f),  u.hpct(0.2f) - u.hinpct(0.1f)};
        SongTitleBox.x = SongTitleBox.x - u.hinpct(0.12f);
        SongArtistBox.x = SongArtistBox.x - u.hinpct(0.12f);
        DrawTextEx(menuAss.rubikBoldItalic, songList.curSong->title.c_str(), SongTitleBox, SongFontSize, 0, WHITE);
        DrawTextEx(menuAss.rubikItalic, songList.curSong->artist.c_str(), SongArtistBox, SongFontSize, 0, WHITE);



        for (auto& stream : menuAudioManager.loadedStreams) {
            menuAudioManager.SetAudioStreamVolume(stream.handle, settings.MainVolume * settings.MenuVolume);
        }
        float played = menuAudioManager.GetMusicTimePlayed();
        float length = menuAudioManager.GetMusicTimeLength();
        DrawRectangle(0, u.hpct(0.2f) - u.hinpct(0.01f), Remap(played, 0, length, 0, GetScreenWidth()),
                      u.hinpct(0.005f), SKYBLUE);

        if (played >= length-0.5) {
            menuAudioManager.unloadStreams();
            albumArtLoaded = false;
            streamsPaused = false;
            songChosen = false;
            streamsLoaded = false;
            randomSongChosen = false;
        }
        GuiSetStyle(BUTTON, BORDER_WIDTH, 0);
        if (GuiButton({u.RightSide-u.hinpct(0.12f),u.hpct(0.2f)-u.hinpct(0.1f)-u.hinpct(0.031f),u.hinpct(0.06f),u.hinpct(0.06f)}, streamsPaused ? ">" : "||")) {
            if (!streamsPaused) {
                menuAudioManager.pauseStreams();
                streamsPaused = true;
            }
            else if (streamsPaused) {
                streamsPaused = false;
                menuAudioManager.playStreams();
            }
        }
        if (GuiButton({u.RightSide-u.hinpct(0.06f),u.hpct(0.2f)-u.hinpct(0.1f)-u.hinpct(0.031f),u.hinpct(0.06f),u.hinpct(0.06f)}, ">>")) {
            menuAudioManager.unloadStreams();
            albumArtLoaded = false;
            streamsPaused = false;
            songChosen = false;
            streamsLoaded = false;
            randomSongChosen = false;
        }
        GuiSetStyle(BUTTON, BORDER_WIDTH, 2);
    } else {
        Vector2 TitleSize = MeasureTextEx(menuAss.rubikBoldItalic, "No song loaded", SongFontSize, 0);
        Vector2 ArtistSize = MeasureTextEx(menuAss.rubikItalic, "", SongFontSize, 0);

        Vector2 SongTitleBox = {u.RightSide - TitleSize.x - u.winpct(0.01f),  u.hpct(0.2f) - u.hinpct(0.1f) - (TitleSize.y*1.1f)};
        Vector2 SongArtistBox = {u.RightSide - ArtistSize.x - u.winpct(0.01f),  u.hpct(0.2f) - u.hinpct(0.1f)};
        DrawTextEx(menuAss.rubikBoldItalic, songList.curSong->title.c_str(), SongTitleBox, SongFontSize, 0, WHITE);
        DrawTextEx(menuAss.rubikItalic, songList.curSong->artist.c_str(), SongArtistBox, SongFontSize, 0, WHITE);
    }
    osr.DrawOvershell();
}


bool AlbumArtLoadingStuff = false;
// sentenced to
void GameMenu::showResults() {
    PlayerManager &player_manager = PlayerManager::getInstance();
    for (int i = 0; i < player_manager.PlayersActive; i++) {
        renderPlayerResults(player_manager.GetActivePlayer(i), *songListMenu.curSong, i);
    }

    DrawTopOvershell(0.2f);
    DrawBottomOvershell();
    DrawBottomBottomOvershell();
    OvershellRenderer osr;
    osr.DrawOvershell();
    DrawVersion();

    float songNamePos = (float)GetScreenWidth()/2 - MeasureTextEx(menuAss.redHatDisplayBlack,songListMenu.curSong->title.c_str(), u.hinpct(0.09f), 0).x/2;
    float bigScorePos = (float)GetScreenWidth()/2 - u.winpct(0.04f) - MeasureTextEx(menuAss.redHatDisplayItalicLarge,scoreCommaFormatter(player_manager.BandStats.Score).c_str(), u.hinpct(0.08f), 0).x;
    float bigStarPos = (float)GetScreenWidth()/2 + u.winpct(0.005f);
    float scoreWidth = MeasureTextEx(menuAss.redHatDisplayItalicLarge,scoreCommaFormatter(player_manager.BandStats.Score).c_str(), u.hinpct(0.06f), 0).x;

    DrawTextEx(menuAss.redHatDisplayItalicLarge, songListMenu.curSong->title.c_str(), {u.LeftSide,u.hpct(0.02125f)},u.hinpct(0.05f),0,WHITE);
    DrawTextEx(menuAss.rubikItalic, songListMenu.curSong->artist.c_str(), {u.LeftSide,u.hpct(0.07f)},u.hinpct(0.035f),0,WHITE);
    DrawTextEx(menuAss.redHatDisplayItalicLarge, scoreCommaFormatter(player_manager.BandStats.Score).c_str(), {u.LeftSide,u.hpct(0.1f)},u.hinpct(0.06f),0, GetColor(0x00adffFF));
    DrawTextEx(menuAss.redHatDisplayItalicLarge, "!", {u.LeftSide,u.hpct(0.1525f)},u.hinpct(0.05f),0, RED);
    DrawTextEx(menuAss.josefinSansItalic, "  Scoring is disabled in indev builds", {u.LeftSide,u.hpct(0.158f)},u.hinpct(0.025f),0.125,WHITE);
    renderStars(&player_manager.BandStats, scoreWidth + u.LeftSide + u.winpct(0.01f), u.hpct(0.105f), u.hinpct(0.05f),true);
    // assets.DrawTextRHDI(player->songToBeJudged.title.c_str(),songNamePos, 50, WHITE);
}

void GameMenu::SwitchScreen(Screens screen){
    currentScreen = screen;
    Menu::onNewMenu = true;
    switch (screen) {
        case MENU:
            // reset lerps
            stringChosen = false;
            break;
        case SONG_SELECT:
        case READY_UP:
        case GAMEPLAY:
            break;
        case RESULTS:
            AlbumArtLoadingStuff = false;
            break;
        case SETTINGS:
        case CALIBRATION:
        case CHART_LOADING_SCREEN:
        case SOUND_TEST:
        case CACHE_LOADING_SCREEN:
        default:
            break;
    }
}

void GameMenu::DrawFPS(int posX, int posY) {
        Color color = LIME;                         // Good FPS
        int fps = GetFPS();

        if ((fps < 30) && (fps >= 15)) color = ORANGE;  // Warning FPS
        else if (fps < 15) color = RED;             // Low FPS

        DrawTextEx(menuAss.josefinSansItalic, TextFormat("%2i FPS", fps), {(float)posX, (float)posY}, u.hinpct(0.025f), 0, color);
}

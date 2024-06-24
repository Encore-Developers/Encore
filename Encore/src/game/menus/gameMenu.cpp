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

bool randomSongChosen = false;

std::string menuCommitHash = GIT_COMMIT_HASH;
std::string menuVersion = ENCORE_VERSION;
Assets &menuAss = Assets::getInstance();
Settings& settings = Settings::getInstance();
SongList &songListMenu = SongList::getInstance();
Units u = Units::getInstance();
std::vector<std::string> songPartsList{ "Drums","Bass","Guitar","Vocals","Classic Drums", "Classic Bass", "Classic Lead"};
std::vector<std::string> diffList{ "Easy","Medium","Hard","Expert" };
void Menu::DrawTopOvershell(float TopOvershell) {
    DrawRectangle(0,0,(int)GetScreenWidth(), u.hpct(TopOvershell),WHITE);
    DrawRectangle(0,0,(int)GetScreenWidth(), u.hpct(TopOvershell)-u.hinpct(0.005f),ColorBrightness(GetColor(0x181827FF),-0.25f));
}

void Menu::DrawBottomOvershell() {
    float BottomOvershell = GetScreenHeight() - u.hpct(0.15f);
    DrawRectangle(0,BottomOvershell,(float)(GetScreenWidth()), (float)GetScreenHeight(),WHITE);
    DrawRectangle(0,BottomOvershell+u.hinpct(0.005f),(float)(GetScreenWidth()), (float)GetScreenHeight(),ColorBrightness(GetColor(0x181827FF),-0.5f));
}

void Menu::DrawBottomBottomOvershell() {
    float BottomBottomOvershell = GetScreenHeight() - u.hpct(0.1f);
    DrawRectangle(0,BottomBottomOvershell,(float)(GetScreenWidth()), (float)GetScreenHeight(),WHITE);
    DrawRectangle(0,BottomBottomOvershell+u.hinpct(0.005f),(float)(GetScreenWidth()), (float)GetScreenHeight(),ColorBrightness(GetColor(0x181827FF),-0.5f));
}

// should be reduced to just PlayerSongStats (instead of Player) eventually
void Menu::renderPlayerResults(Player player, Song song) {

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
    if (player.perfectHit==player.notes && rendAsFC) {
        DrawRectangleGradientV(cardPos,u.hpct(0.2f)+ u.hinpct(0.2f), u.winpct(0.22f), u.hinpct(0.63f), GetColor(0x181827FF),
                               ColorBrightness(WHITE, -0.5f));
    }

    DrawLine(cardPos,u.hpct(0.2f) + u.hinpct(0.2f), cardPos + u.winpct(0.22f),u.hpct(0.2f) + u.hinpct(0.2f),WHITE);
    DrawLine(cardPos,u.hpct(0.2f) + u.hinpct(0.4f), cardPos + u.winpct(0.22f),u.hpct(0.2f) + u.hinpct(0.4f),WHITE);

    float scorePos = (cardPos + u.winpct(0.11f)) - (MeasureTextEx(menuAss.redHatDisplayItalic, scoreCommaFormatter(player.score).c_str(), u.hinpct(0.07f), 0).x /2);
    float Percent = floorf(((float)player.notesHit/ (float)player.notes) * 100.0f);

    DrawTextEx(
            menuAss.redHatDisplayItalic,
            scoreCommaFormatter(player.score).c_str(),
            {
                    scorePos,
                    (float)GetScreenHeight()/2},
            u.hinpct(0.065f),
            0,
            GetColor(0x00adffFF));

    renderStars(player,  (cardPos + u.winpct(0.11f)), (float)GetScreenHeight()/2 - u.hinpct(0.06f), u.hinpct(0.055f),false);


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
    } if (player.quit) {
        float flawlessFontSize = 0.05f;
        DrawTextEx(
                menuAss.rubikBoldItalic,
                "Quit",
                {
                        (cardPos + u.winpct(0.11f))-(MeasureTextEx(menuAss.rubikBoldItalic, "Quit", u.hinpct(flawlessFontSize), 0.0f).x/2),
                        u.hpct(0.335f)},
                u.hinpct(flawlessFontSize),
                0.0f,
                RED);
    }
    DrawTextEx(menuAss.redHatDisplayItalicLarge, TextFormat("%3.0f%%", Percent), {(cardPos + u.winpct(0.11f)) - (MeasureTextEx(menuAss.redHatDisplayItalicLarge, rendAsFC ? TextFormat("%3.0f", Percent) : TextFormat("%3.0f%%", Percent), u.hinpct(0.1f),0).x/(rendAsFC ? 1.5f : 2.0f)),u.hpct(0.24f)},u.hinpct(0.1f),0, rendAsFC ? YELLOW : WHITE);

    float statsHeight = u.hpct(0.2f) + u.hinpct(0.415f);
    float statsLeft = cardPos + u.winpct(0.01f);
    float statsRight = cardPos + u.winpct(0.21f);

    DrawTextEx(menuAss.rubik, "Perfects:", {statsLeft, statsHeight}, u.hinpct(0.03f),0,WHITE);
    DrawTextEx(menuAss.rubik, "Goods:", {statsLeft, statsHeight+u.hinpct(0.035f)}, u.hinpct(0.03f),0,WHITE);
    DrawTextEx(menuAss.rubik, "Missed:", {statsLeft, statsHeight+u.hinpct(0.07f)}, u.hinpct(0.03f),0,WHITE);
    DrawTextEx(menuAss.rubik, "Strikes:", {statsLeft, statsHeight+u.hinpct(0.105f)}, u.hinpct(0.03f),0,WHITE);
    DrawTextEx(menuAss.rubik, "Max Streak:", {statsLeft, statsHeight+u.hinpct(0.14f)}, u.hinpct(0.03f),0,WHITE);

    DrawTextEx(menuAss.rubikBold, TextFormat("%s %s", diffList[player.diff].c_str(), songPartsList[player.instrument].c_str()), {cardPos + u.winpct(0.11f) -
                                                                                                                                          (MeasureTextEx(menuAss.rubikBold, TextFormat("%s %s", diffList[player.diff].c_str(), songPartsList[player.instrument].c_str()), u.hinpct(0.03f),0).x/2), statsHeight+u.hinpct(0.20f)}, u.hinpct(0.03f),0,WHITE);

    int MaxNotes = song.parts[player.instrument]->charts[player.diff].notes.size();
    DrawTextEx(menuAss.rubik, TextFormat("%01i/%01i", player.perfectHit, player.notes), {statsRight - MeasureTextEx(menuAss.rubik, TextFormat("%01i/%01i", player.perfectHit, player.notes), u.hinpct(0.03f), 0).x, statsHeight}, u.hinpct(0.03f), 0, WHITE);
    DrawTextEx(menuAss.rubik, TextFormat("%01i/%01i", player.notesHit-player.perfectHit, player.notes), {statsRight - MeasureTextEx(menuAss.rubik, TextFormat("%01i/%01i", player.notesHit-player.perfectHit, player.notes), u.hinpct(0.03f), 0).x, statsHeight+u.hinpct(0.035f)}, u.hinpct(0.03f),0,WHITE);
    DrawTextEx(menuAss.rubik, TextFormat("%01i/%01i", player.notesMissed, player.notes), {statsRight - MeasureTextEx(menuAss.rubik, TextFormat("%01i/%01i", player.notesMissed, player.notes), u.hinpct(0.03f), 0).x, statsHeight+u.hinpct(0.07f)}, u.hinpct(0.03f),0,WHITE);
    DrawTextEx(menuAss.rubik, TextFormat("%01i", player.playerOverhits, player.notes), {statsRight - MeasureTextEx(menuAss.rubik, TextFormat("%01i", player.playerOverhits), u.hinpct(0.03f), 0).x, statsHeight+u.hinpct(0.105f)}, u.hinpct(0.03f),0,WHITE);
    DrawTextEx(menuAss.rubik, TextFormat("%01i/%01i", player.maxCombo, player.notes), {statsRight - MeasureTextEx(menuAss.rubik, TextFormat("%01i/%01i", player.maxCombo, player.notes), u.hinpct(0.03f), 0).x, statsHeight+u.hinpct(0.14f)}, u.hinpct(0.03f),0,WHITE);
    DrawTextEx(menuAss.rubik, TextFormat("%2.2f", player.totalOffset / player.notesHit), {statsRight - MeasureTextEx(menuAss.rubik, TextFormat("%2.2f", player.totalOffset / player.notesHit), u.hinpct(0.03f), 0).x, statsHeight+u.hinpct(0.17f)}, u.hinpct(0.03f),0,WHITE);
};

// todo: replace player with band stats
void Menu::renderStars(Player player, float xPos, float yPos, float scale, bool left) {
    int starsval = player.stars(player.songToBeJudged.parts[player.instrument]->charts[player.diff].baseScore,player.diff);
    float starPercent = (float)player.score/(float)player.songToBeJudged.parts[player.instrument]->charts[player.diff].baseScore;

    float starX = left ? 0 : scale*2.5f;
    for (int i = 0; i < 5; i++) {
        DrawTexturePro(menuAss.emptyStar, {0,0,(float)menuAss.emptyStar.width,(float)menuAss.emptyStar.height}, {(xPos+(i*scale)-starX),yPos, scale, scale},{0,0},0,WHITE);
    }
    for (int i = 0; i < starsval ; i++) {
        DrawTexturePro(player.goldStars?menuAss.goldStar:menuAss.star, {0,0,(float)menuAss.emptyStar.width,(float)menuAss.emptyStar.height}, {(xPos+(i*scale)-starX),yPos, scale, scale}, {0,0},0, WHITE);
    }
};
void Menu::DrawAlbumArtBackground(Texture2D song) {
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

void Menu::DrawVersion() {
    DrawTextEx(menuAss.josefinSansItalic, TextFormat("%s-%s",menuVersion.c_str() , menuCommitHash.c_str()), {u.wpct(0.0025f), u.hpct(0.0025f)}, u.hinpct(0.025f), 0, WHITE);
};




// todo: text box rendering for splashes, cleanup of buttons
void Menu::loadMenu(GLFWgamepadstatefun gamepadStateCallbackSetControls) {
    AudioManager& menuAudioManager = AudioManager::getInstance();
    std::filesystem::path directory = GetPrevDirectoryPath(GetApplicationDirectory());

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
    if (std::filesystem::exists("songCache.encr") && songListMenu.songs.size()>0) {
        if (!albumArtLoaded) {
            AlbumArtBackground = menuAss.highwayTexture;

            if (!songChosen && songsLoaded) {
                if (!randomSongChosen) {
                    SetRandomSeed(std::chrono::system_clock::now().time_since_epoch().count() * GetTime());
                    int my = GetRandomValue(0, (int) songListMenu.songs.size() - 1);

                    ChosenSong = songListMenu.songs[my];
                    ChosenSong.LoadAlbumArt(ChosenSong.albumArtPath);
                    ChosenSongInt = my;
                    randomSongChosen = true;

                } else {

                    ChosenSong.LoadAlbumArt(ChosenSong.albumArtPath);
                }

                AlbumArtBackground = ChosenSong.albumArtBlur;
                TraceLog(LOG_INFO, ChosenSong.title.c_str());
                songChosen = true;
            } else {
                AlbumArtBackground = menuAss.highwayTexture;
            };
            albumArtLoaded = true;

        };
        if (!streamsLoaded) {
            ChosenSong.LoadAudio(ChosenSong.songInfoPath);
            menuAudioManager.loadStreams(ChosenSong.stemsPath);
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
    float TitleHeight = MeasureTextEx(menuAss.rubikBoldItalic, ChosenSong.title.c_str(), SongFontSize, 0).y;
    float TitleWidth = MeasureTextEx(menuAss.rubikBoldItalic, ChosenSong.title.c_str(), SongFontSize, 0).x;
    float ArtistHeight = MeasureTextEx(menuAss.rubikItalic, ChosenSong.artist.c_str(), SongFontSize, 0).y;
    float ArtistWidth = MeasureTextEx(menuAss.rubikItalic, ChosenSong.artist.c_str(), SongFontSize, 0).x;

    Vector2 SongTitleBox = {u.RightSide - TitleWidth - u.winpct(0.01f),  u.hpct(0.2f) - u.hinpct(0.1f) - (TitleHeight*1.1f)};
    Vector2 SongArtistBox = {u.RightSide - ArtistWidth - u.winpct(0.01f),  u.hpct(0.2f) - u.hinpct(0.1f)};

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
            for (Song &songi: songListMenu.songs) {
                songi.titleScrollTime = GetTime();
                songi.titleTextWidth = menuAss.MeasureTextRubik(songi.title.c_str(), 24);
                songi.artistScrollTime = GetTime();
                songi.artistTextWidth = menuAss.MeasureTextRubik(songi.artist.c_str(), 20);
            }
            Menu::SwitchScreen(SONG_SELECT);
        }
    } else {
        GuiSetStyle(BUTTON,BASE_COLOR_NORMAL, ColorToInt(Color{128,0,0,255}));
        GuiButton({u.wpct(0.02f), u.hpct(0.3f), u.winpct(0.2f), u.hinpct(0.08f)}, "Invalid song cache!");
        songListMenu.ScanSongs(settings.songPaths); 
        songsLoaded = false;
        DrawRectanglePro({((float) GetScreenWidth() / 2) - 125, ((float) GetScreenHeight() / 2) - 120, 250, 60},{0,0},0, Color{0,0,0,64});
        GuiSetStyle(BUTTON,BASE_COLOR_NORMAL, 0x181827FF);
    }
    if (GuiButton({u.wpct(0.02f), u.hpct(0.39f), u.winpct(0.2f), u.hinpct(0.08f)},
                  "Options")) {
        glfwSetGamepadStateCallback(gamepadStateCallbackSetControls);
        Menu::SwitchScreen(SETTINGS);
    }
    if (GuiButton({u.wpct(0.02f), u.hpct(0.48f), u.winpct(0.2f), u.hinpct(0.08f)}, "Quit")) {
        exit(0);
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
        OpenURL("https://github.com/Encore-Developers/Encore-Raylib");
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
        SongTitleBox.x = SongTitleBox.x - u.hinpct(0.12f);
        SongArtistBox.x = SongArtistBox.x - u.hinpct(0.12f);
        DrawTextEx(menuAss.rubikBoldItalic, ChosenSong.title.c_str(), SongTitleBox, SongFontSize, 0, WHITE);
        DrawTextEx(menuAss.rubikItalic, ChosenSong.artist.c_str(), SongArtistBox, SongFontSize, 0, WHITE);



        for (auto& stream : menuAudioManager.loadedStreams) {
            menuAudioManager.SetAudioStreamVolume(stream.handle, settings.MainVolume * 0.15f);
        }
        float played = menuAudioManager.GetMusicTimePlayed(menuAudioManager.loadedStreams[0].handle);
        float length = menuAudioManager.GetMusicTimeLength(menuAudioManager.loadedStreams[0].handle);
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
        DrawTextEx(menuAss.rubikBoldItalic, ChosenSong.title.c_str(), SongTitleBox, SongFontSize, 0, WHITE);
        DrawTextEx(menuAss.rubikItalic, ChosenSong.artist.c_str(), SongArtistBox, SongFontSize, 0, WHITE);
    }
}

bool AlbumArtLoadingStuff = false;
void Menu::showResults(Player &player) {

    for (int i = 0; i < 4; i++) {
        renderPlayerResults(player, ChosenSong);
    }

    DrawTopOvershell(0.2f);
    DrawBottomOvershell();
    DrawBottomBottomOvershell();

    DrawTextEx(menuAss.josefinSansItalic, TextFormat("%s-%s",menuVersion.c_str() , menuCommitHash.c_str()), {u.wpct(0), u.hpct(0)}, u.hinpct(0.025f), 0, WHITE);

    float songNamePos = (float)GetScreenWidth()/2 - MeasureTextEx(menuAss.redHatDisplayBlack,player.songToBeJudged.title.c_str(), u.hinpct(0.09f), 0).x/2;
    float bigScorePos = (float)GetScreenWidth()/2 - u.winpct(0.04f) - MeasureTextEx(menuAss.redHatDisplayItalicLarge,scoreCommaFormatter(player.score).c_str(), u.hinpct(0.08f), 0).x;
    float bigStarPos = (float)GetScreenWidth()/2 + u.winpct(0.005f);


    DrawTextEx(menuAss.redHatDisplayBlack, player.songToBeJudged.title.c_str(), {songNamePos,u.hpct(0.01f)},u.hinpct(0.09f),0,WHITE);
    DrawTextEx(menuAss.redHatDisplayItalicLarge, scoreCommaFormatter(player.score).c_str(), {bigScorePos,u.hpct(0.1f)},u.hinpct(0.08f),0, GetColor(0x00adffFF));
    renderStars(player, bigStarPos, u.hpct(0.1125f), u.hinpct(0.055f),true);
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
            AlbumArtLoadingStuff = false;
            break;
        case SETTINGS:
            break;
        case CALIBRATION:
            break;
    }
}

void Menu::DrawFPS(int posX, int posY) {
        Color color = LIME;                         // Good FPS
        int fps = GetFPS();

        if ((fps < 30) && (fps >= 15)) color = ORANGE;  // Warning FPS
        else if (fps < 15) color = RED;             // Low FPS

        DrawTextEx(menuAss.josefinSansItalic, TextFormat("%2i FPS", fps), {(float)posX, (float)posY}, u.hinpct(0.025f), 0, color);
}

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
#include "../song/songlist.h"
#include "gameplay/gameplayRenderer.h"

#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH
#endif
#include "MenuManager.h"
#include "settings.h"

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
SongList &songListMenu = TheSongList;
Units u = Units::getInstance();

MainMenu TheGameMenu;

Font GameMenu::LoadFontFilter(const std::filesystem::path &fontPath) {
    int fontSize = 128;
    Font font = LoadFontEx(fontPath.string().c_str(), fontSize, 0, 250, FONT_DEFAULT);
    font.baseSize = 128;
    font.glyphCount = 250;
    int fileSize = 0;
    unsigned char *fileData = LoadFileData(fontPath.string().c_str(), &fileSize);
    font.glyphs = LoadFontData(fileData, fileSize, 128, 0, 250, FONT_SDF);
    std::filesystem::path atlasFilePath =
        ((fontPath.parent_path() / fontPath.filename()).string() + ".png");
    Image atlas = GenImageFontAtlas(font.glyphs, &font.recs, 250, 128, 4, 0);
    ExportImage(atlas, atlasFilePath.string().c_str());
    font.texture = LoadTextureFromImage(atlas);
    UnloadImage(atlas);
    SetTextureFilter(font.texture, TEXTURE_FILTER_TRILINEAR);
    return font;
}

Texture2D GameMenu::LoadTextureFilter(const std::filesystem::path &texturePath) {
    Texture2D tex = LoadTexture(texturePath.string().c_str());
    GenTextureMipmaps(&tex);
    SetTextureFilter(tex, TEXTURE_FILTER_TRILINEAR);
    return tex;
}

void GameMenu::mhDrawText(
    Font font,
    std::string text,
    Vector2 pos,
    float fontSize,
    Color color,
    Shader sdfShader,
    int align
) {
    float textLeftPos = pos.x;
    float TextWidth = MeasureTextEx(font, text.c_str(), fontSize, 0).x;

    switch (align) {
    case CENTER: {
        textLeftPos = pos.x - (TextWidth / 2);
        break;
    }
    case RIGHT: {
        textLeftPos = pos.x - (TextWidth);
        break;
    }
    }
    BeginShaderMode(sdfShader);
    DrawTextEx(font, text.c_str(), { textLeftPos, pos.y }, fontSize, 0, color);
    EndShaderMode();
}

void GameMenu::DrawTopOvershell(float TopOvershell) {
    DrawRectangleGradientV(
        0,
        u.hpct(TopOvershell) - 2,
        GetScreenWidth(),
        u.hinpct(0.025f),
        Color { 0, 0, 0, 128 },
        Color { 0, 0, 0, 0 }
    );
    DrawRectangle(0, 0, (int)GetScreenWidth(), u.hpct(TopOvershell), WHITE);
    DrawRectangle(
        0,
        0,
        (int)GetScreenWidth(),
        u.hpct(TopOvershell) - u.hinpct(0.005f),
        ColorBrightness(GetColor(0x181827FF), -0.25f)
    );
}

void GameMenu::DrawBottomOvershell() {
    float BottomOvershell = GetScreenHeight() - u.hpct(0.15f);
    DrawRectangle(
        0, BottomOvershell, (float)(GetScreenWidth()), (float)GetScreenHeight(), WHITE
    );
    DrawRectangle(
        0,
        BottomOvershell + u.hinpct(0.005f),
        (float)(GetScreenWidth()),
        (float)GetScreenHeight(),
        ColorBrightness(GetColor(0x181827FF), -0.5f)
    );
}

// should be reduced to just PlayerSongStats (instead of Player) eventually

// todo: replace player with band stats

void GameMenu::DrawAlbumArtBackground(Texture2D song) {
    float diagonalLength = sqrtf(
        (float)(GetScreenWidth() * GetScreenWidth())
        + (float)(GetScreenHeight() * GetScreenHeight())
    );
    float RectXPos = GetScreenWidth() / 2;
    float RectYPos = diagonalLength / 2;

    BeginShaderMode(menuAss.bgShader);
    DrawTexturePro(
        song,
        Rectangle { 0, 0, (float)song.width, (float)song.width },
        Rectangle { RectXPos, -RectYPos * 2, diagonalLength * 2, diagonalLength * 2 },
        { 0, 0 },
        45,
        WHITE
    );
    EndShaderMode();
};

void GameMenu::DrawVersion() {
    DrawTextEx(
        menuAss.josefinSansItalic,
        TextFormat(
            "%s-%s:%s", menuVersion.c_str(), gitBranch.c_str(), menuCommitHash.c_str()
        ),
        { u.wpct(0.0025f), u.hpct(0.0025f) },
        u.hinpct(0.025f),
        0,
        WHITE
    );
};

void MainMenu::Load() {
    std::filesystem::path directory = GetPrevDirectoryPath(GetApplicationDirectory());
    SongList &songList = TheSongList;
    std::ifstream splashes;
    splashes.open((directory / "Assets/ui/splashes.txt"));
    std::string line;

    for (std::size_t n = 0; std::getline(splashes, line); n++) {
        int rng = GetRandomValue(0, n);
        if (rng < 1)
            SplashString = line;
    }

    if (std::filesystem::exists("songCache.encr") && songList.songs.size() > 0) {
        AlbumArtBackground = menuAss.highwayTexture;

        try {
            int my = GetRandomValue(0, (int)songList.songs.size() - 1);
            songList.curSong = &songList.songs[my];
            // ChosenSongInt = my;

            songList.curSong->LoadAlbumArt();
            AlbumArtBackground = songList.curSong->albumArtBlur;
            TraceLog(LOG_INFO, songList.curSong->title.c_str());
            songChosen = true;
            albumArtLoaded = true;
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            AlbumArtBackground = menuAss.highwayTexture;
        };

        if (songList.curSong->ini)
            songList.curSong->LoadAudioINI(songList.curSong->songDir);
        else
            songList.curSong->LoadAudio(songList.curSong->songInfoPath);
        TheAudioManager.loadStreams(songList.curSong->stemsPath);
        streamsLoaded = true;
        for (auto &stream : TheAudioManager.loadedStreams) {
            TheAudioManager.SetAudioStreamVolume(
                stream.handle, TheGameSettings.avMainVolume * 0.15f
            );
        }
        TheAudioManager.BeginPlayback(TheAudioManager.loadedStreams[0].handle);
    }
}

// todo: text box rendering for splashes, cleanup of buttons
void MainMenu::Draw() {
    std::filesystem::path directory = GetPrevDirectoryPath(GetApplicationDirectory());
    SongList &songList = TheSongList;
    std::ifstream splashes;

    static std::string result;

    if (albumArtLoaded)
        GameMenu::DrawAlbumArtBackground(AlbumArtBackground);

    float SplashFontSize = u.hinpct(0.03f);
    float SplashHeight =
        MeasureTextEx(menuAss.josefinSansItalic, result.c_str(), SplashFontSize, 0).y;
    float SplashWidth =
        MeasureTextEx(menuAss.josefinSansItalic, result.c_str(), SplashFontSize, 0).x;

    float SongFontSize = u.hinpct(0.03f);

    Vector2 StringBox = { u.wpct(0.01f), u.hpct(0.2125f) };
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color { 0, 0, 0, 128 });
    GameMenu::DrawTopOvershell(0.2f);

    float logoHeight = u.hinpct(0.145f);
    GameMenu::DrawVersion();

    Color accentColor =
        ColorBrightness(ColorContrast(Color { 255, 0, 255, 128 }, -0.125f), -0.25f);

    DrawRectangle(0, u.hpct(0.2f), u.LeftSide, u.hinpct(0.05f), accentColor);
    DrawRectangleGradientH(
        u.LeftSide,
        u.hpct(0.2f),
        SplashWidth + u.winpct(0.1f),
        u.hinpct(0.05f),
        accentColor,
        Color { 0, 0, 0, 0 }
    );

    DrawTextEx(
        menuAss.josefinSansItalic, result.c_str(), StringBox, SplashFontSize, 0, WHITE
    );

    Rectangle LogoRect = { u.LeftSide + u.winpct(0.01f),
                           u.hpct(0.035f),
                           Remap(
                               menuAss.encoreWhiteLogo.height,
                               0,
                               menuAss.encoreWhiteLogo.width / 4.25,
                               0,
                               u.winpct(0.5f)
                           ),
                           logoHeight };
    DrawTexturePro(
        menuAss.encoreWhiteLogo,
        { 0,
          0,
          (float)menuAss.encoreWhiteLogo.width,
          (float)menuAss.encoreWhiteLogo.height },
        LogoRect,
        { 0, 0 },
        0,
        WHITE
    );
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
    if (GuiButton({ 0, u.hpct(0.8f), u.LeftSide + SplashWidth, u.hpct(0.05f) }, "")) {
        stringChosen = false;
    }
    if (std::filesystem::exists("songCache.encr")) {
        if (GuiButton(
                { u.wpct(0.02f), u.hpct(0.3f), u.winpct(0.2f), u.hinpct(0.08f) }, "Play"
            )) {
            TheAudioManager.unloadStreams();
            streamsLoaded = false;
            streamsPaused = false;
            for (Song &songi : songList.songs) {
                songi.titleScrollTime = GetTime();
                songi.titleTextWidth = menuAss.MeasureTextRubik(songi.title.c_str(), 24);
                songi.artistScrollTime = GetTime();
                songi.artistTextWidth =
                    menuAss.MeasureTextRubik(songi.artist.c_str(), 20);
            }
            TheMenuManager.SwitchScreen(SONG_SELECT);
        }
    } else {
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(Color { 128, 0, 0, 255 }));
        GuiButton(
            { u.wpct(0.02f), u.hpct(0.3f), u.winpct(0.2f), u.hinpct(0.08f) },
            "Invalid song cache!"
        );
        songList.ScanSongs(TheGameSettings.SongPaths);
        songsLoaded = false;
        DrawRectanglePro(
            { ((float)GetScreenWidth() / 2) - 125,
              ((float)GetScreenHeight() / 2) - 120,
              250,
              60 },
            { 0, 0 },
            0,
            Color { 0, 0, 0, 64 }
        );
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
    }
    if (GuiButton(
            { u.wpct(0.02f), u.hpct(0.39f), u.winpct(0.5), u.hinpct(0.08f) }, "Options - out of order"
        )) {
        // glfwSetGamepadStateCallback(gamepadStateCallbackSetControls);
        TheMenuManager.SwitchScreen(SETTINGS);
    }
    if (GuiButton(
            { u.wpct(0.02f), u.hpct(0.48f), u.winpct(0.2f), u.hinpct(0.08f) }, "Quit"
        )) {
        exit(0);
    }
    if (GuiButton(
            { u.wpct(0.8f), u.hpct(0.90f), u.winpct(0.5f), u.hinpct(0.05f) }, "Sound Test"
        )) {
        TheMenuManager.SwitchScreen(SOUND_TEST);
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
    GuiSetStyle(
        BUTTON,
        BASE_COLOR_FOCUSED,
        ColorToInt(ColorBrightness(Color { 255, 0, 255, 255 }, -0.5))
    );
    GuiSetStyle(
        BUTTON,
        BASE_COLOR_PRESSED,
        ColorToInt(ColorBrightness(Color { 255, 0, 255, 255 }, -0.3))
    );
    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, 0xFFFFFFFF);
    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, 0xFFFFFFFF);
    GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, 0xFFFFFFFF);
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0x505050ff);
    GuiSetStyle(BUTTON, BORDER_WIDTH, 2);

    if (GuiButton(
            { (float)GetScreenWidth() - 60,
              (float)GetScreenHeight() - u.hpct(0.15f) - 60,
              60,
              60 },
            ""
        )) {
        OpenURL("https://github.com/Encore-Developers/Encore");
    }

    if (GuiButton(
            { (float)GetScreenWidth() - 120,
              (float)GetScreenHeight() - u.hpct(0.15f) - 60,
              60,
              60 },
            ""
        )) {
        OpenURL("https://discord.gg/GhkgVUAC9v");
    }

    GuiSetStyle(DEFAULT, TEXT_SIZE, (int)u.hinpct(0.03f));
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    GuiSetFont(menuAss.rubik);
    DrawTextureEx(
        menuAss.github,
        { (float)GetScreenWidth() - 54, (float)GetScreenHeight() - 54 - u.hpct(0.15f) },
        0,
        0.2,
        WHITE
    );
    DrawTextureEx(
        menuAss.discord,
        { (float)GetScreenWidth() - 113, (float)GetScreenHeight() - 48 - u.hpct(0.15f) },
        0,
        0.075,
        WHITE
    );
    if (streamsLoaded) {
        Vector2 TitleSize = MeasureTextEx(
            menuAss.rubikBoldItalic, songList.curSong->title.c_str(), SongFontSize, 0
        );
        Vector2 ArtistSize = MeasureTextEx(
            menuAss.rubikItalic, songList.curSong->artist.c_str(), SongFontSize, 0
        );

        Vector2 SongTitleBox = { u.RightSide - TitleSize.x - u.winpct(0.01f),
                                 u.hpct(0.2f) - u.hinpct(0.1f) - (TitleSize.y * 1.1f) };
        Vector2 SongArtistBox = { u.RightSide - ArtistSize.x - u.winpct(0.01f),
                                  u.hpct(0.2f) - u.hinpct(0.1f) };
        SongTitleBox.x = SongTitleBox.x - u.hinpct(0.12f);
        SongArtistBox.x = SongArtistBox.x - u.hinpct(0.12f);
        DrawTextEx(
            menuAss.rubikBoldItalic,
            songList.curSong->title.c_str(),
            SongTitleBox,
            SongFontSize,
            0,
            WHITE
        );
        DrawTextEx(
            menuAss.rubikItalic,
            songList.curSong->artist.c_str(),
            SongArtistBox,
            SongFontSize,
            0,
            WHITE
        );

        for (auto &stream : TheAudioManager.loadedStreams) {
            TheAudioManager.SetAudioStreamVolume(
                stream.handle, TheGameSettings.avMainVolume * TheGameSettings.avMenuMusicVolume
            );
        }
        float played = TheAudioManager.GetMusicTimePlayed();
        float length = TheAudioManager.GetMusicTimeLength();
        DrawRectangle(
            0,
            u.hpct(0.2f) - u.hinpct(0.01f),
            Remap(played, 0, length, 0, GetScreenWidth()),
            u.hinpct(0.005f),
            SKYBLUE
        );

        if (played >= length - 0.5) {
            TheAudioManager.unloadStreams();
            albumArtLoaded = false;
            streamsPaused = false;
            songChosen = false;
            streamsLoaded = false;
            randomSongChosen = false;
        }
        GuiSetStyle(BUTTON, BORDER_WIDTH, 0);
        if (GuiButton(
                { u.RightSide - u.hinpct(0.12f),
                  u.hpct(0.2f) - u.hinpct(0.1f) - u.hinpct(0.031f),
                  u.hinpct(0.06f),
                  u.hinpct(0.06f) },
                streamsPaused ? ">" : "||"
            )) {
            if (!streamsPaused) {
                TheAudioManager.pauseStreams();
                streamsPaused = true;
            } else if (streamsPaused) {
                streamsPaused = false;
                TheAudioManager.playStreams();
            }
        }
        if (GuiButton(
                { u.RightSide - u.hinpct(0.06f),
                  u.hpct(0.2f) - u.hinpct(0.1f) - u.hinpct(0.031f),
                  u.hinpct(0.06f),
                  u.hinpct(0.06f) },
                ">>"
            )) {
            TheAudioManager.unloadStreams();
            albumArtLoaded = false;
            streamsPaused = false;
            songChosen = false;
            streamsLoaded = false;
            randomSongChosen = false;
        }
        GuiSetStyle(BUTTON, BORDER_WIDTH, 2);
    } else {
        Vector2 TitleSize =
            MeasureTextEx(menuAss.rubikBoldItalic, "No song loaded", SongFontSize, 0);
        Vector2 ArtistSize = MeasureTextEx(menuAss.rubikItalic, "", SongFontSize, 0);

        Vector2 SongTitleBox = { u.RightSide - TitleSize.x - u.winpct(0.01f),
                                 u.hpct(0.2f) - u.hinpct(0.1f) - (TitleSize.y * 1.1f) };
        Vector2 SongArtistBox = { u.RightSide - ArtistSize.x - u.winpct(0.01f),
                                  u.hpct(0.2f) - u.hinpct(0.1f) };
        DrawTextEx(
            menuAss.rubikBoldItalic,
            songList.curSong->title.c_str(),
            SongTitleBox,
            SongFontSize,
            0,
            WHITE
        );
        DrawTextEx(
            menuAss.rubikItalic,
            songList.curSong->artist.c_str(),
            SongArtistBox,
            SongFontSize,
            0,
            WHITE
        );
    }
    GameMenu::DrawBottomOvershell();
    DrawOvershell();
}

bool AlbumArtLoadingStuff = false;
// sentenced to

void GameMenu::DrawFPS(int posX, int posY) {
    Color color = LIME; // Good FPS
    int fps = GetFPS();

    if ((fps < 45) && (fps >= 15))
        color = ORANGE; // Warning FPS
    else if (fps < 20)
        color = RED; // Low FPS

    DrawTextEx(
        menuAss.josefinSansItalic,
        TextFormat("%2i FPS", fps),
        { (float)posX, (float)posY },
        u.hinpct(0.025f),
        0,
        color
    );
}

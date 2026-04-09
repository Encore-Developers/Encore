//
// Created by marie on 16/11/2024.
//

#include "raylib.h"
#include "SongSelectMenu.h"


#include "MenuManager.h"
#include "gameMenu.h"
#include "raygui.h"
#include "settings/settings.h"
#include "uiUnits.h"
#include "OvershellHelper.h"

#include "song/audio.h"
#include "song/songlist.h"
#include "assets.h"
#include <filesystem>
#include <string>
#include <set>
#include <vector>

float EaseInOutQuad(float t) {
    t = t < 0.5f ? 2.0f * t * t : 1.0f - powf(-2.0f * t + 2.0f, 2.0f) / 2.0f;
    return t;
}

SortType currentSortValue = SortType::Title;
Color AccentColor = {255, 0, 255, 255};
SongSelectMenu::~SongSelectMenu() {
    Unload();
}

void SongSelectMenu::Load() {
    // oh brother, this guy STINKS!
    //if (!IsAudioDeviceReady()) {
    //    InitAudioDevice();
    //    TraceLog(LOG_INFO, "Initialized audio device");
    //}
    previewStartTime = 0.0;
    phaseStartTime = 0.0;
    currentPreviewVolume = 0.0f;
    previewState = PreviewState::FadeIn;
    pendingSongID = -1;
    selectionTime = 0.0;

    if (TheSongList.curSong && !TheSongList.curSong->AlbumArtLoaded) {
        try {
            TheSongList.curSong->LoadAlbumArt();
            SetTextureWrap(TheSongList.curSong->albumArtBlur, TEXTURE_WRAP_REPEAT);
            SetTextureFilter(TheSongList.curSong->albumArtBlur, TEXTURE_FILTER_ANISOTROPIC_16X);
            TheSongList.curSong->AlbumArtLoaded = true;
            TraceLog(LOG_DEBUG, "Loaded album art for %s", TheSongList.curSong->title.c_str());
        } catch (const std::exception& e) {
            TraceLog(LOG_ERROR, "Failed to load album art for %s: %s", TheSongList.curSong->title.c_str(), e.what());
        }
    }

    if (TheSongList.curSong) {
        TheSongList.SongSelectOffset = TheSongList.curSong->songListPos - 5;
        if (TheSongList.SongSelectOffset < 1) TheSongList.SongSelectOffset = 1;
        if (TheSongList.SongSelectOffset > TheSongList.listMenuEntries.size() - 10)
            TheSongList.SongSelectOffset = TheSongList.listMenuEntries.size() - 10;
    } else {
        TheSongList.SongSelectOffset = 1;
    }

    // TheGameRenderer.streamsLoaded = false;
    // TheGameRenderer.midiLoaded = false;
    // for (Song& song : TheSongList.songs) {
    //     if (!song.ini) {
    //         // song.LoadSongJSON(song.songInfoPath);
    //     } else {
    //         song.LoadInfoINI(song.songInfoPath);
    //     }
    // }
}

void SongSelectMenu::Unload() {
    if (!TheAudioManager.loadedStreams.empty()) {
        for (auto& stream : TheAudioManager.loadedStreams) {
            TheAudioManager.StopPlayback(stream.handle);
        }
        TheAudioManager.loadedStreams.clear();
    }
}


void SongSelectMenu::UpdatePreviewVolume(double currentTime) {
    float targetVolume = TheGameSettings.avMainVolume * TheGameSettings.avMenuMusicVolume;
    float t;

    if (TheAudioManager.loadedStreams.empty()) {
        currentPreviewVolume = 0.0f;
        return;
    }

    switch (previewState) {
        case PreviewState::FadeIn:
            t = (currentTime - phaseStartTime) / 1.0f;
            if (t >= 1.0f) {
                currentPreviewVolume = targetVolume;
                previewState = PreviewState::Playing;
                phaseStartTime = currentTime;
            } else {
                t = EaseInOutQuad(t);
                currentPreviewVolume = t * targetVolume;
            }
            break;
        case PreviewState::Playing:
            currentPreviewVolume = targetVolume;
            if (currentTime - phaseStartTime >= previewPlayDuration) {
                previewState = PreviewState::FadeOut;
                phaseStartTime = currentTime;
            }
            break;
        case PreviewState::FadeOut:
            t = (currentTime - phaseStartTime) / fadeDuration;
            if (t >= 1.0f) {
                currentPreviewVolume = 0.0f;
                previewState = PreviewState::Pause;
                phaseStartTime = currentTime;
            } else {
                t = EaseInOutQuad(t);
                currentPreviewVolume = (1.0f - t) * targetVolume;
            }
            break;
        case PreviewState::Pause:
            currentPreviewVolume = 0.0f;
            if (currentTime - phaseStartTime >= pauseDuration) {
                previewState = PreviewState::FadeIn;
                phaseStartTime = currentTime;
                if (TheSongList.curSong && !TheAudioManager.loadedStreams.empty()) {
                    float previewStartTimeSec = TheSongList.curSong->previewStartTime / 1000.0f;
                    TheAudioManager.seekStreams(previewStartTimeSec);
                    for (auto& stream : TheAudioManager.loadedStreams) {
                        TheAudioManager.BeginPlayback(stream.handle);
                    }
                }
            }
            break;
    }

    for (int i = 0; i < TheAudioManager.loadedStreams.size(); i++) {
        float volume = currentPreviewVolume;
        if (i == PartVocals) volume = 0;
        TheAudioManager.SetAudioStreamVolume(TheAudioManager.loadedStreams[i].handle, volume);
    }
}


void SongSelectMenu::KeyboardInputCallback(int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS && key == GLFW_KEY_UP) {
        if (TheSongList.SongSelectOffset <= TheSongList.listMenuEntries.size() && TheSongList.SongSelectOffset >= 1
            && TheSongList.listMenuEntries.size() >= 10) {
            TheSongList.SongSelectOffset--;
            }

        // prevent going past top
        if (TheSongList.SongSelectOffset < 1)
            TheSongList.SongSelectOffset = 1;

        // prevent going past bottom
        if (TheSongList.SongSelectOffset >= TheSongList.listMenuEntries.size() - 10)
            TheSongList.SongSelectOffset = TheSongList.listMenuEntries.size() - 10;
    }
}
void SongSelectMenu::ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event) {}
void SongSelectMenu::Draw() {
    Assets &assets = Assets::getInstance();
    Units u = Units::getInstance();

    double curTime = GetTime();
    // -5 -4 -3 -2 -1 0 1 2 3 4 5 6
    if (pendingSongID >= 0 && curTime - selectionTime >= 0.75) {
        if (pendingSongID < TheSongList.songs.size()) {
            try {
                TheAudioManager.loadStreams(TheSongList.songs[pendingSongID].stemsPath);
                float previewStartTimeSec = TheSongList.songs[pendingSongID].previewStartTime / 1000.0f;
                TheAudioManager.seekStreams(previewStartTimeSec);
                for (int j = 0; j < TheAudioManager.loadedStreams.size(); j++) {
                    float volume = 0.0f;
                    if (j == PartVocals) volume = 0;
                    TheAudioManager.SetAudioStreamVolume(TheAudioManager.loadedStreams[j].handle, volume);
                    TheAudioManager.BeginPlayback(TheAudioManager.loadedStreams[j].handle);
                }
                previewStartTime = curTime;
                phaseStartTime = curTime;
                currentPreviewVolume = 0.0f;
                previewState = PreviewState::FadeIn;
            } catch (const std::exception& e) {
                TraceLog(LOG_ERROR, "Failed to load preview audio for song %d: %s", pendingSongID, e.what());
            }
        }
        pendingSongID = -1;
    }

    UpdatePreviewVolume(curTime);

    Vector2 mouseWheel = GetMouseWheelMoveV();
    // Update song select offset based on mouse wheel
    if (TheSongList.SongSelectOffset <= TheSongList.listMenuEntries.size() && TheSongList.SongSelectOffset >= 1
        && TheSongList.listMenuEntries.size() >= 10) {
        TheSongList.SongSelectOffset -= (int)mouseWheel.y;
    }

    // prevent going past top
    if (TheSongList.SongSelectOffset < 1)
        TheSongList.SongSelectOffset = 1;

    // prevent going past bottom
    if (TheSongList.SongSelectOffset >= TheSongList.listMenuEntries.size() - 10)
        TheSongList.SongSelectOffset = TheSongList.listMenuEntries.size() - 10;

    // todo(3drosalia): clean this shit up after changing it
    static Song blankSong = Song();
    Song *SongToDisplayInfo = TheSongList.curSong ? TheSongList.curSong : &blankSong;

    BeginDrawing();
    ClearBackground(DARKGRAY);
    if (TheSongList.curSong && TheSongList.curSong->AlbumArtLoaded) {
        BeginShaderMode(assets.bgShader);
        GameMenu::DrawAlbumArtBackground(TheSongList.curSong->albumArtBlur);
        EndShaderMode();
    }

    float TopOvershell = u.hpct(0.15f);
    DrawRectangle(
        0, 0, u.RightSide - u.LeftSide, (float)GetRenderHeight(), GetColor(0x00000080)
    );
    BeginScissorMode(0, u.hpct(0.15f), u.RightSide - u.winpct(0.25f), u.hinpct(0.7f));
    GameMenu::DrawTopOvershell(0.208333f);
    EndScissorMode();
    encOS::DrawTopOvershell(0.15f);

    GameMenu::DrawVersion();
    int AlbumX = u.RightSide - u.winpct(0.25f);
    int AlbumY = u.hpct(0.075f);
    int AlbumHeight = u.winpct(0.25f);
    int AlbumOuter = u.hinpct(0.01f);
    int AlbumInner = u.hinpct(0.005f);

    DrawTextEx(
        assets.josefinSansItalic,
        TextFormat("Sorted by: %s", sortTypes[(int)currentSortValue].c_str()),
        { u.LeftSide, u.hinpct(0.165f) },
        u.hinpct(0.03f),
        0,
        WHITE
    );
    DrawTextEx(
        assets.josefinSansItalic,
        TextFormat("Songs loaded: %01i", TheSongList.songs.size()),
        { AlbumX - (AlbumOuter * 2)
              - MeasureTextEx(
                    assets.josefinSansItalic,
                    TextFormat("Songs loaded: %01i", TheSongList.songs.size()),
                    u.hinpct(0.03f),
                    0
              ).x,
          u.hinpct(0.165f) },
        u.hinpct(0.03f),
        0,
        WHITE
    );

    float songEntryHeight = u.hinpct(0.058333f);
    DrawRectangle(
        0,
        u.hinpct(0.208333f),
        (u.RightSide - u.winpct(0.25f)),
        songEntryHeight,
        ColorBrightness(AccentColor, -0.75f)
    );

    for (int j = 0; j < 5; j++) {
        DrawRectangle(
            0,
            ((songEntryHeight * 2) * j) + u.hinpct(0.208333f) + songEntryHeight,
            (u.RightSide - u.winpct(0.25f)),
            songEntryHeight,
            Color { 0, 0, 0, 64 }
        );
    }

    for (int i = TheSongList.SongSelectOffset;
         i < TheSongList.listMenuEntries.size() && i < TheSongList.SongSelectOffset + 10;
         i++) {
        if (TheSongList.listMenuEntries.size() == i)
            break;
        if (TheSongList.listMenuEntries[i].isHeader) {
            float songXPos = u.LeftSide + u.winpct(0.005f) - 2;
            float songYPos = std::floor(
                (u.hpct(0.266666f)) + ((songEntryHeight) * ((i - TheSongList.SongSelectOffset)))
            );
            DrawRectangle(
                0,
                songYPos,
                (u.RightSide - u.winpct(0.25f)),
                songEntryHeight,
                ColorBrightness(AccentColor, -0.75f)
            );

            DrawTextEx(
                assets.rubikBold,
                TheSongList.listMenuEntries[i].headerChar.c_str(),
                { songXPos, songYPos + u.hinpct(0.0125f) },
                u.hinpct(0.035f),
                0,
                WHITE
            );
        } else if (!TheSongList.listMenuEntries[i].hiddenEntry) {
            bool isCurSong = TheSongList.curSong && i == TheSongList.curSong->songListPos - 1;
            Font artistFont = assets.josefinSansItalic;
            Song &songi = TheSongList.songs[TheSongList.listMenuEntries[i].songListID];
            int songID = TheSongList.listMenuEntries[i].songListID;

            float songXPos = u.LeftSide + u.winpct(0.005f) - 2;
            float songYPos = std::floor(
                (u.hpct(0.266666f)) + ((songEntryHeight) * ((i - TheSongList.SongSelectOffset)))
            );
            GuiSetStyle(BUTTON, BORDER_WIDTH, 0);
            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0);
            if (isCurSong) {
                GuiSetStyle(
                    BUTTON,
                    BASE_COLOR_NORMAL,
                    ColorToInt(ColorBrightness(AccentColor, -0.4))
                );
            }
            BeginScissorMode(
                0, u.hpct(0.15f), u.RightSide - u.winpct(0.25f), u.hinpct(0.7f)
            );
            if (GuiButton(
                    Rectangle {
                        0, songYPos, (u.RightSide - u.winpct(0.25f)), songEntryHeight },
                    ""
                )) {
                TheSongList.curSong = &TheSongList.songs[songID];

                if (!TheAudioManager.loadedStreams.empty()) {
                    for (auto& stream : TheAudioManager.loadedStreams) {
                        TheAudioManager.StopPlayback(stream.handle);
                    }
                    TheAudioManager.loadedStreams.clear();
                    currentPreviewVolume = 0.0f;
                    previewState = PreviewState::FadeIn;
                }
                pendingSongID = songID;
                selectionTime = curTime;

                if (!TheSongList.songs[songID].AlbumArtLoaded) {
                    try {
                        TheSongList.songs[songID].LoadAlbumArt();
                        TheSongList.songs[songID].AlbumArtLoaded = true;
                    } catch (const std::exception &e) {
                        Encore::EncoreLog(LOG_ERROR, e.what());
                    }
                }
            }
            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
            EndScissorMode();

            int songTitleWidth = (u.winpct(0.3f)) - 6;
            int songArtistWidth = (u.winpct(0.5f)) - 6;

            if (songi.titleTextWidth >= (float)songTitleWidth) {
                if (curTime > songi.titleScrollTime
                    && curTime < songi.titleScrollTime + 3.0)
                    songi.titleXOffset = 0;

                if (curTime > songi.titleScrollTime + 3.0) {
                    songi.titleXOffset -= 1;
                    if (songi.titleXOffset
                        < -(songi.titleTextWidth - (float)songTitleWidth)) {
                        songi.titleXOffset = -(songi.titleTextWidth - (float)songTitleWidth);
                        songi.titleScrollTime = curTime + 3.0;
                    }
                }
            }
            auto LightText = Color { 203, 203, 203, 255 };
            BeginScissorMode(
                (int)songXPos + (isCurSong ? 5 : 20),
                (int)songYPos,
                songTitleWidth,
                songEntryHeight
            );
            DrawTextEx(
                assets.rubikBold,
                songi.title.c_str(),
                { songXPos + songi.titleXOffset + (isCurSong ? 10 : 20),
                  songYPos + u.hinpct(0.0125f) },
                u.hinpct(0.035f),
                0,
                isCurSong ? WHITE : LightText
            );
            EndScissorMode();

            if (songi.artistTextWidth > (float)songArtistWidth) {
                if (curTime > songi.artistScrollTime
                    && curTime < songi.artistScrollTime + 3.0)
                    songi.artistXOffset = 0;

                if (curTime > songi.artistScrollTime + 3.0) {
                    songi.artistXOffset -= 1;
                    if (songi.artistXOffset
                        < -(songi.artistTextWidth - (float)songArtistWidth)) {
                        songi.artistScrollTime = curTime + 3.0;
                    }
                }
            }

            BeginScissorMode(
                (int)songXPos + 30 + (int)songTitleWidth,
                (int)songYPos,
                songArtistWidth,
                songEntryHeight
            );
            DrawTextEx(
                artistFont,
                songi.artist.c_str(),
                { songXPos + 30 + (float)songTitleWidth + songi.artistXOffset,
                  songYPos + u.hinpct(0.02f) },
                u.hinpct(0.025f),
                0,
                isCurSong ? WHITE : LightText
            );
            EndScissorMode();
        }
    }
    if (TheSongList.SongSelectOffset > 0 && TheSongList.SongSelectOffset < TheSongList.listMenuEntries.size()) {
        std::string categoryHeaderText = "";
        int songIndex = TheSongList.SongSelectOffset;

        if (TheSongList.listMenuEntries[songIndex].isHeader && songIndex > 0 && !TheSongList.listMenuEntries[songIndex - 1].isHeader) {
            songIndex--;
        } else if (!TheSongList.listMenuEntries[songIndex].isHeader) {
        } else if (songIndex + 1 < TheSongList.listMenuEntries.size() && !TheSongList.listMenuEntries[songIndex + 1].isHeader) {
            songIndex++;
        }

        if (songIndex < TheSongList.listMenuEntries.size() && !TheSongList.listMenuEntries[songIndex].isHeader) {
            Song& representativeSong = TheSongList.songs[TheSongList.listMenuEntries[songIndex].songListID];
            switch (currentSortValue) {
                case SortType::Title:
                    categoryHeaderText = representativeSong.title.empty() ? "#" : std::string(1, toupper(representativeSong.title[0]));
                    break;
                case SortType::Artist:
                    categoryHeaderText = representativeSong.artist.empty() ? "#" : std::string(1, toupper(representativeSong.artist[0]));
                    break;
                case SortType::Source:
                    categoryHeaderText = representativeSong.source.empty() ? "Unknown" : representativeSong.source;
                    break;
                case SortType::Length:
                    categoryHeaderText = TheSongList.listMenuEntries[TheSongList.SongSelectOffset].headerChar;
                    break;
                case SortType::Year:
                    categoryHeaderText = representativeSong.releaseYear.empty() ? "Unknown Year" : representativeSong.releaseYear;
                    break;
                default:
                    categoryHeaderText = "";
                    break;
            }
        }

        if (categoryHeaderText.empty()) {
            categoryHeaderText = TheSongList.listMenuEntries[TheSongList.SongSelectOffset].headerChar;
        }

        DrawTextEx(
            assets.rubikBold,
            categoryHeaderText.c_str(),
            { u.LeftSide + 5, u.hpct(0.218333f) },
            u.hinpct(0.035f),
            0,
            WHITE
        );
    }

    DrawRectangle(
        AlbumX - AlbumOuter,
        AlbumY + AlbumHeight,
        AlbumHeight + AlbumOuter,
        AlbumHeight + u.hinpct(0.01f),
        WHITE
    );
    DrawRectangle(
        AlbumX - AlbumInner,
        AlbumY + AlbumHeight,
        AlbumHeight,
        u.hinpct(0.075f) + AlbumHeight,
        GetColor(0x181827FF)
    );
    DrawRectangle(
        AlbumX - AlbumOuter,
        AlbumY - AlbumInner,
        AlbumHeight + AlbumOuter,
        AlbumHeight + AlbumOuter,
        WHITE
    );
    DrawRectangle(AlbumX - AlbumInner, AlbumY, AlbumHeight, AlbumHeight, BLACK);

    std::string titleText = SongToDisplayInfo->source.empty() ? "Custom" : SongToDisplayInfo->source;
    float titleFontSize = u.hinpct(0.035f);
    float titleTextWidth = MeasureTextEx(assets.rubikBold, titleText.c_str(), titleFontSize, 0).x;
    float titleTextX = AlbumX - AlbumInner + (AlbumHeight / 2.0f) - (titleTextWidth / 2.0f);
    float titleTextY = AlbumY - u.hinpct(0.045f);
    DrawTextEx(
        assets.rubikBold,
        titleText.c_str(),
        { titleTextX, titleTextY },
        titleFontSize,
        0,
        WHITE
    );

    if (TheSongList.curSong && TheSongList.curSong->AlbumArtLoaded) {
        DrawTexturePro(
            TheSongList.curSong->albumArt,
            Rectangle { 0,
                        0,
                        (float)TheSongList.curSong->albumArt.width,
                        (float)TheSongList.curSong->albumArt.width },
            Rectangle { (float)AlbumX - AlbumInner,
                        (float)AlbumY,
                        (float)AlbumHeight,
                        (float)AlbumHeight },
            { 0, 0 },
            0,
            WHITE
        );
    } else {
        DrawRectangle(AlbumX - AlbumInner, AlbumY, AlbumHeight, AlbumHeight, DARKGRAY);
    }

    float TextPlacementTB = u.hpct(0.05f);
    float TextPlacementLR = u.LeftSide;
    GameMenu::mhDrawText(assets.redHatDisplayBlack, "MUSIC LIBRARY", { TextPlacementLR, TextPlacementTB }, u.hinpct(0.125f), WHITE, assets.sdfShader, LEFT);

    std::string albumText = SongToDisplayInfo->album.empty() ? "No Album Listed" : SongToDisplayInfo->album;
    std::string yearText = SongToDisplayInfo->releaseYear.empty() ? "Unknown Year" : SongToDisplayInfo->releaseYear;
    std::string albumDisplayText = albumText + " | " + yearText;
    float albumTextHeight = MeasureTextEx(assets.rubikBold, albumDisplayText.c_str(), u.hinpct(0.035f), 0).y;
    float albumTextWidth = MeasureTextEx(assets.rubikBold, albumDisplayText.c_str(), u.hinpct(0.035f), 0).x;
    float albumNameTextCenter = u.RightSide - u.winpct(0.125f) - AlbumInner;
    float albumTTop = AlbumY + AlbumHeight + u.hinpct(0.011f);
    float albumNameFontSize = albumTextWidth <= u.winpct(0.25f) ? u.hinpct(0.035f) : u.winpct(0.23f) / (albumTextWidth / albumTextHeight);
    float albumNameLeft = albumNameTextCenter - (MeasureTextEx(assets.rubikBold, albumDisplayText.c_str(), albumNameFontSize, 0).x / 2);
    float albumNameTextTop = albumTextWidth <= u.winpct(0.25f) ? albumTTop : albumTTop + ((u.hinpct(0.035f) / 2) - (albumNameFontSize / 2));
    DrawTextEx(assets.rubikBold, albumDisplayText.c_str(), { albumNameLeft, albumNameTextTop }, albumNameFontSize, 0, WHITE);

    DrawLine(u.RightSide - AlbumHeight - AlbumOuter, AlbumY + AlbumHeight + AlbumOuter + (u.hinpct(0.04f)), u.RightSide, AlbumY + AlbumHeight + AlbumOuter + (u.hinpct(0.04f)), WHITE);

    float DiffTop = AlbumY + AlbumHeight + AlbumOuter + (u.hinpct(0.045f));
    float IconWidth = float(AlbumHeight - AlbumOuter) / 5.0f;
    GameMenu::mhDrawText(assets.rubikItalic, "Pad", { (u.RightSide - AlbumHeight + AlbumInner), DiffTop }, AlbumOuter * 3, WHITE, assets.sdfShader, LEFT);
    GameMenu::mhDrawText(assets.rubikItalic, "Classic", { (u.RightSide - AlbumHeight + AlbumInner), DiffTop + IconWidth + (AlbumOuter * 3) }, AlbumOuter * 3, WHITE, assets.sdfShader, LEFT);
    for (int i = 0; i < 10; i++) {
        bool RowTwo = i < 5;
        int RowTwoInt = i - 5;
        float PosTopAddition = RowTwo ? AlbumOuter * 3 : AlbumOuter * 6;
        float BoxTopPos = DiffTop + PosTopAddition + float(IconWidth * (RowTwo ? 0 : 1));
        float ResetToLeftPos = (float)(RowTwo ? i : RowTwoInt);
        int asdasd = (float)(RowTwo ? i : RowTwoInt);
        float IconLeftPos = (float)(u.RightSide - AlbumHeight) + IconWidth * ResetToLeftPos;
        Rectangle Placement = { IconLeftPos, BoxTopPos, IconWidth, IconWidth };
        Color TintColor = WHITE;
        int diffNumber = SongToDisplayInfo->parts[i]->diff;
        if (SongToDisplayInfo->parts[i] && diffNumber == -1) TintColor = DARKGRAY;
        auto instIcon = assets.InstIcons[asdasd];
        DrawTexturePro(*instIcon, { 0, 0, (float)instIcon->width, (float)instIcon->height }, Placement, { 0, 0 }, 0, TintColor);
        DrawTexturePro(assets.BaseRingTexture, { 0, 0, (float)assets.BaseRingTexture.width, (float)assets.BaseRingTexture.height }, Placement, { 0, 0 }, 0, ColorBrightness(WHITE, 2));
        if (SongToDisplayInfo->parts[i] && diffNumber > 0) {
            if (diffNumber > 6) {
                diffNumber = 6;
            }
            auto ring = assets.YargRings[diffNumber - 1];
            DrawTexturePro(
                *ring,
                { 0, 0, (float)ring->width, (float)ring->height },
                Placement,
                { 0, 0 },
                0,
                WHITE
            );
        }
    }

    GameMenu::DrawBottomOvershell();
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(ColorBrightness(AccentColor, -0.25)));
    if (GuiButton(Rectangle{ u.LeftSide, GetRenderHeight() - u.hpct(0.1475f), u.winpct(0.2f), u.hinpct(0.05f) }, "Play Song")) {
        if (TheSongList.curSong) {
            Unload();
            TheSongList.curSong->LoadSongIni(TheSongList.curSong->songDir);
            TheMenuManager.SwitchScreen(READY_UP);
        }
    }
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);

    if (GuiButton(
            Rectangle { u.LeftSide + u.winpct(0.4f) - 2,
                        GetRenderHeight() - u.hpct(0.1475f),
                        u.winpct(0.2f),
                        u.hinpct(0.05f) },
            "Sort"
        )) {
        int selectedSongIndex = -1;
        if (TheSongList.curSong) {
            for (size_t i = 0; i < TheSongList.songs.size(); i++) {
                if (&TheSongList.songs[i] == TheSongList.curSong) {
                    selectedSongIndex = i;
                    break;
                }
            }
        }
        currentSortValue = NextSortType(currentSortValue);
        TheSongList.sortList(currentSortValue, selectedSongIndex);
        if (selectedSongIndex >= 0 && selectedSongIndex < TheSongList.songs.size()) {
            TheSongList.curSong = &TheSongList.songs[selectedSongIndex];
            TheSongList.SongSelectOffset = TheSongList.curSong->songListPos - 5;
            if (TheSongList.SongSelectOffset < 1) TheSongList.SongSelectOffset = 1;
            if (TheSongList.SongSelectOffset > TheSongList.listMenuEntries.size() - 10)
                TheSongList.SongSelectOffset = TheSongList.listMenuEntries.size() - 10;
            if (!TheAudioManager.loadedStreams.empty()) {
                for (auto& stream : TheAudioManager.loadedStreams) {
                    TheAudioManager.StopPlayback(stream.handle);
                }
                TheAudioManager.loadedStreams.clear();
                currentPreviewVolume = 0.0f;
                previewState = PreviewState::FadeIn;
            }
            pendingSongID = selectedSongIndex;
            selectionTime = curTime;
        }
    }
    if (GuiButton(Rectangle{ u.LeftSide + u.winpct(0.2f) - 1, GetRenderHeight() - u.hpct(0.1475f), u.winpct(0.2f), u.hinpct(0.05f) }, "Back")) {
        if (!TheAudioManager.loadedStreams.empty()) {
            for (auto& stream : TheAudioManager.loadedStreams) {
                TheAudioManager.StopPlayback(stream.handle);
            }
            TheAudioManager.loadedStreams.clear();
        }
        Unload();
        TheMenuManager.SwitchScreen(MAIN_MENU);
    }
    DrawOvershell();
}
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
#include "imgui.h"
#include "raymath.h"
#include "song/ArtLoader.h"
#include "song/OpenSource.h"

#include <filesystem>
#include <string>
#include <set>
#include <vector>

float EaseInOutQuad(float t) {
    t = t < 0.5f ? 2.0f * t * t : 1.0f - powf(-2.0f * t + 2.0f, 2.0f) / 2.0f;
    return t;
}

SortType currentSortValue = SortType::Title;
Color AccentColor = { 255, 0, 255, 255 };

SongSelectMenu::~SongSelectMenu() {
    Unload();
}

void SongSelectMenu::ScrollUpHeader() {
    for (int sectInt = 0; sectInt < TheSongList.sectionEntries.size() - 1; sectInt++) {
        auto sect = TheSongList.sectionEntries[sectInt];
        if (curSongMenuPos >= sect.firstListID && curSongMenuPos <= sect.lastListID) {
            if (sectInt > 1) {
                curSongMenuPos = TheSongList.sectionEntries[sectInt - 1].firstListID;
            } else {
                curSongMenuPos = 1;
            }
            StopPreview();
            break;
        }
    }
}

void SongSelectMenu::ScrollDownHeader() {
    for (int sectInt = 0; sectInt < TheSongList.sectionEntries.size() - 1; sectInt++) {
        auto sect = TheSongList.sectionEntries[sectInt];
        if (curSongMenuPos >= sect.firstListID && curSongMenuPos <= sect.lastListID &&
            sectInt < TheSongList.sectionEntries.size() - 2) {
            curSongMenuPos = TheSongList.sectionEntries[sectInt + 1].firstListID;
            StopPreview();
            break;
        }
    }
}

void SongSelectMenu::ScrollSongSelect(int val) {
    auto oldPos = curSongMenuPos;
    // Dealing with the logic of current pos not being signed is annoying. just store it
    // as signed temporarily
    int newPos = curSongMenuPos;
    if (newPos <= TheSongList.listMenuEntries.size()) {
        newPos -= val;
    }

    // prevent going past top
    if (newPos < 1)
        newPos = 1;

    // prevent going past bottom
    if (newPos >= TheSongList.listMenuEntries.size())
        newPos = TheSongList.listMenuEntries.size() - 1;

    if (oldPos != newPos && !TheSongList.listMenuEntries[newPos].isHeader) {
        TheSongList.curSong = TheSongList.sortedSongs[TheSongList.listMenuEntries[newPos].
            songListID];

        StopPreview();
        currentPreviewVolume = 0.0f;
        previewState = PreviewState::Hysteresis;
        selectionTime = curTime;

        TheSongList.sortedSongs[TheSongList.listMenuEntries[newPos].songListID]->LoadAlbumArt();
    }
    if (TheSongList.listMenuEntries[newPos].isHeader) {
        StopPreview();
    }
    curSongMenuPos = newPos;
}

void SongSelectMenu::ControllerInputCallback(
    Encore::RhythmEngine::ControllerEvent event) {
    int curSlot = 0;
    if (ThePlayerManager.GetPlayerForJoystick(event.slot)) {
        curSlot = ThePlayerManager.GetPlayerForJoystick(event.slot)->ActiveSlot;
    }
    if (event.action == Encore::RhythmEngine::Action::PRESS) {
        switch (event.channel) {
        case Encore::RhythmEngine::InputChannel::STRUM_UP:
            if (ControllerOrangeHeld.at(curSlot)) {
                ScrollUpHeader();
            } else {
                ScrollSongSelect(1);
            }
            break;
        case Encore::RhythmEngine::InputChannel::STRUM_DOWN:
            if (ControllerOrangeHeld.at(curSlot)) {
                ScrollDownHeader();
            } else {
                ScrollSongSelect(-1);
            }
            break;
        case Encore::RhythmEngine::InputChannel::INPUT_LEFT:
            ScrollSongSelect(5);
            break;
        case Encore::RhythmEngine::InputChannel::INPUT_RIGHT:
            ScrollSongSelect(-5);
            break;
        case Encore::RhythmEngine::InputChannel::LANE_1:
            if (TheSongList.curSong) {
                Unload();
                //TheSongList.curSong->LoadSongIni(TheSongList.curSong->songDir);
                TheMenuManager.SwitchScreen(READY_UP);
            }
            break;
        case Encore::RhythmEngine::InputChannel::LANE_2:
            if (TheSongList.curSong) {
                Unload();
                TheMenuManager.SwitchScreen(MAIN_MENU);
            }
            break;
        case Encore::RhythmEngine::InputChannel::LANE_5:
            ControllerOrangeHeld.at(curSlot) = true;
            break;
        default:
            break;
        }
    }
    if (event.action == Encore::RhythmEngine::Action::RELEASE) {
        switch (event.channel) {
        case Encore::RhythmEngine::InputChannel::LANE_5:
            ControllerOrangeHeld.at(curSlot) = false;
            break;
        }
    }
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
    previewState = PreviewState::Hysteresis;
    selectionTime = 0.0;

    TheSongList.curSong->LoadAlbumArt();

    if (TheSongList.curSong) {
        // TODO: should probably fix the actual value here. Just subtracting 1 for now.
        curSongMenuPos = TheSongList.curSong->songListPos - 1;
        if (curSongMenuPos < 1)
            curSongMenuPos = 1;
        if (curSongMenuPos > TheSongList.listMenuEntries.size())
            curSongMenuPos = TheSongList.listMenuEntries.size() - 1;
    } else {
        curSongMenuPos = 1;
    }

    // TheGameRenderer.streamsLoaded = false;
    // TheGameRenderer.midiLoaded = false;
    // for (Song& song : TheSongList.sortedSongs) {
    //     if (!song.ini) {
    //         // song.LoadSongJSON(song.songInfoPath);
    //     } else {
    //         song.LoadInfoINI(song.songInfoPath);
    //     }
    // }
}

void SongSelectMenu::StopPreview() {
    if (!TheAudioManager.loadedStreams.empty()) {
        for (auto &stream : TheAudioManager.loadedStreams) {
            TheAudioManager.StopPlayback(stream.handle);
        }
        TheAudioManager.loadedStreams.clear();
    }
}

void SongSelectMenu::Unload() {
    StopPreview();
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
                float previewStartTimeSec = TheSongList.curSong->previewStartTime /
                    1000.0f;
                TheAudioManager.seekStreams(previewStartTimeSec);
                for (auto &stream : TheAudioManager.loadedStreams) {
                    TheAudioManager.BeginPlayback(stream.handle);
                }
            }
        }
        break;
    }

    for (int i = 0; i < TheAudioManager.loadedStreams.size(); i++) {
        float volume = currentPreviewVolume;
        if (i == PartVocals)
            volume = 0;
        TheAudioManager.SetAudioStreamVolume(TheAudioManager.loadedStreams[i].handle,
                                             volume);
    }
}


void SongSelectMenu::KeyboardInputCallback(int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
        case GLFW_KEY_UP:
            ScrollSongSelect(1);
            break;
        case GLFW_KEY_DOWN:
            ScrollSongSelect(-1);
            break;
        case GLFW_KEY_LEFT:
            ScrollSongSelect(5);
            break;
        case GLFW_KEY_RIGHT:
            ScrollSongSelect(-5);
            break;
        case GLFW_KEY_PAGE_UP:
            ScrollUpHeader();
            break;
        case GLFW_KEY_PAGE_DOWN:
            ScrollDownHeader();
            break;
        default:
            return;
        }
    }
}

void SongSelectMenu::LoadPreview(Song &song) {
    ZoneScoped
    try {
        TheAudioManager.loadStreams(
            song
            .LoadAudioINI()
        );
        float previewStartTimeSec =
            song
            .previewStartTime
            / 1000.0f;
        TheAudioManager.seekStreams(previewStartTimeSec);
        for (int j = 0; j < TheAudioManager.loadedStreams.size(); j++) {
            float volume = 0.0f;
            if (j == PartVocals)
                volume = 0;
            TheAudioManager.SetAudioStreamVolume(
                TheAudioManager.loadedStreams[j].handle,
                volume
            );
            TheAudioManager.BeginPlayback(TheAudioManager.loadedStreams[j].handle);
        }
        previewStartTime = curTime;
        phaseStartTime = curTime;
        currentPreviewVolume = 0.0f;
        previewState = PreviewState::FadeIn;
    } catch (const std::exception &e) {
        TraceLog(
            LOG_ERROR,
            "Failed to load preview audio for song %d: %s",
            TheSongList.listMenuEntries[curSongMenuPos].songListID,
            e.what()
        );
        previewState = PreviewState::Failed;
    }
}

void SongSelectMenu::Draw() {
    Assets &assets = Assets::getInstance();
    Units u = Units::getInstance();

    Vector2 mouseWheel = GetMouseWheelMoveV();
    // Update song select offset based on mouse wheel
    ScrollSongSelect(mouseWheel.y);

    curTime = GetTime();
    // -5 -4 -3 -2 -1 0 1 2 3 4 5 6
    if (previewState == PreviewState::Hysteresis) {
        if (!TheSongList.listMenuEntries[curSongMenuPos].isHeader) {
            if (TheSongList.listMenuEntries[curSongMenuPos].songListID >= 0 && curTime -
                selectionTime >= 0.75) {
                if (TheSongList.listMenuEntries[curSongMenuPos].songListID < TheSongList.sortedSongs
                    .size()) {
                    LoadPreview(
                        *TheSongList.sortedSongs[TheSongList.listMenuEntries[curSongMenuPos].
                            songListID]);
                    }
                }
        }
    }

    UpdatePreviewVolume(curTime);

    // todo(3drosalia): clean this shit up after changing it
    static Song blankSong = Song();
    Song *SongToDisplayInfo = TheSongList.curSong ? TheSongList.curSong : &blankSong;

    BeginDrawing();
    ClearBackground(DARKGRAY);
    GameMenu::DrawAlbumArtBackground();

    float TopOvershell = u.hpct(0.15f);
    DrawRectangle(
        0,
        0,
        u.RightSide - u.LeftSide,
        (float)GetRenderHeight(),
        GetColor(0x00000080)
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

    GameMenu::mhDrawText(
        assets.josefinSansItalic,
        TextFormat("Sorted by: %s", sortTypes[(int)currentSortValue].c_str()),
        { u.LeftSide, u.hinpct(0.165f) },
        u.hinpct(0.03f),
        WHITE,
        ASSET(sdfShader),
        LEFT
    );
    GameMenu::mhDrawText(
        assets.josefinSansItalic,
        TextFormat("Songs loaded: %01i", TheSongList.sortedSongs.size()),
        { AlbumX - (AlbumOuter * 2)
          - MeasureTextEx(
              assets.josefinSansItalic,
              TextFormat("Songs loaded: %01i", TheSongList.sortedSongs.size()),
              u.hinpct(0.03f),
              0
          ).x,
          u.hinpct(0.165f) },
        u.hinpct(0.03f),
        WHITE,
        ASSET(sdfShader),
        LEFT
    );

    float songEntryHeight = u.hinpct(0.058333f);
    DrawRectangle(
        0,
        u.hinpct(0.208333f),
        (u.RightSide - u.winpct(0.25f)),
        songEntryHeight,
        ColorBrightness(AccentColor, -0.75f)
    );
    int topOflistMenu = curSongMenuPos <= 4 ? 1 : curSongMenuPos - 4;
    for (size_t listMenuPos = topOflistMenu;
         listMenuPos < TheSongList.listMenuEntries.size() &&
         (listMenuPos < curSongMenuPos + (topOflistMenu <= 4 ? 10 : 6));
         listMenuPos++) {
        ZoneScopedN("Draw Entry")
        if (TheSongList.listMenuEntries.size() == listMenuPos)
            break;
        if (TheSongList.listMenuEntries[listMenuPos].isHeader) {
            float songXPos = u.LeftSide + u.winpct(0.005f) - 2;
            float songYPos = std::floor(
                (u.hpct(0.266666f)) + (songEntryHeight * (listMenuPos - topOflistMenu))
            );
            Color headerColor = ColorBrightness(AccentColor, -0.75f);
            FontAsset *headerFont = ASSETPTR(rubikBold);
            if (listMenuPos == curSongMenuPos) {
                headerColor = ColorBrightness(AccentColor, -0.4f);
                headerFont = ASSETPTR(rubikBoldItalic);
            }
            DrawRectangle(
                0,
                songYPos,
                (u.RightSide - u.winpct(0.25f)),
                songEntryHeight,
                headerColor
            );

            GameMenu::mhDrawText(
                headerFont->Fetch(),
                TheSongList.listMenuEntries[listMenuPos].headerChar.c_str(),
                { songXPos, songYPos + u.hinpct(0.0125f) },
                u.hinpct(0.035f),
                WHITE,
                ASSET(sdfShader),
                LEFT
            );
        } else if (!TheSongList.listMenuEntries[listMenuPos].hiddenEntry) {
            bool isCurSong = TheSongList.curSong && (listMenuPos == curSongMenuPos);
            Font artistFont = assets.josefinSansItalic;
            Song *songi = TheSongList.sortedSongs[TheSongList.listMenuEntries[listMenuPos].
                songListID];
            float songXPos = u.LeftSide + u.winpct(0.005f) - 2;
            float songYPos = std::floor(
                (u.hpct(0.266666f)) + ((songEntryHeight) * (listMenuPos - topOflistMenu))
            );
            if (listMenuPos % 2) {
                DrawRectangle(
                    0,
                    songYPos,
                    (u.RightSide - u.winpct(0.25f)),
                    songEntryHeight,
                    Color{ 0, 0, 0, 64 }
                );
            }
            GuiSetStyle(BUTTON, BORDER_WIDTH, 0);
            if (isCurSong) {
                auto timer = Clamp(curTime - selectionTime, 0, 0.15);
                DrawRectangleRec(Rectangle{ 0, songYPos, (u.RightSide - u.winpct(0.25f)),
                                            songEntryHeight },
                                 ColorBrightness(AccentColor,
                                                 Remap(timer, 0, 0.15, 0, -0.4)));
            }
            auto sourceTex = TheSourceIcons[songi->source]->GetTexture();
            auto padding = u.hinpct(0.005f);
            DrawTexturePro(
                sourceTex,
                { 0, 0, (float)sourceTex.width, (float)sourceTex.height}, {songXPos-songEntryHeight, songYPos+padding, songEntryHeight-padding*2, songEntryHeight-padding*2}, {0 , 0}, 0, WHITE);
            BeginScissorMode(
                0,
                u.hpct(0.15f),
                u.RightSide - u.winpct(0.25f),
                u.hinpct(0.7f)
            );
            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
            EndScissorMode();

            int songTitleWidth = (u.winpct(0.3f)) - 6;
            int songArtistWidth = (u.winpct(0.5f)) - 6;

            if (songi->titleTextWidth >= (float)songTitleWidth) {
                if (curTime > songi->titleScrollTime
                    && curTime < songi->titleScrollTime + 3.0)
                    songi->titleXOffset = 0;

                if (curTime > songi->titleScrollTime + 3.0) {
                    songi->titleXOffset -= 1;
                    if (songi->titleXOffset
                        < -(songi->titleTextWidth - (float)songTitleWidth)) {
                        songi->titleXOffset = -(songi->titleTextWidth - (float)
                            songTitleWidth);
                        songi->titleScrollTime = curTime + 3.0;
                    }
                }
            }
            auto LightText = Color{ 203, 203, 203, 255 };
            BeginScissorMode(
                (int)songXPos + (isCurSong ? 5 : 20),
                (int)songYPos,
                songTitleWidth,
                songEntryHeight
            );
            GameMenu::mhDrawText(
                assets.rubikBold,
                songi->title.c_str(),
                { songXPos + songi->titleXOffset + (isCurSong ? 10 : 20),
                  songYPos + u.hinpct(0.0125f) },
                u.hinpct(0.035f),
                isCurSong ? WHITE : LightText,
                ASSET(sdfShader),
                LEFT
            );
            EndScissorMode();

            if (songi->artistTextWidth > (float)songArtistWidth) {
                if (curTime > songi->artistScrollTime
                    && curTime < songi->artistScrollTime + 3.0)
                    songi->artistXOffset = 0;

                if (curTime > songi->artistScrollTime + 3.0) {
                    songi->artistXOffset -= 1;
                    if (songi->artistXOffset
                        < -(songi->artistTextWidth - (float)songArtistWidth)) {
                        songi->artistScrollTime = curTime + 3.0;
                    }
                }
            }

            BeginScissorMode(
                (int)songXPos + 30 + (int)songTitleWidth,
                (int)songYPos,
                songArtistWidth,
                songEntryHeight
            );
            GameMenu::mhDrawText(
                artistFont,
                songi->artist.c_str(),
                { songXPos + 30 + (float)songTitleWidth + songi->artistXOffset,
                  songYPos + u.hinpct(0.02f) },
                u.hinpct(0.025f),
                isCurSong ? WHITE : LightText,
                ASSET(sdfShader),
                LEFT
            );
            EndScissorMode();
        }
    }
    if (curSongMenuPos > 0 && curSongMenuPos < TheSongList.
                                               listMenuEntries.size()) {
        std::string categoryHeaderText = "";
        size_t songIndex = curSongMenuPos;

        if (TheSongList.listMenuEntries[songIndex].isHeader && songIndex > 0 && !
            TheSongList.listMenuEntries[songIndex - 1].isHeader) {
            songIndex--;
        } else if (!TheSongList.listMenuEntries[songIndex].isHeader) {
        } else if (songIndex + 1 < TheSongList.listMenuEntries.size() && !TheSongList.
            listMenuEntries[songIndex + 1].isHeader) {
            songIndex++;
        }

        if (songIndex < TheSongList.listMenuEntries.size() && !TheSongList.listMenuEntries
            [songIndex].isHeader) {
            Song *representativeSong = TheSongList.sortedSongs[TheSongList.listMenuEntries[
                songIndex].songListID];
            switch (currentSortValue) {
            case SortType::Title:
                categoryHeaderText = representativeSong->title.empty()
                    ? "#"
                    : std::string(1, toupper(representativeSong->title[0]));
                break;
            case SortType::Artist:
                categoryHeaderText = representativeSong->artist.empty()
                    ? "#"
                    : std::string(1, toupper(representativeSong->artist[0]));
                break;
            case SortType::Source:
                categoryHeaderText = representativeSong->source.empty()
                    ? "Unknown"
                    : representativeSong->source;
                break;
            case SortType::Length:
                categoryHeaderText = TheSongList.listMenuEntries[curSongMenuPos].
                    headerChar;
                break;
            case SortType::Year:
                categoryHeaderText = representativeSong->releaseYear.empty()
                    ? "Unknown Year"
                    : representativeSong->releaseYear;
                break;
            default:
                categoryHeaderText = "";
                break;
            }
        }

        if (categoryHeaderText.empty()) {
            categoryHeaderText = TheSongList.listMenuEntries[curSongMenuPos]
                .headerChar;
        }

        GameMenu::mhDrawText(
            assets.rubikBold,
            categoryHeaderText.c_str(),
            { u.LeftSide + 5, u.hpct(0.218333f) },
            u.hinpct(0.035f),
            WHITE,
            ASSET(sdfShader),
            LEFT
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

    std::string titleText = TheSourceIcons[SongToDisplayInfo->source]->name;
    float titleFontSize = u.hinpct(0.035f);
    float titleTextWidth = MeasureTextEx(assets.rubikBold,
                                         titleText.c_str(),
                                         titleFontSize,
                                         0).x;
    float titleTextX = AlbumX - AlbumInner + (AlbumHeight / 2.0f) - (titleTextWidth /
        2.0f);
    float titleTextY = AlbumY - u.hinpct(0.045f);
    GameMenu::mhDrawText(
        assets.rubikBold,
        titleText.c_str(),
        { titleTextX, titleTextY },
        titleFontSize,
        WHITE,
        ASSET(sdfShader),
        LEFT
    );

    if (IsTextureValid(TheArtLoader.loadedArt)) {
        DrawTexturePro(
            TheArtLoader.loadedArt,
            Rectangle{ 0,
                       0,
                       (float)TheArtLoader.loadedArt.width,
                       (float)TheArtLoader.loadedArt.width },
            Rectangle{ (float)AlbumX - AlbumInner,
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
    GameMenu::mhDrawText(assets.redHatDisplayBlack,
                         "MUSIC LIBRARY",
                         { TextPlacementLR, TextPlacementTB },
                         u.hinpct(0.125f),
                         WHITE,
                         assets.sdfShader,
                         LEFT);

    std::string albumText = SongToDisplayInfo->album.empty()
        ? "No Album Listed"
        : SongToDisplayInfo->album;
    std::string yearText = SongToDisplayInfo->releaseYear.empty()
        ? "Unknown Year"
        : SongToDisplayInfo->releaseYear;
    std::string albumDisplayText = albumText + " | " + yearText;
    float albumTextHeight = MeasureTextEx(assets.rubikBold,
                                          albumDisplayText.c_str(),
                                          u.hinpct(0.035f),
                                          0).y;
    float albumTextWidth = MeasureTextEx(assets.rubikBold,
                                         albumDisplayText.c_str(),
                                         u.hinpct(0.035f),
                                         0).x;
    float albumNameTextCenter = u.RightSide - u.winpct(0.125f) - AlbumInner;
    float albumTTop = AlbumY + AlbumHeight + u.hinpct(0.011f);
    float albumNameFontSize = albumTextWidth <= u.winpct(0.25f)
        ? u.hinpct(0.035f)
        : u.winpct(0.23f) / (albumTextWidth / albumTextHeight);
    float albumNameLeft = albumNameTextCenter - (MeasureTextEx(
        assets.rubikBold,
        albumDisplayText.c_str(),
        albumNameFontSize,
        0).x / 2);
    float albumNameTextTop = albumTextWidth <= u.winpct(0.25f)
        ? albumTTop
        : albumTTop + ((u.hinpct(0.035f) / 2) - (albumNameFontSize / 2));
    GameMenu::mhDrawText(assets.rubikBold,
               albumDisplayText.c_str(),
               { albumNameLeft, albumNameTextTop },
               albumNameFontSize,
               WHITE,
               ASSET(sdfShader),
               LEFT);

    DrawLine(u.RightSide - AlbumHeight - AlbumOuter,
             AlbumY + AlbumHeight + AlbumOuter + (u.hinpct(0.04f)),
             u.RightSide,
             AlbumY + AlbumHeight + AlbumOuter + (u.hinpct(0.04f)),
             WHITE);

    float DiffTop = AlbumY + AlbumHeight + AlbumOuter + (u.hinpct(0.045f));
    float IconWidth = float(AlbumHeight - AlbumOuter) / 5.0f;
    GameMenu::mhDrawText(assets.rubikItalic,
                         "Pad",
                         { (u.RightSide - AlbumHeight + AlbumInner), DiffTop },
                         AlbumOuter * 3,
                         WHITE,
                         assets.sdfShader,
                         LEFT);
    GameMenu::mhDrawText(assets.rubikItalic,
                         "Classic",
                         { (u.RightSide - AlbumHeight + AlbumInner),
                           DiffTop + IconWidth + (AlbumOuter * 3) },
                         AlbumOuter * 3,
                         WHITE,
                         assets.sdfShader,
                         LEFT);
    for (int i = 0; i < 10; i++) {
        bool RowTwo = i < 5;
        int RowTwoInt = i - 5;
        float PosTopAddition = RowTwo ? AlbumOuter * 3 : AlbumOuter * 6;
        float BoxTopPos = DiffTop + PosTopAddition + float(IconWidth * (RowTwo ? 0 : 1));
        float ResetToLeftPos = (float)(RowTwo ? i : RowTwoInt);
        int asdasd = (float)(RowTwo ? i : RowTwoInt);
        float IconLeftPos = (float)(u.RightSide - AlbumHeight) + IconWidth *
            ResetToLeftPos;
        Rectangle Placement = { IconLeftPos, BoxTopPos, IconWidth, IconWidth };
        Color TintColor = WHITE;
        int diffNumber = SongToDisplayInfo->parts[i].diff;
        if (diffNumber == -1)
            TintColor = DARKGRAY;
        auto instIcon = assets.InstIcons[asdasd];
        DrawTexturePro(*instIcon,
                       { 0, 0, (float)instIcon->width, (float)instIcon->height },
                       Placement,
                       { 0, 0 },
                       0,
                       TintColor);
        DrawTexturePro(assets.BaseRingTexture,
                       { 0, 0, (float)assets.BaseRingTexture.width,
                         (float)assets.BaseRingTexture.height },
                       Placement,
                       { 0, 0 },
                       0,
                       ColorBrightness(WHITE, 2));
        if (diffNumber > 0) {
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
    GuiSetStyle(BUTTON,
                BASE_COLOR_NORMAL,
                0x00000000);
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    float ButtonWidth = u.winpct(0.125f);
    float ButtonStart = u.LeftSide;
    float ButtonTop = GetRenderHeight() - u.hpct(0.1475f);
    float LabelStart = u.LeftSide + u.hinpct(0.01f);
    float LabelTop = GetRenderHeight() - u.hpct(0.1375f);
    if (GuiButton(Rectangle{ ButtonStart, ButtonTop,
                             ButtonWidth, u.hinpct(0.05f) },
                  "       Play Song")) {
        if (TheSongList.curSong) {
            Unload();
            //TheSongList.curSong->LoadSongIni(TheSongList.curSong->songDir);
            TheMenuManager.SwitchScreen(READY_UP);
        }
    }
    GameMenu::mhDrawText(ASSET(rubikBold), "A", {LabelStart, LabelTop}, u.hinpct(0.03f), GREEN, ASSET(sdfShader), LEFT);

    if (GuiButton(
        Rectangle{ ButtonStart + ButtonWidth * 2,
                   ButtonTop,
                   ButtonWidth,
                   u.hinpct(0.05f) },
        "      Sort"
    )) {
        //todo: I BROKE THE SORT BUTTON LMFAO
        // no i didnt
        size_t selectedSongIndex = -1;
        if (TheSongList.curSong) {
            for (size_t i = 0; i < TheSongList.sortedSongs.size(); i++) {
                if (TheSongList.sortedSongs[i] == TheSongList.curSong) {
                    selectedSongIndex = i;
                    break;
                }
            }
        }
        currentSortValue = NextSortType(currentSortValue);
        TheSongList.sortList(currentSortValue);
        if (selectedSongIndex >= 0 && selectedSongIndex < TheSongList.sortedSongs.size()) {
            TheSongList.curSong = TheSongList.sortedSongs[selectedSongIndex];
            curSongMenuPos = TheSongList.curSong->songListPos - 6;
            if (curSongMenuPos < 1)
                curSongMenuPos = 1;
            if (curSongMenuPos > TheSongList.listMenuEntries.size())
                curSongMenuPos = TheSongList.listMenuEntries.size() - 1;
            if (!TheAudioManager.loadedStreams.empty()) {
                for (auto &stream : TheAudioManager.loadedStreams) {
                    TheAudioManager.StopPlayback(stream.handle);
                }
                TheAudioManager.loadedStreams.clear();
                currentPreviewVolume = 0.0f;
                previewState = PreviewState::FadeIn;
            }
            selectionTime = curTime;
        }
    }
    if (GuiButton(Rectangle{ ButtonStart + (ButtonWidth),
                             ButtonTop, ButtonWidth,
                             u.hinpct(0.05f) },
                  "       Back")) {
        if (!TheAudioManager.loadedStreams.empty()) {
            for (auto &stream : TheAudioManager.loadedStreams) {
                TheAudioManager.StopPlayback(stream.handle);
            }
            TheAudioManager.loadedStreams.clear();
        }
        Unload();
        TheMenuManager.SwitchScreen(MAIN_MENU);
    }
    GameMenu::mhDrawText(ASSET(rubikBold), "B", {LabelStart + ButtonWidth, LabelTop}, u.hinpct(0.03f), RED, ASSET(sdfShader), LEFT);
    GuiButton(Rectangle{ ButtonStart + (ButtonWidth * 3),
                             ButtonTop, ButtonWidth * 2,
                             u.hinpct(0.05f) },
                  "        (Hold) Jump Sections");
    GameMenu::mhDrawText(ASSET(rubikBold), "LB", {LabelStart + (ButtonWidth * 3), LabelTop}, u.hinpct(0.03f), ORANGE, ASSET(sdfShader), LEFT);
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    DrawOvershell();
}
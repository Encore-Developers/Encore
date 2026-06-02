//
// Created by marie on 16/11/2024.
//

#include "raylib.h"
#include "SongSelectMenu.h"


#include "../MenuManager.h"
#include "MainMenu.h"
#include "raygui.h"
#include "settings/settings.h"
#include "../uiUnits.h"
#include "../overshell/OvershellHelper.h"

#include "song/audio.h"
#include "song/songlist.h"
#include "assets.h"
#include "imgui.h"
#include "raymath.h"
#include "menus/gameplay/ReadyUpMenu.h"
#include "menus/locale/Locale.h"
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
    for (int sectInt = 0; sectInt < TheSongList.sectionEntries.size(); sectInt++) {
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
        if (curSongMenuPos >= sect.firstListID && curSongMenuPos <= sect.lastListID) {
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

        TheSongList.sortedSongs[TheSongList.listMenuEntries[newPos].songListID]->
            LoadAlbumArt();
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
    buttReg.HandleInput(event);
    /*
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
    }*/
}

void SongSelectMenu::ScrollToCurrentSong() {
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
    gameplaySet.StartLoad();

    ScrollToCurrentSong();
    buttReg.buttMap.clear();
    NEWBUTTONACTION2(buttReg,
                     LANE_1,
                     "songSelect.playSong",
                     {
                     if (_action != Encore::RhythmEngine::Action::PRESS) return;
                     if (!TheSongList.curSong) return;
                     Unload();
                     TheMenuManager.CreateAndSwitchMenu<ReadyUpMenu>();
                     })
    NEWBUTTONACTION2(buttReg,
                     LANE_2,
                     "generic.back",
                     {
                     if (_action != Encore::RhythmEngine::Action::PRESS) return;
                     if (!TheSongList.curSong) return;
                     Unload();
                     TheMenuManager.CreateAndSwitchMenu<MainMenu>();
                     })
    NEWBUTTONACTION2(buttReg,
                     LANE_3,
                     "songSelect.sort",
                     {
                     if (_action != Encore::RhythmEngine::Action::PRESS) return;
                     currentSortValue = NextSortType(currentSortValue);
                     TheSongList.sortList(currentSortValue);
                     ScrollToCurrentSong();
                     })
    NEWBUTTONACTION2(buttReg,
                     LANE_5,
                     "songSelect.jumpHeaders",
                     {
                     if (_action == Encore::RhythmEngine::Action::REPEAT) return;
                     ControllerOrangeHeld.at(slot) = _action == Encore::RhythmEngine::
                     Action::PRESS;
                     })
    NEWBUTTONACTION2(buttReg,
                     INPUT_LEFT,
                     "PgUp",
                     {
                     if (_action != Encore::RhythmEngine::Action::PRESS) return;
                     ScrollSongSelect(5);
                     },
                     false)
    NEWBUTTONACTION2(buttReg,
                     INPUT_RIGHT,
                     "PgDn",
                     {
                     if (_action != Encore::RhythmEngine::Action::PRESS) return;
                     ScrollSongSelect(-5);
                     },
                     false)
    NEWBUTTONACTION2(buttReg,
                     STRUM_UP,
                     "Up",
                     {
                     if (_action != Encore::RhythmEngine::Action::PRESS) return;
                     if (ControllerOrangeHeld.at(slot)) {
                     ScrollUpHeader();
                     } else {
                     ScrollSongSelect(1);
                     }
                     },
                     false)
    NEWBUTTONACTION2(buttReg,
                     STRUM_DOWN,
                     "Down",
                     {
                     if (_action != Encore::RhythmEngine::Action::PRESS) return;
                     if (ControllerOrangeHeld.at(slot)) {
                     ScrollDownHeader();
                     } else {
                     ScrollSongSelect(-1);
                     }
                     },
                     false)
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

    for (auto &stream : TheAudioManager.loadedStreams) {
        float volume = currentPreviewVolume;
        TheAudioManager.SetAudioStreamVolume(stream.instrument, volume);
    }

    TheAudioManager.UpdateAudioStreamVolumes();
}


void SongSelectMenu::KeyboardInputCallback(SDL_KeyboardEvent *event) {
    if (event->down || event->repeat) {
        switch (event->key) {
        case SDLK_UP:
            ScrollSongSelect(1);
            break;
        case SDLK_DOWN:
            ScrollSongSelect(-1);
            break;
        case SDLK_LEFT:
            ScrollSongSelect(5);
            break;
        case SDLK_RIGHT:
            ScrollSongSelect(-5);
            break;
        case SDLK_PAGEUP:
            ScrollUpHeader();
            break;
        case SDLK_PAGEDOWN:
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
        for (auto &stream : TheAudioManager.loadedStreams) {
            float volume = 0.0f;
            TheAudioManager.SetAudioStreamVolume(stream.instrument, volume);
        }
        TheAudioManager.UpdateAudioStreamVolumes();
        TheAudioManager.BeginPlayback(TheAudioManager.loadedStreams[0].handle);
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
                if (TheSongList.listMenuEntries[curSongMenuPos].songListID < TheSongList.
                    sortedSongs
                    .size()) {
                    LoadPreview(
                        *TheSongList.sortedSongs[TheSongList.listMenuEntries[
                                curSongMenuPos].
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
        GetRenderWidth(),
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

    std::string sortType = LOCALIZE(
        "songSelect.sortTypes."+sortTypes[(int)currentSortValue]);
    Encore::TextDisplay sortData;
    sortData.Fnt(ASSET(josefinSansItalic)).Size(u.hinpct(0.03f))
            .Pos(u.LeftSide, u.hpct(0.165f))
            .DrawText(LOCALISE_FMT("songSelect.sortedBy", sortType));

    sortData.pos.x = AlbumX - (AlbumOuter * 2);
    std::size_t sortCount = TheSongList.sortedSongs.size();
    sortData.Align(RIGHT).DrawText(LOCALISE_FMT("songSelect.songsLoaded", sortCount));

    float songTitleWidth = u.winpct(0.3f);
    float songArtistWidth = u.winpct(0.5f);
    float songEntryHeight = u.hinpct(0.058333f);
    ASSET(AltBackground).Draw({0,
        u.hpct(0.266666f) - songEntryHeight,
        (u.RightSide - u.winpct(0.25f)),
        songEntryHeight
        }, ColorBrightness(AccentColor, -0.5f));
    int topOflistMenu = curSongMenuPos <= 4 ? 1 : curSongMenuPos - 4;
    Encore::TextDisplay songTitle;
    Encore::TextDisplay songArtist;
    float songXPos = u.LeftSide + u.winpct(0.005f) - 2 + songEntryHeight;
    float songYPos = std::floor(u.hpct(0.266666f));
    songTitle.Fnt(ASSET(rubikBold)).Size(u.hinpct(0.035f))
             .Pos(songXPos, songYPos + u.hinpct(0.0125f));
    auto padding = u.hinpct(0.01f);
    songArtist.Fnt(ASSET(josefinSansItalic)).Size(u.hinpct(0.025f))
              .Pos(songXPos + songTitleWidth + 30, songYPos + u.hinpct(0.02f));

    for (size_t listMenuPos = topOflistMenu;
         listMenuPos < TheSongList.listMenuEntries.size() &&
         (listMenuPos < curSongMenuPos + (topOflistMenu <= 4 ? 10 : 6));
         listMenuPos++) {
        ZoneScopedN("Draw Entry")
        if (TheSongList.listMenuEntries.size() == listMenuPos)
            break;

        if (TheSongList.listMenuEntries[listMenuPos].isHeader) {
            Rectangle entryRec{ 0, songYPos, u.RightSide - u.winpct(0.25f),
                                songEntryHeight };
            Color headerColor = ColorBrightness(AccentColor, -0.5f);
            if (listMenuPos == curSongMenuPos) {
                headerColor = ColorBrightness(AccentColor, -0.25f);
            }
            songTitle.Col(WHITE);
            songArtist.Col(WHITE);

            ASSET(AltBackground).Draw(entryRec, headerColor);

            // DrawRectangleRec(entryRec, headerColor);
            std::string headerText;
            if (currentSortValue == SortType::Source) {
                headerText = TheSourceIcons.GetSourceName(
                    TheSongList.listMenuEntries[listMenuPos].headerChar);

                auto SourceTex = TheSourceIcons.GetIcon(
                    TheSongList.sortedSongs[TheSongList.listMenuEntries[
                        topOflistMenu].songListID]->source);
                Rectangle source = { 0, 0, float(SourceTex->GetTexture().width),
                                     float(SourceTex->GetTexture().height) };
                Rectangle dest = { songXPos - songEntryHeight, songYPos + padding,
                                   songEntryHeight - padding * 2,
                                   songEntryHeight - padding * 2 };
                DrawTexturePro(SourceTex->GetTexture(), source, dest, { 0 }, 0, WHITE);
            } else {
                headerText = TheSongList.listMenuEntries[listMenuPos].headerChar;
            }
            songTitle.pos.x = songXPos;
            songTitle.DrawText(headerText);
            songTitle.pos.y += songEntryHeight;
            songArtist.pos.y += songEntryHeight;
            songYPos += songEntryHeight;
        } else if (!TheSongList.listMenuEntries[listMenuPos].hiddenEntry) {
            Rectangle entryRec{ 0, songYPos, u.RightSide - u.winpct(0.25f),
                                songEntryHeight };
            bool isCurSong = TheSongList.curSong && (listMenuPos == curSongMenuPos);
            Song *songi = TheSongList.sortedSongs[TheSongList.listMenuEntries[listMenuPos]
                .songListID];
            Color background = { 128, 128, 128, 128 };
            if (listMenuPos % 2) {
                background = {128, 128, 128, 64 };
            }
            if (isCurSong) {
                auto timer = Clamp(curTime - selectionTime, 0, 0.15);
                background =
                    ColorBrightness(AccentColor,
                                    Remap(timer, 0, 0.15, 0, -0.25));
            }
            ASSET(AltBackground).Draw(entryRec, background);

            if (currentSortValue != SortType::Source) {
                auto sourceTex = TheSourceIcons[songi->source]->GetTexture();
                DrawTexturePro(
                    sourceTex,
                    { 0, 0, (float)sourceTex.width, (float)sourceTex.height },
                    { songXPos - songEntryHeight, songYPos + padding,
                      songEntryHeight - padding * 2, songEntryHeight - padding * 2 },
                    { 0, 0 },
                    0,
                    WHITE);
            }
            // im so fucking scared
            // i dont know what this does. its framerate dependant too. im so fucking confused
            // theres gotta be a better way but im not being paid enough to figure it out
            songi->titleTextWidth = songTitle.TextWidth(songi->title.c_str());
            if (songi->titleTextWidth > songTitleWidth) {
                if (curTime > songi->titleScrollTime && curTime < songi->titleScrollTime +
                    3.0) {
                    songi->titleXOffset = 0;
                }
                if (curTime > songi->titleScrollTime + 3.0) {
                    songi->titleXOffset -= 1;
                    if (songi->titleXOffset < -(songi->titleTextWidth - songTitleWidth)) {
                        songi->titleXOffset = -(songi->titleTextWidth - songTitleWidth);
                        songi->titleScrollTime = curTime + 3.0;
                    }
                }
            }
            std::string artistText = songi->artist;
            if (currentSortValue == SortType::Artist) {
                artistText = songi->album;
            }
            songi->artistTextWidth = songTitle.TextWidth(artistText);
            if (songi->artistTextWidth > songArtistWidth) {
                if (curTime > songi->artistScrollTime && curTime < songi->artistScrollTime
                    + 3.0) {
                    songi->artistXOffset = 0;
                }
                if (curTime > songi->artistScrollTime + 3.0) {
                    songi->artistXOffset -= 1;
                    if (songi->artistXOffset < -(songi->artistTextWidth -
                        songArtistWidth)) {
                        songi->artistXOffset = -(songi->artistTextWidth -
                            songArtistWidth);
                        songi->artistScrollTime = curTime + 3.0;
                    }
                }
            }
            if (!isCurSong) {
                songTitle.Col({ 203, 203, 203, 255 });
                songArtist.Col({ 203, 203, 203, 255 });
            } else {
                songTitle.Col(WHITE);
                songArtist.Col(WHITE);
            }
            songTitle.pos.x = songXPos + songi->titleXOffset;
            BeginScissorMode(songXPos, songYPos, songTitleWidth, songEntryHeight);
            songTitle.DrawText(songi->title);
            EndScissorMode();

            BeginScissorMode(songXPos + songTitleWidth + 30,
                             songYPos,
                             songArtistWidth,
                             songEntryHeight);
            songArtist.pos.x = songXPos + songTitleWidth + songi->artistXOffset + 30;
            songArtist.DrawText(artistText);
            EndScissorMode();

            songTitle.pos.y += songEntryHeight;
            songArtist.pos.y += songEntryHeight;
            songYPos += songEntryHeight;
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
        Song &repSong = *TheSongList.sortedSongs[TheSongList.listMenuEntries[
            topOflistMenu].songListID];
        if (TheSongList.listMenuEntries[topOflistMenu].isHeader) {
            repSong = *TheSongList.sortedSongs[TheSongList.listMenuEntries[
                topOflistMenu - 1].songListID];
        }
        if (songIndex < TheSongList.listMenuEntries.size() && !TheSongList.listMenuEntries
            [songIndex].isHeader) {
            switch (currentSortValue) {
            case SortType::Title:
                for (int sectInt = 0; sectInt < TheSongList.sectionEntries.size(); sectInt
                     ++) {
                    auto sect = TheSongList.sectionEntries[sectInt];
                    if (topOflistMenu - 1 >= sect.firstListID && topOflistMenu - 1 <= sect
                        .lastListID) {
                        categoryHeaderText = TheSongList.listMenuEntries[sect.firstListID]
                            .headerChar;
                        break;
                    }
                }
                // categoryHeaderText = repSong.title.empty()
                //    ? "#"
                //    : std::string(1, toupper(repSong.title[0]));
                break;
            case SortType::Artist:
                categoryHeaderText = repSong.artist.empty()
                    ? "Unknown Artist"
                    : repSong.artist;
                break;
            case SortType::Source:
                categoryHeaderText = TheSourceIcons.GetSourceName(repSong.source);
                break;
            case SortType::Length:
                categoryHeaderText = TheSongList.listMenuEntries[curSongMenuPos].
                    headerChar;
                break;
            case SortType::Year:
                categoryHeaderText = repSong.releaseYear.empty()
                    ? "Unknown Year"
                    : repSong.releaseYear;
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
        if (currentSortValue == SortType::Source) {
            auto SourceTex = TheSourceIcons.GetIcon(repSong.source);
            Rectangle source = { 0, 0, float(SourceTex->GetTexture().width),
                                 float(SourceTex->GetTexture().height) };
            Rectangle dest = { songXPos - songEntryHeight, u.hpct(0.2075f) + padding,
                               songEntryHeight - padding * 2,
                               songEntryHeight - padding * 2 };
            DrawTexturePro(SourceTex->GetTexture(), source, dest, { 0 }, 0, WHITE);
        }
        songTitle.Pos(songXPos, u.hpct(0.218333f));
        songTitle.Col(WHITE);
        songTitle.DrawText(categoryHeaderText);
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

    std::string titleText = TheSourceIcons.GetSourceName(SongToDisplayInfo->source);
    float titleFontSize = u.hinpct(0.035f);
    float titleTextWidth = MeasureTextEx(assets.rubikBold,
                                         titleText.c_str(),
                                         titleFontSize,
                                         0).x;
    float titleTextX = AlbumX - AlbumInner + (AlbumHeight / 2.0f) - (titleTextWidth /
        2.0f);
    float titleTextY = AlbumY - u.hinpct(0.045f);
    Encore::Text::DrawText(
        assets.rubikBold,
        titleText.c_str(),
        { titleTextX, titleTextY },
        titleFontSize,
        WHITE,
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
    Encore::Text::lDrawText(assets.rubik,
                            "songSelect.quickplay",
                            { u.LeftSide, u.hpct(0.027f) },
                            u.hinpct(0.042f),
                            LIGHTGRAY,
                            LEFT);

    Encore::TextDisplay header;
    header.Pos(TextPlacementLR, TextPlacementTB)
          .Bounds(u.winpct(0.74), u.hinpct(0.125f))
          .Size(u.hinpct(0.125f)).Fnt(ASSET(redHatDisplayBlack)).lDrawText(
              "songSelect.header");

    std::string albumText = SongToDisplayInfo->album.empty()
        ? "No Album Listed"
        : SongToDisplayInfo->album;
    std::string yearText = SongToDisplayInfo->releaseYear.empty()
        ? "67"
        : SongToDisplayInfo->releaseYear;
    std::string albumDisplayText = albumText + ", " + yearText;

    float albumTTop = AlbumY + AlbumHeight + AlbumOuter;

    DrawLine(u.RightSide - AlbumHeight - AlbumOuter,
             AlbumY + AlbumHeight + AlbumOuter + (u.hinpct(0.04f)),
             u.RightSide,
             AlbumY + AlbumHeight + AlbumOuter + (u.hinpct(0.04f)),
             WHITE);

    float DiffTop = AlbumY + AlbumHeight + AlbumOuter + (u.hinpct(0.045f));
    float IconWidth = float(AlbumHeight - AlbumOuter) / 5.0f;
    Encore::Text::lDrawText(assets.rubikItalic,
                            "parts.pad",
                            { (u.RightSide - AlbumHeight + AlbumInner), DiffTop },
                            AlbumOuter * 3,
                            WHITE,
                            LEFT);
    Encore::Text::lDrawText(assets.rubikItalic,
                            "parts.classic",
                            { (u.RightSide - AlbumHeight + AlbumInner),
                              DiffTop + IconWidth + (AlbumOuter * 3) },
                            AlbumOuter * 3,
                            WHITE,
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
    buttReg.DrawPrompts(isOSOpen());

    Encore::TextDisplay albumData;
    albumData.Bounds(AlbumHeight - AlbumOuter, u.hinpct(0.04f))
             .Pos(AlbumX, albumTTop).Size(u.hinpct(0.035f)).Align(CENTER)
             .Fnt(ASSET(rubik));
    albumData.DrawText(albumDisplayText);
    /*if (GuiButton(Rectangle{ ButtonStart, ButtonTop,
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
        currentSortValue = NextSortType(currentSortValue);
        TheSongList.sortList(currentSortValue);
        ScrollToCurrentSong();
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
*/
    DrawOvershell();
}
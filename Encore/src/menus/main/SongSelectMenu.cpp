//
// Created by marie on 16/11/2024.
//

#include "raylib.h"
#include "SongSelectMenu.h"


#include "../MenuManager.h"
#include "MainMenu.h"
#include "raygui.h"
#include "settings/settings.h"
#include "../util/uiUnits.h"
#include "../overshell/OvershellHelper.h"

#include "song/audio.h"
#include "song/songlist.h"
#include "assets.h"
#include "imgui.h"
#include "raymath.h"
#include "menus/gameplay/ReadyUpMenu.h"
#include "menus/util/locale/Locale.h"
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
    Encore::ControllerEvent event) {
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

void SongSelectMenu::TogglePlaylistMode() {
    TheSongList.PlaylistMode = !TheSongList.PlaylistMode;
    if (TheSongList.PlaylistMode) {
        buttReg.buttMap.at(Encore::InputChannel::LANE_1).Name =
            "songSelect.addToPlaylist";
        buttReg.buttMap.at(Encore::InputChannel::LANE_2).Name =
            "songSelect.removeFromPlaylist";
        buttReg.buttMap.at(Encore::InputChannel::LANE_4).Name =
            "songSelect.disablePlaylist";
        buttReg.buttMap.at(Encore::InputChannel::OVERDRIVE).barVisible = true;
        return;
    }
    buttReg.buttMap.at(Encore::InputChannel::LANE_1).Name = "songSelect.playSong";
    buttReg.buttMap.at(Encore::InputChannel::LANE_2).Name = "generic.back";
    buttReg.buttMap.at(Encore::InputChannel::LANE_4).Name = "songSelect.createPlaylist";
    buttReg.buttMap.at(Encore::InputChannel::OVERDRIVE).barVisible = false;
    TheSongList.playlist.clear();
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
    if (!TheSongList.curSong) {
        TheSongList.curSong = TheSongList.sortedSongs[0];
    }
    TheSongList.curSong->LoadAlbumArt();
    gameplaySet.StartLoad();

    ScrollToCurrentSong();
    buttReg.buttMap.clear();
    NEWBUTTONACTION2(buttReg,
                     LANE_1,
                     "songSelect.playSong",
                     {
                     if (_action != Encore::Action::PRESS) return;
                     if (TheSongList.PlaylistMode) {
                     if (TheSongList.curSong) {
                     TheSongList.playlist.push_back(TheSongList.curSong);
                     }
                     return;
                     }
                     if (!TheSongList.curSong) return;
                     Unload();
                     TheMenuManager.CreateAndSwitchMenu<ReadyUpMenu>(TheSongList.curSong);

                     })
    NEWBUTTONACTION2(buttReg,
                     LANE_2,
                     "generic.back",
                     {
                     if (_action != Encore::Action::PRESS) return;
                     if (TheSongList.PlaylistMode) {
                     if (!TheSongList.playlist.empty()) {
                     TheSongList.playlist.pop_back();
                     return;
                     }
                     return;
                     }
                     if (!TheSongList.curSong) return;
                     Unload();
                     TheMenuManager.CreateAndSwitchMenu<MainMenu>();
                     })
    NEWBUTTONACTION2(buttReg,
                     LANE_3,
                     "songSelect.sort",
                     {
                     if (_action != Encore::Action::PRESS) return;
                     currentSortValue = NextSortType(currentSortValue);
                     TheSongList.sortList(currentSortValue);
                     ScrollToCurrentSong();
                     })
    NEWBUTTONACTION2(buttReg,
                     LANE_4,
                     "songSelect.createPlaylist",
                     {
                     if (_action != Encore::Action::PRESS) return;
                     TogglePlaylistMode();
                     })
    NEWBUTTONACTION2(buttReg,
                     LANE_5,
                     "songSelect.jumpHeaders",
                     {
                     if (_action == Encore::Action::REPEAT) return;
                     if (slot == -1) return;
                     ControllerOrangeHeld.at(slot) = _action == Encore::Action::PRESS;
                     })
    NEWBUTTONACTION2(buttReg,
                     OVERDRIVE,
                     "songSelect.playPlaylist",
                     {
                     if (_action == Encore::Action::REPEAT) return;
                     if (TheSongList.playlist.empty()) return;
                     Unload();
                     TheSongList.PlaylistSize = TheSongList.playlist.size();
                     TheSongList.PlaylistIndex = 1;
                     TheMenuManager.CreateAndSwitchMenu<ReadyUpMenu>(TheSongList.playlist.
                         front());

                     },
                     false)
    NEWBUTTONACTION2(buttReg,
                     INPUT_LEFT,
                     "PgUp",
                     {
                     if (_action != Encore::Action::PRESS) return;
                     ScrollSongSelect(5);
                     },
                     false)
    NEWBUTTONACTION2(buttReg,
                     INPUT_RIGHT,
                     "PgDn",
                     {
                     if (_action != Encore::Action::PRESS) return;
                     ScrollSongSelect(-5);
                     },
                     false)
    NEWBUTTONACTION2(buttReg,
                     STRUM_UP,
                     "Up",
                     {
                     if (_action != Encore::Action::PRESS) return;
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
                     if (_action != Encore::Action::PRESS) return;
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
        if (TheAudioManager.loadedStreams.empty())
            return;
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

std::string SongSelectMenu::GetHeader() {
    for (int sectInt = 0; sectInt < TheSongList.sectionEntries.size(); sectInt
         ++) {
        auto sect = TheSongList.sectionEntries[sectInt];
        if (topOflistMenu - 1 >= sect.firstListID && topOflistMenu - 1 <= sect
            .lastListID) {
            return TheSongList.listMenuEntries[sect.firstListID].headerChar;
        }
    }
    return "pee";
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
    // GameMenu::DrawTopOvershell(0.208333f);
    EndScissorMode();
    float OvershellTop = u.hpct(0.18f);
    float songEntryHeight = u.hinpct(0.64f / 11);
    GameMenu::DrawTopOvershell(0.18f);
    if (TheSongList.PlaylistMode) {
        DrawRectangleGradientV(
            0,
            0,
            GetRenderWidth(),
            u.hpct(0.15f) - u.hinpct(0.005f),
            Color{ 0, 0, 0, 0 },
            Color{ 255, 0, 255, 128 }
        );
    }
    float TextPlacementTB = u.hpct(0.08f);
    float TextPlacementLR = u.LeftSide;
    Encore::TextDisplay subheader;
    subheader.Pos(u.LeftSide, u.hpct(0.06f)).Size(u.hinpct(0.035f)).Col(LIGHTGRAY).Fnt(
        ASSET(josefinSansBold));
    if (TheSongList.PlaylistMode) {
        subheader.DrawText(LOCALISE("songSelect.quickplay").toString()
            + " > "
            + LOCALISE("songSelect.playlistMode").toString());
    } else {
        subheader.lDrawText("songSelect.quickplay");
    }

    GameMenu::DrawTopBarText();
    int AlbumX = u.RightSide - u.winpct(0.25f);
    int AlbumY = u.hpct(0.04f);
    int AlbumHeight = u.winpct(0.25f);
    int AlbumOuter = u.hinpct(0.01f);
    int AlbumInner = u.hinpct(0.005f);

    std::string sortType = LOCALIZE(
        "songSelect.sortTypes."+sortTypes[(int)currentSortValue]);
    Encore::TextDisplay sortTypeD;
    float SortFontSize = u.hinpct(0.03f);
    sortTypeD.Pos(AlbumX - (AlbumOuter * 2), OvershellTop - SortFontSize - AlbumOuter).
              Size(SortFontSize).
              Fnt(ASSET(josefinSansItalic)).Col(LIGHTGRAY).Align(RIGHT).
              DrawText(LOCALISE_FMT("songSelect.sortedBy", sortType));
    float sortWidth = sortTypeD.TextWidth(LOCALISE_FMT("songSelect.sortedBy", sortType));

    Encore::TextDisplay header;
    header.Pos(TextPlacementLR, TextPlacementTB)
          .Bounds(u.winpct(0.73) - sortWidth, u.hinpct(0.125f))
          .Size(u.hinpct(0.125f)).Fnt(ASSET(redHatDisplayBlack)).lDrawText(
              "songSelect.header");
    //sortData.pos.x = AlbumX - (AlbumOuter * 2);
    //std::size_t sortCount = TheSongList.sortedSongs.size();
    //sortData.Align(RIGHT).DrawText(LOCALISE_FMT("songSelect.songsLoaded", sortCount));

    float songTitleWidth = u.winpct(0.3f);
    float songArtistWidth = u.winpct(0.5f);
    ASSET(AltBackground).Draw({ 0,
                                OvershellTop,
                                (u.RightSide - u.winpct(0.25f)),
                                songEntryHeight
                              },
                              ColorBrightness(AccentColor, -0.5f));
    // Long casts so these are signed
    topOflistMenu = std::clamp((long)topOflistMenu,
                               (long)curSongMenuPos - 6,
                               (long)curSongMenuPos - 3);
    topOflistMenu = std::clamp((long)topOflistMenu,
                               (long)1,
                               (long)TheSongList.listMenuEntries.size() - 10);
    Encore::TextDisplay songTitle;
    Encore::TextDisplay songArtist;
    float songXPos = u.LeftSide + u.winpct(0.005f) - 2 + songEntryHeight;
    float songYPos = std::floor(OvershellTop + songEntryHeight);
    songTitle.Fnt(ASSET(rubikBold)).Size(u.hinpct(0.035f))
             .Pos(songXPos, songYPos + u.hinpct(0.0125f));
    auto padding = u.hinpct(0.01f);
    songArtist.Fnt(ASSET(josefinSansItalic)).Size(u.hinpct(0.025f))
              .Pos(songXPos + songTitleWidth + 30, songYPos + u.hinpct(0.02f));

    for (size_t listMenuPos = topOflistMenu;
         listMenuPos < TheSongList.listMenuEntries.size() &&
         (listMenuPos < topOflistMenu + 10);
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
                    TheSongList.listMenuEntries[listMenuPos].headerChar);
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
                background = { 160, 160, 160, 128 };
            }
            if (isCurSong) {
                auto timer = Clamp(curTime - selectionTime, 0, 0.15);
                background =
                    ColorBrightness(AccentColor,
                                    Remap(timer, 0, 0.15, 0.1, -0.2));
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
    DrawSongInformation(u.RightSide - u.winpct(0.25f), SongToDisplayInfo);
    if (curSongMenuPos > 0 && curSongMenuPos < TheSongList.
                                               listMenuEntries.size()) {
        std::string categoryHeaderText = GetHeader();
        if (currentSortValue == SortType::Source) {
            categoryHeaderText = TheSourceIcons.GetSourceName(GetHeader());
            auto SourceTex = TheSourceIcons.GetIcon(GetHeader());
            Rectangle source = { 0, 0, float(SourceTex->GetTexture().width),
                                 float(SourceTex->GetTexture().height) };
            Rectangle dest = { songXPos - songEntryHeight, OvershellTop + padding,
                               songEntryHeight - padding * 2,
                               songEntryHeight - padding * 2 };
            DrawTexturePro(SourceTex->GetTexture(), source, dest, { 0 }, 0, WHITE);
        }
        songTitle.Pos(songXPos,
                      OvershellTop + ((songEntryHeight - songTitle.fontSize) / 2));
        songTitle.Col(WHITE);
        songTitle.DrawText(categoryHeaderText);
    }

    if (TheSongList.PlaylistMode) {
        Encore::TextDisplay playlistSongs;
        playlistSongs.Size(u.hinpct(0.035f)).Pos(AlbumX, AlbumY);
        for (auto &listSong : TheSongList.playlist) {
            playlistSongs.DrawText(listSong->title);
            playlistSongs.pos.y += u.hinpct(0.0375f);
        }
    }
    buttReg.DrawPrompts(isOSOpen());
    DrawOvershell();
}


void SongSelectMenu::DrawSongInformation(float leftPos, Song* curSong) {
    Units &u = Units::getInstance();

    float AlbumX = leftPos;
    float AlbumY = u.hpct(0.04f);
    float AlbumWidth = u.winpct(0.25f);
    float InfoBoxHeight = u.hpct(1.0) - u.hinpct(0.36);
    Rectangle albumRect{ AlbumX, AlbumY, AlbumWidth, AlbumWidth };
    NPatchInfo shadowOverlay;
    shadowOverlay.source = {0,0,128,128};
    shadowOverlay.top = AlbumWidth*0.1;
    shadowOverlay.bottom = AlbumWidth*0.1;
    shadowOverlay.left = AlbumWidth*0.1;
    shadowOverlay.right = AlbumWidth*0.1;
    shadowOverlay.layout = 0;
    if (TheArtLoader.loadedArt->GetTexture().id != 0) {
        DrawTexturePro(
            *TheArtLoader.loadedArt,
            Rectangle{ 0,
                       0,
                       (float)TheArtLoader.loadedArt->GetTexture().width,
                       (float)TheArtLoader.loadedArt->GetTexture().width },
            albumRect,
            { 0, 0 },
            0,
            WHITE
        );
    } else {
        DrawTexturePro(
            ASSET(missingAlbumArt),
            Rectangle{ 0,
                       0,
                       (float)ASSET(missingAlbumArt).width,
                       (float)ASSET(missingAlbumArt).width },
            albumRect,
            { 0, 0 },
            0,
            WHITE
        );
    }
    DrawTextureNPatch(ASSET(borderShadowLight), shadowOverlay, albumRect, {0}, 0, {255,255,255,128});

    float patchShit = albumRect.width * 0.075f;
    float infoBoxPadding = albumRect.width * 0.05f;
    NPatchInfo scoreBoxPatch;
    scoreBoxPatch.source = { 0, 0, 128, 128 };
    scoreBoxPatch.top = patchShit;
    scoreBoxPatch.bottom = patchShit;
    scoreBoxPatch.left = patchShit;
    scoreBoxPatch.right = patchShit;
    scoreBoxPatch.layout = 0;

    float InfoBoxGradientHeight = albumRect.width * 0.35f;
    float InfoBoxWidth = albumRect.width - (infoBoxPadding * 2);
    DrawRectangleGradientV(
        albumRect.x,
        albumRect.y + albumRect.width,
        albumRect.width,
        InfoBoxGradientHeight,
        GetColor(0x202033FF),
        GetColor(0x181827FF)
    );
    DrawRectangleGradientV(
        albumRect.x,
        albumRect.y + albumRect.width,
        albumRect.width,
        albumRect.height * 0.025f,
        {0,0,0,64},
        {0}
    );
    DrawRectangle(albumRect.x, albumRect.y+InfoBoxGradientHeight+albumRect.height, albumRect.width, InfoBoxHeight, GetColor(0x181827FF));
    Rectangle infoBoxRect{ albumRect.x + infoBoxPadding,
                           albumRect.y + infoBoxPadding + albumRect.width,
                           albumRect.width - (infoBoxPadding * 2),
                           InfoBoxGradientHeight - (infoBoxPadding * 2) };
    DrawTextureNPatch(ASSET(resultsBox), scoreBoxPatch, infoBoxRect, { 0 }, 0, {255,255,255,128});
    float AlbumFontHeight = (InfoBoxGradientHeight - (infoBoxPadding * 2) )/5;
    Encore::TextDisplay BoxDisplay;
    // this sucks can we start scripting it or writing it to a file to reload it yet
    BoxDisplay.Pos(AlbumX+(infoBoxPadding*1.3f), AlbumY+AlbumWidth+(infoBoxPadding*1.55f))
    .Bounds(albumRect.width - ((infoBoxPadding*1.3f) * 2), AlbumFontHeight)
    .Size(AlbumFontHeight)
    .Align(CENTER)
    .Fnt(ASSET(josefinSansBold))
    .DrawText(curSong->album)
    .AddY(AlbumFontHeight * 1.3f)
    .Fnt(ASSET(josefinSansNormal))
    .Col(LIGHTGRAY)
    .Size(AlbumFontHeight * 0.85f)
    .DrawText(curSong->charters[0]);

    // the more i work on this the more i wanna blow my brains out holy shit
    float Midpoint = AlbumX + (AlbumWidth / 2);
    auto SourceTex = TheSourceIcons.GetIcon(curSong->source);
    Encore::TextDisplay sourceName;
    sourceName.Size(AlbumFontHeight*0.9f).Fnt(ASSET(josefinSansNormal));
    float SourceTop = BoxDisplay.pos.y + (AlbumFontHeight * 0.25f);
    float IconSize = AlbumFontHeight * 1.25f;
    float IconSpace = AlbumFontHeight * 1.5f;
    float TextWidth = sourceName.TextWidth(SourceTex->name);
    float LeftPos = Midpoint - ((TextWidth + IconSpace) / 2);

    Rectangle source = { 0, 0, float(SourceTex->GetTexture().width),
                         float(SourceTex->GetTexture().height) };
    Rectangle dest = { LeftPos, SourceTop + AlbumFontHeight,
                       IconSize,
                       IconSize };
    DrawTexturePro(SourceTex->GetTexture(), source, dest, { 0 }, 0, WHITE);
    sourceName.Pos(LeftPos + IconSpace, SourceTop + (IconSize)).Bounds(BoxDisplay.width - IconSpace, 1).DrawText(SourceTex->name);


    // grits teeth
    // please Yoshi give me a proper UI library
    // please rockstar hide it again
    float IconWidth = InfoBoxWidth / 5.125f;
    float IconPadding = (infoBoxPadding * 0.5f) / 5.0f;
    float InfoBoxBottom = AlbumY + AlbumWidth + InfoBoxGradientHeight;
    Encore::TextDisplay partType;
    partType.Size(infoBoxPadding)
    .Fnt(ASSET(rubikBold))
    .Pos(leftPos + infoBoxPadding, InfoBoxBottom)
    .lDrawText("parts.pad")
    .AddY(IconWidth + IconPadding + (infoBoxPadding * 1.5f))
    .lDrawText("parts.classic");
    DrawRectangleGradientH(leftPos+infoBoxPadding, InfoBoxBottom + infoBoxPadding, InfoBoxWidth, 2, WHITE, {255,255,255,32});
    DrawRectangleGradientH(leftPos+infoBoxPadding, partType.pos.y + infoBoxPadding, InfoBoxWidth, 2, WHITE, {255,255,255,32});
    for (int i = 0; i < 10; i++) {
        bool RowTwo = i < 5;
        int RowTwoInt = i - 5;
        float PosTopAddition = RowTwo ? infoBoxPadding * 1.25f : infoBoxPadding * 3;
        float BoxTopPos = InfoBoxBottom + PosTopAddition + (IconWidth * (RowTwo ? 0 : 1));
        float ResetToLeftPos = (float)(RowTwo ? i : RowTwoInt);
        int asdasd = (float)(RowTwo ? i : RowTwoInt);
        float IconLeftPos = leftPos + infoBoxPadding + ((IconWidth + IconPadding)*
            ResetToLeftPos);
        Rectangle Placement = { IconLeftPos, BoxTopPos, IconWidth, IconWidth };
        Color TintColor = WHITE;
        int diffNumber = curSong->Difficulties[i];
        if (diffNumber == -1)
            TintColor = DARKGRAY;
        auto instIcon = TheAssets.InstIcons[asdasd];
        DrawTexturePro(*instIcon,
                       { 0, 0, (float)instIcon->width, (float)instIcon->height },
                       Placement,
                       { 0, 0 },
                       0,
                       TintColor);
        DrawTexturePro(TheAssets.BaseRingTexture,
                       { 0, 0, (float)TheAssets.BaseRingTexture.width,
                         (float)TheAssets.BaseRingTexture.height },
                       Placement,
                       { 0, 0 },
                       0,
                       ColorBrightness(WHITE, 2));
        if (diffNumber > 0) {
            if (diffNumber > 6) {
                diffNumber = 6;
            }
            auto ring = TheAssets.YargRings[diffNumber - 1];
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
}
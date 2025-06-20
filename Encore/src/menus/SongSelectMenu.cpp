//
// Created by marie on 16/11/2024.
//

#include "raylib.h"
#include "SongSelectMenu.h"


#include "MenuManager.h"
#include "gameMenu.h"
#include "raygui.h"
#include "settings.h"
#include "uiUnits.h"
#include "gameplay/gameplayRenderer.h"
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
    if (!IsAudioDeviceReady()) {
        InitAudioDevice();
        TraceLog(LOG_INFO, "Initialized audio device");
    }
    previewStartTime = 0.0;
    phaseStartTime = 0.0;
    currentPreviewVolume = 0.0f;
    previewState = PreviewState::FadeIn;
    animatingSongID = -1;
    prevAnimatingSongID = -1;
    pendingSongID = -1;
    selectionTime = 0.0;
    songTextMetrics.clear();

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
        animatingSongID = TheSongList.curSong->songListPos - 1;
        animationStartTime = GetTime();
        ComputeSongTextMetrics(*TheSongList.curSong);
    } else {
        Encore::EncoreLog(LOG_WARNING, "No current song selected for offset adjustment");
        TheSongList.SongSelectOffset = 1;
    }

    // TheGameRenderer.streamsLoaded = false;
    // TheGameRenderer.midiLoaded = false;
    for (Song& song : TheSongList.songs) {
        if (!song.ini) {
            song.LoadInfo(song.songInfoPath);
        } else {
            song.LoadInfoINI(song.songInfoPath);
        }
        ComputeSongTextMetrics(song);
    }
}

void SongSelectMenu::Unload() {
    if (!TheAudioManager.loadedStreams.empty()) {
        for (auto& stream : TheAudioManager.loadedStreams) {
            TheAudioManager.StopPlayback(stream.handle);
        }
        TheAudioManager.loadedStreams.clear();
    }
}

void SongSelectMenu::KeyboardInputCallback(int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS && key == GLFW_KEY_UP) {
        if (TheSongList.SongSelectOffset <= TheSongList.listMenuEntries.size() && TheSongList.SongSelectOffset >= 1
            && TheSongList.listMenuEntries.size() >= 10) {
            TheSongList.SongSelectOffset--;
            }

        // prevent going past top
        }
        if (TheSongList.SongSelectOffset < 1)
            TheSongList.SongSelectOffset = 1;

        // prevent going past bottom
        if (TheSongList.SongSelectOffset >= TheSongList.listMenuEntries.size() - 10)
            TheSongList.SongSelectOffset = TheSongList.listMenuEntries.size() - 10;
    }
void SongSelectMenu::ControllerInputCallback(int joypadID, GLFWgamepadstate state) {}
std::string SecondsToTimeFormat(int seconds) {
    int minutes = seconds / 60;
    int remainingSeconds = seconds % 60;
    return TextFormat("%d:%02d", minutes, remainingSeconds);
}

void SongSelectMenu::ComputeSongTextMetrics(Song& song) {
    Units u = Units::getInstance();
    Assets& assets = Assets::getInstance();

    TextMetrics metrics;
    const int songTitleWidth = (u.winpct(0.25f)) - 6;
    const int songArtistWidth = (u.winpct(0.25f)) - 6;

    // i tried........ - Jaydenz
    metrics.titleFontSize = u.hinpct(0.035f);
    metrics.titleTextWidth = MeasureTextEx(assets.rubikBold, song.title.c_str(), metrics.titleFontSize, 0).x;
    if (metrics.titleTextWidth > songTitleWidth) {
        metrics.titleFontSize = (songTitleWidth / metrics.titleTextWidth) * u.hinpct(0.035f);
        const float minFontSize = u.hinpct(0.02f);
        if (metrics.titleFontSize < minFontSize) metrics.titleFontSize = minFontSize;
        metrics.titleTextWidth = MeasureTextEx(assets.rubikBold, song.title.c_str(), metrics.titleFontSize, 0).x;
    }

    metrics.artistFontSize = u.hinpct(0.025f);
    metrics.artistTextWidth = MeasureTextEx(assets.josefinSansItalic, song.artist.c_str(), metrics.artistFontSize, 0).x;
    if (metrics.artistTextWidth > songArtistWidth) {
        metrics.artistFontSize = (songArtistWidth / metrics.artistTextWidth) * u.hinpct(0.025f);
        const float minFontSize = u.hinpct(0.02f);
        if (metrics.artistFontSize < minFontSize) metrics.artistFontSize = minFontSize;
        metrics.artistTextWidth = MeasureTextEx(assets.josefinSansItalic, song.artist.c_str(), metrics.artistFontSize, 0).x;
    }

    if (song.songListPos >= 0) {
        songTextMetrics[song.songListPos] = metrics;
    }
}

void SongSelectMenu::UpdatePreviewVolume(double currentTime) {
    float targetVolume = TheGameSettings.avMainVolume * TheGameSettings.avMenuMusicVolume;
    float t;
    static PreviewState lastState = previewState;

    if (previewState != lastState) {
        lastState = previewState;
    }

    if (TheAudioManager.loadedStreams.empty()) {
        currentPreviewVolume = 0.0f;
        return;
    }

    switch (previewState) {
        case PreviewState::FadeIn:
            t = (currentTime - phaseStartTime) / fadeDuration;
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
    int lastIntChosen = (int)mouseWheel.y;
    // set to specified height

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
    Song SongToDisplayInfo = TheSongList.curSong ? *TheSongList.curSong : Song();
    if (TheSongList.curSong) {
        if (TheSongList.curSong->ini)
            TheSongList.curSong->LoadInfoINI(TheSongList.curSong->songInfoPath);
        else
            TheSongList.curSong->LoadInfo(TheSongList.curSong->songInfoPath);
    }

    BeginDrawing();
    ClearBackground(DARKGRAY);
    if (TheSongList.curSong && TheSongList.curSong->AlbumArtLoaded) {
        BeginShaderMode(assets.bgShader);
        DrawAlbumArtBackgroundPro(TheSongList.curSong->albumArtBlur, {0, 0, (float)TheSongList.curSong->albumArtBlur.width, (float)TheSongList.curSong->albumArtBlur.height});
        EndShaderMode();
    } else {
    }

    DrawRectangle(0, 0, u.RightSide - u.LeftSide, (float)GetScreenHeight(), GetColor(0x00000080));
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
        { AlbumX - (AlbumOuter * 2) - MeasureTextEx(assets.josefinSansItalic, TextFormat("Songs loaded: %01i", TheSongList.songs.size()), u.hinpct(0.03f), 0).x, u.hinpct(0.165f) },
        u.hinpct(0.03f),
        0,
        WHITE
    );

    float baseSongEntryHeight = u.hinpct(0.058333f);
    float selectedSongHeightMultiplier = 1.5f;
    float selectedSongEntryHeight = baseSongEntryHeight * selectedSongHeightMultiplier;
    float cumulativeYOffset = u.hpct(0.266666f);

    float scissorHeight = u.hinpct(0.75f);
    if (TheSongList.curSong && TheSongList.curSong->songListPos > 0) {
        float totalListHeight = 0.0f;
        int selectedSongIndex = TheSongList.curSong->songListPos - 1;
        for (int i = TheSongList.SongSelectOffset; i < TheSongList.listMenuEntries.size() && i < TheSongList.SongSelectOffset + 10; i++) {
            if (TheSongList.listMenuEntries[i].hiddenEntry) continue;
            bool isAnimating = (i == selectedSongIndex && i == animatingSongID) || (i == prevAnimatingSongID);
            totalListHeight += isAnimating ? selectedSongEntryHeight : (i == selectedSongIndex ? selectedSongEntryHeight : baseSongEntryHeight);
        }
        scissorHeight = u.hpct(0.266666f) + totalListHeight - u.hpct(0.15f);
        float maxScissorHeight = GetScreenHeight() - u.hpct(0.1475f) - u.hpct(0.15f);
        if (scissorHeight > maxScissorHeight) scissorHeight = maxScissorHeight;
    }

    BeginScissorMode(0, u.hpct(0.15f), u.RightSide - u.winpct(0.25f), scissorHeight);
    for (int i = TheSongList.SongSelectOffset; i < TheSongList.listMenuEntries.size() && i < TheSongList.SongSelectOffset + 10; i++) {
        if (TheSongList.listMenuEntries.size() == i) break;
        float currentEntryHeight = baseSongEntryHeight;
        bool isCurSong = TheSongList.curSong && i == TheSongList.curSong->songListPos - 1;
        bool isDeselecting = i == prevAnimatingSongID && !isCurSong;
        if (isCurSong && i == animatingSongID) {
            float t = (float)(curTime - animationStartTime) / animationDuration;
            if (t < 1.0f) {
                t = EaseInOutQuad(t);
                currentEntryHeight = baseSongEntryHeight + (selectedSongEntryHeight - baseSongEntryHeight) * t;
            } else {
                currentEntryHeight = selectedSongEntryHeight;
                animatingSongID = -1;
            }
        }
        else if (isDeselecting) {
            float t = (float)(curTime - animationStartTime) / animationDuration;
            if (t < 1.0f) {
                t = EaseInOutQuad(t);
                currentEntryHeight = selectedSongEntryHeight - (selectedSongEntryHeight - baseSongEntryHeight) * t;
            } else {
                currentEntryHeight = baseSongEntryHeight;
                prevAnimatingSongID = -1;
            }
        }
        else if (isCurSong) {
            currentEntryHeight = selectedSongEntryHeight;
        }

        if (TheSongList.listMenuEntries[i].isHeader) {
            float songXPos = u.LeftSide + u.winpct(0.005f) - 2;
            float songYPos = cumulativeYOffset;
            DrawRectangle(0, songYPos, (u.RightSide - u.winpct(0.25f)), baseSongEntryHeight, ColorBrightness(AccentColor, -0.75f));
            std::string headerText = TheSongList.listMenuEntries[i].headerChar;
            DrawTextEx(assets.rubikBold, headerText.c_str(), { songXPos, songYPos + u.hinpct(0.0125f) }, u.hinpct(0.035f), 0, WHITE);
            cumulativeYOffset += baseSongEntryHeight;
        } else if (!TheSongList.listMenuEntries[i].hiddenEntry) {
            Font& artistFont = isCurSong ? assets.josefinSansItalic : assets.josefinSansItalic;
            Song& songi = TheSongList.songs[TheSongList.listMenuEntries[i].songListID];
            int songID = TheSongList.listMenuEntries[i].songListID;

            float songXPos = u.LeftSide + u.winpct(0.005f) - 2;
            float songYPos = cumulativeYOffset;
            GuiSetStyle(BUTTON, BORDER_WIDTH, 0);
            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0);
            if (isCurSong) {
                GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(ColorBrightness(AccentColor, -0.4)));
            }
            if (GuiButton(Rectangle{ 0, songYPos, (u.RightSide - u.winpct(0.25f)), currentEntryHeight }, "")) {
                prevAnimatingSongID = TheSongList.curSong ? TheSongList.curSong->songListPos - 1 : -1;
                TheSongList.curSong = &TheSongList.songs[songID];
                animatingSongID = i;
                animationStartTime = curTime;
                ComputeSongTextMetrics(*TheSongList.curSong);
                if (!TheAudioManager.loadedStreams.empty()) {
                    for (auto& stream : TheAudioManager.loadedStreams) {
                        TheAudioManager.StopPlayback(stream.handle);
                    }
                    TheAudioManager.loadedStreams.clear();
                    currentPreviewVolume = 0.0f;
                    previewState = PreviewState::FadeIn;
                }
                if (!TheSongList.songs[songID].ini) {
                    TheSongList.songs[songID].LoadInfo(TheSongList.songs[songID].songInfoPath);
                } else {
                    TheSongList.songs[songID].LoadInfoINI(TheSongList.songs[songID].songInfoPath);
                }
                if (!TheSongList.songs[songID].AlbumArtLoaded) {
                    try {
                        TheSongList.songs[songID].LoadAlbumArt();
                        TheSongList.songs[songID].AlbumArtLoaded = true;
                        SetTextureWrap(TheSongList.songs[songID].albumArtBlur, TEXTURE_WRAP_REPEAT);
                        SetTextureFilter(TheSongList.songs[songID].albumArtBlur, TEXTURE_FILTER_ANISOTROPIC_16X);
                        TraceLog(LOG_DEBUG, "Loaded album art for %s", TheSongList.songs[songID].title.c_str());
                    } catch (const std::exception& e) {
                        TraceLog(LOG_ERROR, "Failed to load album art for %s: %s", TheSongList.songs[songID].title.c_str(), e.what());
                    }
                }
                pendingSongID = songID;
                selectionTime = curTime;
            }
            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);

            int songTitleWidth = (u.winpct(0.25f)) - 6;
            int songArtistWidth = (u.winpct(0.25f)) - 6;
            int songLengthWidth = (u.winpct(0.1f)) - 6;

            float titleFontSize = u.hinpct(0.035f);
            float artistFontSize = u.hinpct(0.025f);
            if (songTextMetrics.find(songi.songListPos) != songTextMetrics.end()) {
                titleFontSize = songTextMetrics[songi.songListPos].titleFontSize;
                artistFontSize = songTextMetrics[songi.songListPos].artistFontSize;
            } else {
                ComputeSongTextMetrics(songi);
                if (songTextMetrics.find(songi.songListPos) != songTextMetrics.end()) {
                    titleFontSize = songTextMetrics[songi.songListPos].titleFontSize;
                    artistFontSize = songTextMetrics[songi.songListPos].artistFontSize;
                }
            }

            float textXOffset = 10;

            auto LightText = Color{ 203, 203, 203, 255 };
            DrawTextEx(
                assets.rubikBold,
                songi.title.c_str(),
                { songXPos + textXOffset, songYPos + (currentEntryHeight - titleFontSize) / 2 },
                titleFontSize,
                0,
                isCurSong ? WHITE : LightText
            );
            DrawTextEx(
                artistFont,
                songi.artist.c_str(),
                { songXPos + textXOffset + songTitleWidth, songYPos + (currentEntryHeight - artistFontSize) / 2 },
                artistFontSize,
                0,
                isCurSong ? WHITE : LightText
            );
            DrawTextEx(
                assets.josefinSansItalic,
                SecondsToTimeFormat(songi.length).c_str(),
                { songXPos + textXOffset + songTitleWidth + songArtistWidth + 10, songYPos + (currentEntryHeight - u.hinpct(0.025f)) / 2 },
                u.hinpct(0.025f),
                0,
                isCurSong ? WHITE : LightText
            );

            cumulativeYOffset += currentEntryHeight;
        }
    }
    EndScissorMode();
    DrawRectangle(AlbumX - AlbumOuter, AlbumY + AlbumHeight, AlbumHeight + AlbumOuter, AlbumHeight + u.hinpct(0.01f), WHITE);
    DrawRectangle(AlbumX - AlbumInner, AlbumY + AlbumHeight, AlbumHeight, u.hinpct(0.075f) + AlbumHeight, GetColor(0x181827FF));
    DrawRectangle(AlbumX - AlbumOuter, AlbumY - AlbumInner, AlbumHeight + AlbumOuter, AlbumHeight + AlbumOuter, WHITE);
    DrawRectangle(AlbumX - AlbumInner, AlbumY, AlbumHeight, AlbumHeight, BLACK);
    if (TheSongList.curSong && TheSongList.curSong->AlbumArtLoaded) {
        DrawTexturePro(
            TheSongList.curSong->albumArt,
            Rectangle{ 0, 0, (float)TheSongList.curSong->albumArt.width, (float)TheSongList.curSong->albumArt.height },
            Rectangle{ (float)AlbumX - AlbumInner, (float)AlbumY, (float)AlbumHeight, (float)AlbumHeight },
            { 0, 0 },
            0,
            WHITE
        );
    } else {
        DrawRectangle(AlbumX - AlbumInner, AlbumY, AlbumHeight, AlbumHeight, DARKGRAY);
    }
    if (TheSongList.SongSelectOffset > 0) {
        std::string SongTitleForCharThingyThatsTemporary = "";
        int songIndex = TheSongList.SongSelectOffset;
        if (TheSongList.listMenuEntries[songIndex].isHeader && songIndex > 0 && !TheSongList.listMenuEntries[songIndex - 1].isHeader) {
            songIndex = TheSongList.SongSelectOffset - 1;
        } else if (!TheSongList.listMenuEntries[songIndex].isHeader) {
            songIndex = TheSongList.SongSelectOffset;
        } else if (songIndex + 1 < TheSongList.listMenuEntries.size() && !TheSongList.listMenuEntries[songIndex + 1].isHeader) {
            songIndex = TheSongList.SongSelectOffset + 1;
        }

        if (songIndex < TheSongList.listMenuEntries.size() && !TheSongList.listMenuEntries[songIndex].isHeader) {
            switch (currentSortValue) {
                case SortType::Title:
                    SongTitleForCharThingyThatsTemporary = TheSongList.songs[TheSongList.listMenuEntries[songIndex].songListID].title.empty() ? "#" : std::string(1, TheSongList.songs[TheSongList.listMenuEntries[songIndex].songListID].title[0]);
                    break;
                case SortType::Artist:
                    SongTitleForCharThingyThatsTemporary = TheSongList.songs[TheSongList.listMenuEntries[songIndex].songListID].artist.empty() ? "#" : std::string(1, TheSongList.songs[TheSongList.listMenuEntries[songIndex].songListID].artist[0]);
                    break;
                case SortType::Source:
                    SongTitleForCharThingyThatsTemporary = TheSongList.songs[TheSongList.listMenuEntries[songIndex].songListID].source.empty() ? "Unknown" : TheSongList.songs[TheSongList.listMenuEntries[songIndex].songListID].source;
                    break;
                case SortType::Length:
                    SongTitleForCharThingyThatsTemporary = TheSongList.listMenuEntries[TheSongList.SongSelectOffset].headerChar;
                    break;
                case SortType::Year:
                    SongTitleForCharThingyThatsTemporary = TheSongList.songs[TheSongList.listMenuEntries[songIndex].songListID].releaseYear.empty() ? "Unknown Year" : TheSongList.songs[TheSongList.listMenuEntries[songIndex].songListID].releaseYear;
                    break;
                default:
                    SongTitleForCharThingyThatsTemporary = "";
                    break;
            }
        }
        if (SongTitleForCharThingyThatsTemporary.empty()) {
            SongTitleForCharThingyThatsTemporary = TheSongList.listMenuEntries[TheSongList.SongSelectOffset].headerChar;
        }
        DrawTextEx(assets.rubikBold, SongTitleForCharThingyThatsTemporary.c_str(), { u.LeftSide + 5, u.hpct(0.218333f) }, u.hinpct(0.035f), 0, WHITE);
    }

    float TextPlacementTB = u.hpct(0.05f);
    float TextPlacementLR = u.LeftSide;
    GameMenu::mhDrawText(assets.redHatDisplayBlack, "MUSIC LIBRARY", { TextPlacementLR, TextPlacementTB }, u.hinpct(0.125f), WHITE, assets.sdfShader, LEFT);

    std::string albumText = SongToDisplayInfo.album.empty() ? "No Album Listed" : SongToDisplayInfo.album;
    std::string yearText = SongToDisplayInfo.releaseYear.empty() ? "Unknown Year" : SongToDisplayInfo.releaseYear;
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
        if (SongToDisplayInfo.parts[i] && SongToDisplayInfo.parts[i]->diff == -1) TintColor = DARKGRAY;
        DrawTexturePro(assets.InstIcons[asdasd], { 0, 0, (float)assets.InstIcons[asdasd].width, (float)assets.InstIcons[asdasd].height }, Placement, { 0, 0 }, 0, TintColor);
        DrawTexturePro(assets.BaseRingTexture, { 0, 0, (float)assets.BaseRingTexture.width, (float)assets.BaseRingTexture.height }, Placement, { 0, 0 }, 0, ColorBrightness(WHITE, 2));
        if (SongToDisplayInfo.parts[i] && SongToDisplayInfo.parts[i]->diff > 0)
            DrawTexturePro(assets.YargRings[SongToDisplayInfo.parts[i]->diff - 1], { 0, 0, (float)assets.YargRings[SongToDisplayInfo.parts[i]->diff - 1].width, (float)assets.YargRings[SongToDisplayInfo.parts[i]->diff - 1].height }, Placement, { 0, 0 }, 0, WHITE);
    }

    GameMenu::DrawBottomOvershell();
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(ColorBrightness(AccentColor, -0.25)));
    if (GuiButton(Rectangle{ u.LeftSide, GetScreenHeight() - u.hpct(0.1475f), u.winpct(0.2f), u.hinpct(0.05f) }, "Play Song")) {
        if (TheSongList.curSong) {
            if (!TheSongList.curSong->ini) {
                TheSongList.curSong->LoadSong(TheSongList.curSong->songInfoPath);
            } else {
                TheSongList.curSong->LoadSongIni(TheSongList.curSong->songDir);
            }
            if (!TheAudioManager.loadedStreams.empty()) {
                for (auto& stream : TheAudioManager.loadedStreams) {
                    TheAudioManager.StopPlayback(stream.handle);
                }
                TheAudioManager.loadedStreams.clear();
            }
            TheMenuManager.SwitchScreen(READY_UP);
        }
    }
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);

    if (GuiButton(Rectangle{ u.LeftSide + u.winpct(0.4f) - 2, GetScreenHeight() - u.hpct(0.1475f), u.winpct(0.2f), u.hinpct(0.05f) }, "Sort")) {
        int selectedSongIndex = -1;
        if (TheSongList.curSong) {
            for (size_t i = 0; i < TheSongList.songs.size(); i++) {
                if (&TheSongList.songs[i] == TheSongList.curSong) {
                    selectedSongIndex = i;
                    break;
                }
            }
        }
        prevAnimatingSongID = TheSongList.curSong ? TheSongList.curSong->songListPos - 1 : -1;
        currentSortValue = NextSortType(currentSortValue);
        TheSongList.sortList(currentSortValue, selectedSongIndex);
        if (selectedSongIndex >= 0 && selectedSongIndex < TheSongList.songs.size()) {
            TheSongList.curSong = &TheSongList.songs[selectedSongIndex];
            TheSongList.SongSelectOffset = TheSongList.curSong->songListPos - 5;
            if (TheSongList.SongSelectOffset < 1) TheSongList.SongSelectOffset = 1;
            if (TheSongList.SongSelectOffset > TheSongList.listMenuEntries.size() - 10)
                TheSongList.SongSelectOffset = TheSongList.listMenuEntries.size() - 10;
            if (!TheSongList.curSong->AlbumArtLoaded) {
                try {
                    TheSongList.curSong->LoadAlbumArt();
                    TheSongList.curSong->AlbumArtLoaded = true;
                    SetTextureWrap(TheSongList.curSong->albumArtBlur, TEXTURE_WRAP_REPEAT);
                    SetTextureFilter(TheSongList.curSong->albumArtBlur, TEXTURE_FILTER_ANISOTROPIC_16X);
                } catch (const std::exception& e) {
                }
            }
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
            animatingSongID = TheSongList.curSong->songListPos - 1;
            animationStartTime = curTime;
            ComputeSongTextMetrics(*TheSongList.curSong);
        }
    }
    if (GuiButton(Rectangle{ u.LeftSide + u.winpct(0.2f) - 1, GetScreenHeight() - u.hpct(0.1475f), u.winpct(0.2f), u.hinpct(0.05f) }, "Back")) {
        if (!TheAudioManager.loadedStreams.empty()) {
            for (auto& stream : TheAudioManager.loadedStreams) {
                TheAudioManager.StopPlayback(stream.handle);
            }
            TheAudioManager.loadedStreams.clear();
        }
        TheMenuManager.SwitchScreen(MAIN_MENU);
    }
    DrawOvershell();
}
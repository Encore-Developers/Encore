//
// Created by maria on 29/05/2026.
//

#include "Jukebox.h"

#include "settings/settings.h"
#include "song/audio.h"
#include "song/songlist.h"


void Encore::Jukebox::TogglePlayback() {
    if (streamsLoaded) {
        if (playing) {
            TheAudioManager.pauseStreams();
            playing = false;
        } else {
            TheAudioManager.playStreams();
            playing = true;
        }
    }
}

void Encore::Jukebox::UnloadStreams() {
    if (streamsLoaded) {
        TheAudioManager.unloadStreams();
        streamsLoaded = false;
        playing = false;
    }
}

void Encore::Jukebox::Update() {
    if (streamsLoaded) {
        float played = TheAudioManager.GetMusicTimePlayed();
        float length = TheAudioManager.GetMusicTimeLength();
        AdjustVolume();
        if (played >= length - 0.5) {
            UnloadStreams();
            PickRandomSong();
            LoadStreams();
            StartStreams();
        }
    } else {
        PickRandomSong();
        LoadStreams();
        StartStreams();
    }
}

void Encore::Jukebox::AdjustVolume() {
    if (streamsLoaded) {
        for (int i = 0; i < TheAudioManager.loadedStreams.size(); i++) {
            float Volume =
                TheGameSettings.avMainVolume * TheGameSettings.avMenuMusicVolume;
            if (TheAudioManager.loadedStreams[i].instrument == PartVocals)
                Volume = 0;
            TheAudioManager.SetAudioStreamVolume(
                TheAudioManager.loadedStreams[i].handle, Volume
            );
        }
    }
}

void Encore::Jukebox::PickRandomSong() {
    if (TheSongList.songs.size() > 0) {
        try {
            int my = GetRandomValue(0, (int)TheSongList.songs.size() - 1);
            TheSongList.curSong = &TheSongList.songs[my];
            // ChosenSongInt = my;

            TheSongList.curSong->LoadAlbumArt();
            TraceLog(LOG_INFO, "Jukebox: Picked song");
            TraceLog(LOG_INFO, (TheSongList.curSong->title + " - " + TheSongList.curSong->artist).c_str());
        } catch (const std::exception &e) {
            TraceLog(LOG_ERROR, e.what());
        };
    }
}

void Encore::Jukebox::StartStreams() {
    if (streamsLoaded) {
        TheAudioManager.BeginPlayback(TheAudioManager.loadedStreams[0].handle);
        playing = true;
    }
}

void Encore::Jukebox::LoadStreams() {
    if (!streamsLoaded && TheSongList.curSong) {
        TheAudioManager.loadStreams(TheSongList.curSong->LoadAudioINI());
        streamsLoaded = true;
        AdjustVolume();
    }
}

void Encore::Jukebox::FirstLoad() {
    TheGameJukebox.PickRandomSong();
    TheGameJukebox.LoadStreams();
    TheGameJukebox.StartStreams();
}

void Encore::Jukebox::SkipSong() {
    TheGameJukebox.UnloadStreams();
    TheGameJukebox.PickRandomSong();
    TheGameJukebox.LoadStreams();
    TheGameJukebox.StartStreams();
}

void Encore::Jukebox::StartPlayback() {
    TheGameJukebox.LoadStreams();
    TheGameJukebox.StartStreams();
}
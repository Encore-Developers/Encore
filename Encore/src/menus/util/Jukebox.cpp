//
// Created by maria on 29/05/2026.
//

#include "Jukebox.h"

#include "settings/settings.h"
#include "song/audio.h"
#include "song/songlist.h"


void Jukebox::TogglePlayback() {
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

void Jukebox::UnloadStreams() {
    if (streamsLoaded) {
        TheAudioManager.unloadStreams();
        streamsLoaded = false;
        playing = false;
    }
}

void Jukebox::Update() {
    if (streamsLoaded) {
        float played = TheAudioManager.GetMusicTimePlayed();
        float length = TheAudioManager.GetMusicTimeLength();
        AdjustVolume();
        TheAudioManager.UpdateAudioStreamVolumes();
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

void Jukebox::AdjustVolume() {
    if (streamsLoaded) {
        for (auto &stream : TheAudioManager.loadedStreams) {
            float Volume =
                TheGameSettings.avMainVolume * TheGameSettings.avMenuMusicVolume;
            if (stream.instrument == AudioManager::Stems::Vocals)
                Volume = 0;
            TheAudioManager.SetAudioStreamVolume(stream.instrument, Volume);
        }
    }
}

void Jukebox::PickRandomSong() {
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

void Jukebox::StartStreams() {
    if (streamsLoaded) {
        TheAudioManager.BeginPlayback(TheAudioManager.loadedStreams[0].handle);
        playing = true;
    }
}

void Jukebox::LoadStreams() {
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
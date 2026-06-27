//
// Created by maria on 29/05/2026.
//

#include "Jukebox.h"

#include <random>

#include "settings/settings.h"
#include "song/audio.h"
#include "song/songlist.h"
using namespace Encore;

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
    TheAudioManager.unloadStreams();
    streamsLoaded = false;
    playing = false;
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

            std::random_device seed;
            std::minstd_rand prng(seed());
            std::uniform_int_distribution<> dist(0, TheSongList.songs.size() - 1);
            int selectedSong = dist(prng);
            if (TheSongList.curSong == &TheSongList.songs[selectedSong]) {
                selectedSong = dist(prng);
            }
            TheSongList.curSong = &TheSongList.songs[selectedSong];
            // ChosenSongInt = my;

            TheSongList.curSong->LoadAlbumArt();
            Log::Info("Jukebox: Picked song");
            Log::Error("{} - {}", TheSongList.curSong->title, TheSongList.curSong->artist);
        } catch (const std::exception &e) {
            Log::Error("Jukebox failed! {}", e.what());
        };
    }
}

void Jukebox::StartStreams() {
    if (!TheAudioManager.loadedStreams.empty()) {
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
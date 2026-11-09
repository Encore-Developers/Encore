
#include "enctime.h"

#include "settings/settings.h"
#include "song/songlist.h"

#include <cmath>
#include "song/audio.h"

#include <regex>

SongTime TheSongTime;

Encore::RhythmEngine::LyricPhrase &SongTime::GetCurrentLyric() {
    return Lyrics.at(CurrentLyricPhrase);
}
Encore::RhythmEngine::LyricPhrase *SongTime::GetNextLyric() {
    if (CurrentLyricPhrase < Lyrics.size() - 1) {
        return &Lyrics.at(CurrentLyricPhrase + 1);
    }
    return nullptr;
}
Encore::RhythmEngine::LyricPhrase *SongTime::GetPreviousLyric() {
    if (CurrentLyricPhrase > 0) {
        return &Lyrics.at(CurrentLyricPhrase - 1);
    }
    return nullptr;
}


void SongTime::SetOffset(double audioCalibration) {
    aCalib = audioCalibration;
};

void SongTime::Start(double end) {
    endTime = end;
    lastTimeSample = GetTime();
}

// Resets internal state, 
void SongTime::Reset() {
    pauseTime = 0.0;
    running = false;
    paused = false;
}

// Bad.
void SongTime::FullReset() {
    Reset();
    Lyrics.clear();
    CurrentLyricPhrase = 0;
}

void SongTime::Pause() {
    pauseTime = GetElapsedTime();
    TheAudioManager.pauseStreams();
    paused = true;
};

void SongTime::Resume() {
    TheAudioManager.seekStreams(pauseTime - 5 + aCalib);
    TheAudioManager.unpauseStreams();
    paused = false;
};

void SongTime::Stop() {
    running = false;
    paused = false;
    TheAudioManager.unloadStreams();
}

bool SongTime::Running() {
    return running;
}

double SongTime::GetElapsedTime() {
    double audioTime = TheAudioManager.GetMusicTimePlayed();
    if (audioTime >= TheAudioManager.GetMusicTimeLength()) {
        return TheAudioManager.GetMusicTimeLength() + (GetTime() - lastTimeSample);
    }
    lastTimeSample = GetTime();
    return audioTime - aCalib + TheGameSettings.VideoOffset/1000.0;
};
double SongTime::GetStartTime() {
    return 0;
}

// this is actually a lie. it returns "the system time when it ends" i think i forgort
// use GetSongLength if you need song duration
double SongTime::GetEndTime() {
    return endTime;
}

double SongTime::GetSongLength() {
    return endTime;
}

bool SongTime::SongComplete() {
    if (running) {
        return GetElapsedTime() > endTime;
    }
    return false;
}
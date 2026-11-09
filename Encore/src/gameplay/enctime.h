#pragma once
//
// Created by marie on 20/09/2024.
//

#include "RhythmEngine/TempoMap.h"
#include "midifile/MidiFile.h"

#include <vector>

#include "RhythmEngine/events/EncEvents/EncChartEvents.h"
#include "song/song.h"
#include "RhythmEngine/timetypes.h"



struct Section {
    std::string name;
    TimeSeconds start;
    TimeTicks tickStart;
    int notes = 0;
    int hit = 0;
    int perfects = 0;
    int overhits = 0;
};

class SongTime {
private:
    double aCalib = 0.0;
    TimeSeconds startTime = 0.0;
    TimeSeconds fakeStartTime = 0.0;
    TimeSeconds endTime = 0.0;
    TimeSeconds pauseTime = 0.0;
    TimeSeconds lastTimeSample = 0.0;
    bool running = false;
    bool paused = false;

public:
    std::shared_ptr<Encore::RhythmEngine::TempoMap> currentTempoMap;

    std::vector<Section> Sections {};
    size_t CurrentLyricPhrase = 0;
    std::vector<Encore::RhythmEngine::LyricPhrase> Lyrics {};
    // GetCurrentLyric will always be in bounds, and is currently wrapped by an Empty check
    // where it is used. GetPreviousLyric and GetNextLyric return null if out of bounds.
    Encore::RhythmEngine::LyricPhrase& GetCurrentLyric();
    Encore::RhythmEngine::LyricPhrase* GetPreviousLyric();
    Encore::RhythmEngine::LyricPhrase* GetNextLyric();

    SongTime() = default;

    // Start the timer
    void SetOffset(double audioCalibration);

    // TODO: implement pausing
    // TODO: reset after songs
    void Reset();
    void FullReset();
    void Start(double end);
    void Pause();
    void Resume();
    void Stop();
    TimeSeconds GetElapsedTime();
    TimeSeconds GetStartTime();
    TimeSeconds GetEndTime();
    TimeSeconds GetSongLength();
    TimeSeconds GetFakeStartTime();
    bool Running();
    bool SongComplete();
};

extern SongTime TheSongTime;

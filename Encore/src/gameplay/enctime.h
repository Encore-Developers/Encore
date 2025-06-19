#pragma once
//
// Created by marie on 20/09/2024.
//

#include "raylib.h"
#include "midifile/MidiFile.h"

#include <vector>

struct BPM {
    double time;
    double bpm;
    int tick;
};

struct TimeSig {
    double time;
    int numer;
    int denom;
    int tick;
};
enum BeatType {
    Major = 1,
    Minor = 0,
    Measure = 2
};

struct Beatline {
    double time;
    int tick;
    int type = Major;
};
class SongTime {
private:
    double aCalib = 0.0;
    double startTime = 0.0;
    double fakeStartTime = 0.0;
    double endTime = 0.0;
    double pauseTime = 0.0;
    bool running = false;
    bool paused = false;

public:
    std::vector<BPM> BPMChanges {};
    int CurrentBPM;
    std::vector<TimeSig> TimeSigChanges {};
    int CurrentTimeSig;
    std::vector<Beatline> Beatlines {};
    int CurrentBeatline;

    SongTime() = default;

    double LastTick = 0;
    double CurrentTick = 0;
    void BeatmapFromMidiTrack(smf::MidiFile &midiFile, int TrackID, int songEndTick);

    void UpdateTick();
    [[nodiscard]] double GetCurrentTick() const;
    [[nodiscard]] double GetLastTick() const;
    void GenerateBeatmap(int songEndTick);
    void CreateBeatlines(TimeSig timeSig, int tickStart, int tickEnd);
    // Start the timer
    void SetOffset(double audioCalibration);

    // TODO: implement pausing
    // TODO: reset after songs
    void Reset();
    void Start(double end);
    void Start(double start, double end);
    void Pause();
    void Resume();
    void Stop();
    double GetElapsedTime();
    double GetStartTime();
    double GetEndTime();
    double GetSongLength();
    double GetFakeStartTime();
    bool Running();
    bool SongComplete();
};

extern SongTime TheSongTime;

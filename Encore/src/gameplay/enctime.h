#pragma once
//
// Created by marie on 20/09/2024.
//

#include "raylib.h"
#include "midifile/MidiFile.h"

#include <vector>

struct BPM {
    BPM(double _time, int _bpm, int _tick) : time(_time), bpm(_bpm), tick(_tick) {};
    double time;
    double bpm;
    int tick;
};

struct TimeSig {
    TimeSig(double _time, int _numer, int _denom, int _pow, int _tick)
        : time(_time), numer(_numer), denom(_denom), pow(_pow), tick(_tick) {};
    double time;
    int numer;
    int denom;
    int pow;
    int tick;
};
// these are defined by position in BEAT or made off of the chart's beatmap if no BEAT
// exists i think it should just be position? theres no real reason to have length i think
// maybe the tick its at? but that doesnt matter since its like. a timeline.
// the "delta" would just be this "tick" + the current % of time between this tick and the
// next, if the next exists in fact i think this could just be a vector of double
struct OverdriveTick {
    OverdriveTick(double _time, int _tick) : time(_time), tick(_tick) {};
    double time;
    int tick;
};
enum BeatType {
    Major = 1,
    Minor = 0,
    Measure = 2
};

struct Beatline {
    Beatline(double _time, int _tick, int _type)
        : time(_time), tick(_tick), type(_type) {};
    double time;
    int tick;
    int type = Major;
};

struct Section {
    std::string name;
    double start;
};

class SongTime {
private:
    double aCalib = 0.0;
    double startTime = 0.0;
    double fakeStartTime = 0.0;
    double endTime = 0.0;
    double pauseTime = 0.0;
    double lastTimeSample = 0.0;
    bool running = false;
    bool paused = false;

public:
    std::vector<OverdriveTick> OverdriveTicks {};
    int CurrentODTickItr;
    std::vector<BPM> BPMChanges {};
    int CurrentBPM;
    std::vector<TimeSig> TimeSigChanges {};
    int CurrentTimeSig;
    std::vector<Beatline> Beatlines {};
    int CurrentBeatline;

    std::vector<Section> Sections {};

    SongTime() = default;
    void GenerateOverdriveTicks(smf::MidiFile &midiFile, int TrackID);
    void UpdateOverdriveTick();
    void ParseSections(smf::MidiFile);
    double LastTick = 0;
    double CurrentTick = 0;
    double LastODTick = 0;
    double CurrentODTick = 0;
    void BeatmapFromMidiTrack(smf::MidiFile &midiFile, int songEndTick);

    void UpdateTick();
    [[nodiscard]] double GetCurrentTick() const;
    [[nodiscard]] double GetLastTick() const;
    static double TimeRangeToTickDelta(double timeStart, double timeEnd, const BPM &bpm);
    void GenerateBeatmap(int songEndTick);
    static double TickRangeToTimeDelta(int tickStart, int tickEnd, const BPM &currentBPM);
    static double TimeSinceBPMStart(BPM bpm, int endTick);
    void CreateBeatlines(TimeSig timeSig, int tickStart, int tickEnd, int &curTempo);
    // Start the timer
    void SetOffset(double audioCalibration);

    // TODO: implement pausing
    // TODO: reset after songs
    void Reset();
    void Start(double end);
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

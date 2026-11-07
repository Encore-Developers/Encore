#pragma once
//
// Created by marie on 20/09/2024.
//

#include "midifile/MidiFile.h"

#include <vector>

#include "RhythmEngine/events/EncEvents/EncChartEvents.h"
#include "song/song.h"
#include "RhythmEngine/timetypes.h"

struct BPM {
    BPM(TimeSeconds _time, double _bpm, TimeTicks _tick) : time(_time), bpm(_bpm), tick(_tick) {};
    TimeSeconds time;
    double bpm;
    TimeTicks tick;
};

struct TimeSig {
    TimeSig(TimeSeconds _time, int _numer, int _denom, int _pow, TimeTicks _tick)
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

enum BeatType {
    Major = 1,
    Minor = 0,
    Measure = 2
};

struct Beatline {
    Beatline(TimeSeconds _time, int _tick, TimeTicks _type)
        : time(_time), tick(_tick), type(_type) {};
    TimeSeconds time;
    TimeTicks tick;
    int type = Major;
};

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
    int songPPQN = 480;

    std::vector<BPM> BPMChanges {};
    size_t CurrentBPM;
    std::vector<TimeSig> TimeSigChanges {};
    size_t CurrentTimeSig;
    std::vector<Beatline> Beatlines {};
    size_t CurrentBeatline;

    std::vector<Encore::RhythmEngine::TimePoint> OverdriveTicks;

    std::vector<Section> Sections {};
    size_t CurrentLyricPhrase = 0;
    std::vector<Encore::RhythmEngine::LyricPhrase> Lyrics {};
    // GetCurrentLyric will always be in bounds, and is currently wrapped by an Empty check
    // where it is used. GetPreviousLyric and GetNextLyric return null if out of bounds.
    Encore::RhythmEngine::LyricPhrase& GetCurrentLyric();
    Encore::RhythmEngine::LyricPhrase* GetPreviousLyric();
    Encore::RhythmEngine::LyricPhrase* GetNextLyric();

    SongTime() = default;
    void ParseSections(Song* song, smf::MidiFile&);
    double LastTick = 0;
    double CurrentTick = 0;
    void BeatmapFromMidiTrack(smf::MidiFile &midiFile, int songEndTick);

    double GetBeatlineDelta();
    void UpdateBeatlines();

    void UpdateTick();
    TimeBeats SecondsToBeat(TimeSeconds seconds);
    TimeSeconds BeatToSeconds(TimeBeats beat);
    TimeODTicks SecondsToODTick(TimeSeconds seconds) const;
    TimeSeconds ODTickToSeconds(TimeODTicks odTick) const;
    [[nodiscard]] double GetCurrentTick() const;
    [[nodiscard]] double GetLastTick() const;
    double TimeRangeToTickDelta(double timeStart, double timeEnd, const BPM &bpm);
    void GenerateBeatmap(int songEndTick);
    static double TickRangeToTimeDelta(int tickStart, int tickEnd, const BPM &currentBPM);
    static double TimeSinceBPMStart(BPM bpm, int endTick);
    void CreateBeatlines(TimeSig timeSig, int tickStart, int tickEnd, int &curTempo);
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

#pragma once

#include "TempoMap.h"

#include <vector>

#include "RhythmEngine/events/EncEvents/EncChartEvents.h"
#include "song/song.h"
#include "RhythmEngine/timetypes.h"

namespace Encore::RhythmEngine {
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

    class TempoMap;

    class BeatlineTracker {
    public:
        TempoMap* tempoMap;
        size_t currentBeatline = 0;

        void Advance(TimeSeconds time);
        Beatline GetBeatline(size_t index);
        size_t GetBeatlineCount();
    };

    class TempoMap {
    public:
        int ppqn = 480;

        std::vector<BPM> BPMChanges {};
        std::vector<TimeSig> TimeSigChanges {};
        std::vector<Beatline> Beatlines {};
        std::vector<TimePoint> OverdriveTicks {};

        TimeBeats SecondsToBeat(TimeSeconds seconds);
        TimeSeconds BeatToSeconds(TimeBeats beat);
        TimeODTicks SecondsToODTick(TimeSeconds seconds) const;
        TimeSeconds ODTickToSeconds(TimeODTicks odTick) const;

        BeatlineTracker GetBeatlineTracker();

        double GetBeatDelta(TimeSeconds time, int step = 1);

        void GenerateBeatmap(int songEndTick);
        void CreateBeatlines(TimeSig timeSig, int tickStart, int tickEnd, int &curTempo);
    };
}


#pragma once
#include "midifile/MidiFile.h"

namespace Encore::RhythmEngine {
    struct OverdriveTick {
        OverdriveTick(double _time, int _tick) : time(_time), tick(_tick) {};
        double time;
        int tick;
    };

    class OverdriveTicks
    {
        bool compareTicks(const OverdriveTick &a, const OverdriveTick &b) {
            return a.tick < b.tick;
        }
        bool equalTicks(const OverdriveTick &a, const OverdriveTick &b) {
            return a.tick == b.tick;
        }

        std::vector<OverdriveTick> ticks {};
        size_t CurrentODTickItr = 0;

    public:
        void GenerateOverdriveTicks(smf::MidiFile &midiFile, int TrackID);
        void UpdateOverdriveTick();
        double LastODTick = 0;
        double CurrentODTick = 0;
    };
}
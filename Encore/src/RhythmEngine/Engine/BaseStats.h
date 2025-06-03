//
// Created by maria on 01/06/2025.
//

#ifndef BASESTATS_H
#define BASESTATS_H
#include <array>

namespace Encore::RhythmEngine {
    enum class StrumState {
        Default = 0,
        UpStrum = 1,
        DownStrum = 2
    };
    /**
     * @brief BaseStats is the default base class for handling statistics
     * that Encore needs to keep track of. This is basic gameplay information,
     * like scores, overdrive fills, etc.
     *
     * Individual engine information, like ghost count, notes tagged by FretAfterStrum,
     * lifts or taps to be hit by Overdrive, and other such events are stored here.
     *
     * Not to be confused with handling current chart events. Those are handled
     * by their respective chart event vectors.
     *
     * Note: the LaneCount size template is SPECIFICALLY for how many buttons are
     * needed for the instrument being played. Guitar, despite being 1 lane, NEEDS 5
     * buttons.
     */
    template <size_t LaneCount>
    class BaseStats {
    public:
        explicit BaseStats(const int BaseScore) { StarCalcBaseScore = BaseScore; };
        virtual ~BaseStats() = default;
        double OverdriveFill = 0.0;
        double OverdriveActivationTime = 0.0;
        double OverdriveActivationTick = 0.0;
        bool OverdriveActive = false;

        double Score = 0;
        int Combo = 0;
        int PerfectHits = 0;
        int NotesHit = 0;
        int Misses = 0;
        int AttemptedNotes = 0;
        bool AudioMuted = false;
        int StarCalcBaseScore;
        double InputTime = -1;
        double InputOffset = 0;
        bool Paused = false;
        double Health = 1.0;
        void HitNote(int chordSize);
        void Overhit();
        int multiplier() const;
        std::array<bool, LaneCount> HeldFrets = {};
    };

}

#endif // BASESTATS_H

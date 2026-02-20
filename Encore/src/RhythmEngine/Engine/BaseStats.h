//
// Created by maria on 01/06/2025.
//

#ifndef BASESTATS_H
#define BASESTATS_H
#include <array>
#include <vector>
#include "Overdrive.h"
#include "song/scoring.h"

namespace Encore::RhythmEngine {

    enum class StrumState {
        Default = 0,
        UpStrum = 1,
        DownStrum = 2
    };
    enum StatsType {
        Pad = 1,
        Guitar = 0,
        Drums = 2
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
        explicit BaseStats(const int BaseScore) {
            StarCalcBaseScore = BaseScore;
        };
        virtual ~BaseStats() = default;


        int Type = 0;
        double Score = 0;
        int Combo = 0;
        int PerfectHits = 0;
        int NotesHit = 0;
        int Misses = 0;
        int AttemptedNotes = 0;
        int Overhits = 0;
        int MaxCombo = 0;
        bool AudioMuted = false;
        int StarCalcBaseScore;
        double InputTime = -1;
        double InputOffset = 0;
        double LastPerfectTime = -1;
        bool SixMultiplier = false;
        bool Paused = false;
        bool Bot = false;
        double Health = 1.0;
        int Stars = 0;
        double StarThresholdValue = 0.0;
        StrumState strumState = StrumState::Default;
        Overdrive overdrive;
        void HitNote(int chordSize, bool perfect) {
            Combo++;
            const double PointsPerNote = BASE_NOTE_POINT * (perfect ? PERFECT_MULTIPLIER : 1.0);
            Score += (PointsPerNote * chordSize) * multiplier();
            if (perfect) PerfectHits++;
            // PerfectHits = 0;
            NotesHit++;
            AttemptedNotes++;
            AudioMuted = false;
        };
        void MissNote() {
            if (Combo > MaxCombo) MaxCombo = Combo;
            Combo = 0;
            Misses++;
            AttemptedNotes++;
            AudioMuted = true;
        };
        void Overhit() {
            Overhits++;
            if (Combo > MaxCombo) MaxCombo = Combo;
            Combo = 0;
            AudioMuted = true;
        };
        [[nodiscard]] int multiplier() const {
            int od = overdrive.Active ? 2 : 1;
            // if (IsBassOrVox()) {
            //     if (Combo >= 50)
            //         return 6 * od;

            //} else {
            int MaxMult = SixMultiplier ? 6 : 4;
            int Multiplier = (Combo / 10) + 1;
            if (Multiplier > MaxMult) {
                Multiplier = MaxMult;
            }
            return Multiplier * od;

            // if (Combo >= 30)
            //     return MaxMult * od;
            // };
            // return (Combo / 10) + 1 * od;
        };
        [[nodiscard]] int multNoOD() const {
            int MaxMult = SixMultiplier ? 6 : 4;
            int Multiplier = (Combo / 10) + 1;
            if (Multiplier > MaxMult) {
                Multiplier = MaxMult;
            }
            return Multiplier;
        };
        std::array<bool, LaneCount> HeldFrets = {};
    };

}

#endif // BASESTATS_H
//
// Created by maria on 01/06/2025.
//

#ifndef BASESTATS_H
#define BASESTATS_H
#include <array>
#include <vector>
#include "Overdrive.h"
#include "settings/settings.h"
#include "song/audio.h"
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
        bool CanHitHopo = true;
        bool Paused = false;
        bool Bot = false;
        double Health = 1.0;
        double Accuracy = -1;
        double LastHitAccuracy = -1;
        int Stars = 0;
        double StarThresholdValue = 0.0;
        StrumState strumState = StrumState::Default;
        Overdrive overdrive;
        void HitNote(int chordSize, int perfect) {
            if (perfect == -1) {
                MaxCombo = Combo;
                Combo = 0;
            } else {
                Combo++;
                MaxCombo = Combo;
            }
            const double PointsPerNote = BASE_NOTE_POINT * (perfect == 1 ? PERFECT_MULTIPLIER : 1.0);
            Score += (PointsPerNote * chordSize) * multiplier();
            if (perfect == 1) PerfectHits++;
            // PerfectHits = 0;
            NotesHit++;
            AttemptedNotes++;
            CanHitHopo = true;
            AudioMuted = false;
        };
        void MissNote() {
            TheAudioManager.playSample("miss", TheGameSettings.avMainVolume * TheGameSettings.avSoundEffectVolume);
            if (Combo > MaxCombo) MaxCombo = Combo;
            Combo = 0;
            Misses++;
            AttemptedNotes++;
            AudioMuted = true;
            CanHitHopo = false;
        };
        void Overhit() {
            TheAudioManager.playSample("miss", TheGameSettings.avMainVolume * TheGameSettings.avSoundEffectVolume);
            Overhits++;
            if (Combo > MaxCombo) MaxCombo = Combo;
            Combo = 0;
            AudioMuted = true;
            CanHitHopo = false;
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
        float ComboFillCalc() {
            if (Combo == 0) {
                return 0;
            }
            int MaxMultCombo = SixMultiplier ? 50 : 30;
            int ComboMod = Combo % 10;
            if (Combo >= MaxMultCombo || ComboMod == 0) {
                return 1.0f;
            }
            return (static_cast<float>(ComboMod) / 10.0f);
        }
        std::array<bool, LaneCount> HeldFrets = {};
    };

}

#endif // BASESTATS_H
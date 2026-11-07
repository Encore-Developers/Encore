//
// Created by maria on 01/06/2025.
//

#ifndef BASESTATS_H
#define BASESTATS_H
#include <array>
#include "assets.h"
#include "settings/settings.h"
#include "song/audio.h"
#include "../scoring.h"
#include "RhythmEngine/timingvalues.h"

namespace Encore::RhythmEngine {
    class BaseChart;
    struct NoteAccuracy {
        double time;
        double offset;
        bool miss;
        explicit NoteAccuracy(const double time, const double offset, const bool miss)
            : time(time), offset(offset), miss(miss) {}
    };

    struct OverdriveEP {
        int duration = 32; // name this better PLEASE
        int maxCharges = 4;
        int minCharges = 1;

        double DrainPerTick() const {
            return 1 / (double)duration;
        }
    };

    // i just made some BULLSHIIIIIIIIIIIT
    struct EngineParameters {
        struct Multiplier {
            double max = 4;
            int notesPerMult = 10;
            double odMult = 2;
            int comboForMax() const { return (max - 1) * notesPerMult; }
        } mult;
        struct Health {
            float cbMult = 8.0f;
            float starting = 0.75f;
            float Gain = 0.015f;
            float Loss = 0.03f;
            float odMult = 2.0f;
            float pass = 0.75f;
            float hurtThreshold = (1.0f / 3.0f);
        } health;
        OverdriveEP od;
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
     */
    class Stats {
    public:
        explicit Stats(const size_t laneCount = 5) {
            HeldFrets.resize(laneCount, false);
        };
        virtual ~Stats() = default;

        void AddHealth(const float acc) {
            float delta = acc * ep.health.Gain;
            if (acc < ep.health.hurtThreshold)
                delta = -(ep.health.Gain * (1.0 - acc));
            float mult = false ? ep.health.odMult : 1; // TODO: overdrive
            Health += delta * mult;
            if (Health > 1)
                Health = 1;
        }

        void RemoveHealth(const bool comboBreak) {
            double mult = 1;
            if (comboBreak)
                mult = ep.health.cbMult;
            else if (false) // TODO: overdrive
                mult = ep.health.odMult;
            Health -= ep.health.Loss * mult;
            if (Health <= 0) {
                Health = 0;
                Failed = true;
            }
        }

        EngineParameters ep;
        int Type = 0;
        double PointsPerNote = 0;
        double weightedCount = 0;
        double Score = 0;
        int Combo = 0;
        int PerfectHits = 0;
        int NotesHit = 0;
        int Misses = 0;
        int AttemptedNotes = 0;
        int Overhits = 0;
        int MaxCombo = 0;
        double TotalOffset = 0;
        bool AudioMuted = false;
        double LastPerfectTime = -1;
        bool CanHitHopo = true;
        bool Bot = false;
        double Health = defaultHealth;
        bool Failed = false;
        double Accuracy = 0;
        std::vector<NoteAccuracy> accuracies;
        // void HitNote(int chordSize, int perfect) {
        //     if (perfect == -1) {
        //         Combo = 0;
        //     } else {
        //         Combo++;
        //     }
        //     if (Combo > MaxCombo)
        //         MaxCombo = Combo;
        //     Score += (PointsPerNote * chordSize) * multiplier();
        //     if (perfect == 1)
        //         PerfectHits++;
        //     // PerfectHits = 0;
        //     NotesHit++;
        //     AttemptedNotes++;
        //     CanHitHopo = true;
        //     AudioMuted = false;
        // };
        // void MissNote() {
        //     if (Combo > MaxCombo)
        //         MaxCombo = Combo;
        //     Combo = 0;
        //     Misses++;
        //     AttemptedNotes++;
        //     AudioMuted = true;
        //     CanHitHopo = false;
        // };
        // void Overhit() {
        //     TheAudioManager.playSample(
        //         ASSET(missSound),
        //         TheGameSettings.avMainVolume * TheGameSettings.avSoundEffectVolume
        //     );
        //     Overhits++;
        //     if (Combo > MaxCombo)
        //         MaxCombo = Combo;
        //     Combo = 0;
        //     AudioMuted = true;
        //     CanHitHopo = false;
        // };
        // [[nodiscard]] double multiplier() const {
        //     double od = overdrive.Active ? ep.mult.odMult : 1;
        //     // if (IsBassOrVox()) {
        //     //     if (Combo >= 50)
        //     //         return 6 * od;
        //
        //     //} else {
        //     //int MaxMult = SixMultiplier ? 6 : 4;
        //     double Multiplier = (Combo / ep.mult.notesPerMult) + 1;
        //     if (Multiplier > ep.mult.max) {
        //         Multiplier = ep.mult.max;
        //     }
        //     return Multiplier * od;
        //
        //     // if (Combo >= 30)
        //     //     return MaxMult * od;
        //     // };
        //     // return (Combo / 10) + 1 * od;
        // };
        [[nodiscard]] double ComboToMultiplier(int combo) const {
            double Multiplier = (combo / ep.mult.notesPerMult) + 1;
            if (Multiplier > ep.mult.max) {
                Multiplier = ep.mult.max;
            }
            return Multiplier;
        };
        float GetMultiplierProgress() const {
            if (Combo == 0) {
                return 0;
            }
            int MaxMultCombo = ep.mult.notesPerMult * (ep.mult.max - 1);
            int ComboMod = Combo % ep.mult.notesPerMult;
            if (Combo >= MaxMultCombo || ComboMod == 0) {
                return 1.0f;
            }

            return (static_cast<float>(ComboMod) / static_cast<float>(ep.mult.notesPerMult));
        }
        double GetAccuracyScore() const {
            if (AttemptedNotes == 0) {
                return 1;
            }
            return Accuracy / AttemptedNotes;
        }
        std::vector<bool> HeldFrets = {};
        [[nodiscard]] uint8_t HeldFretsArrayToMask() const;
    };

}

#endif // BASESTATS_H
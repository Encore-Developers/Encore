#pragma once
//
// Created by maria on 01/06/2025.
//

#include "Stats.h"
#include "RhythmTimer.h"
#include "../Chart/NoteVector.h"
#include "events/Event.h"

#include <memory>
#include <unordered_map>

#include "users/player.h"
#include "util/Input.h"

class Player;

namespace Encore::RhythmEngine {
    enum HitState {
        HitNote = 0,
        OverhitNote = 1,
        CheckNextInput = 2
    };

    struct Sustain {
        NoteEvent* note;
        TimeBeats start;
        TimeBeats end;
        double multiplier;
        double valuePerBeat;

        double GetValueAtTime(TimeBeats time) const;
    };

    class BaseEngine : public EventSource {
    public:
        BaseEngine(auto _chart, Player* _player)
            : chart(_chart), player(_player) {
            for (auto& lane : chart->Lanes) noteCount += lane.size();
            stats = std::make_shared<Stats>();
            stats->accuracies.reserve(noteCount);
        };

        ~BaseEngine() override = default;
        void CalculateWeightedScore();
        [[nodiscard]] bool EarlyStrike(double noteStartTime) const;
        [[nodiscard]] bool InHitwindow(double noteStartTime) const;
        [[nodiscard]] bool PerfectHit(double noteStartTime) const;
        void ProcessInput(ControllerEvent &event);
        /*
         * Before hitting the note,
         * check to see if note can be hit with the current information
         * (currently ongoing combo, lift note, etc)
         * If that check went well, hit the note.
         * If not, the player overhit the note and should be penalized.
         */

        int inst = 0;
        std::shared_ptr<BaseChart> chart;
        std::shared_ptr<Stats> stats;
        std::unordered_map<std::string, RhythmTimer> Timers;
        std::vector<Sustain> ActiveSustains;
        double EngineTime = 0;
        size_t noteCount = 0;

        Player* player;
        std::pair<int, int> GetNotePoolSize(int lane) const;

        /// Not a live updating value
        double OverdriveValue = 0.0;
        bool OverdriveActive = false;
        TimeSeconds OverdriveActivateTime = 0.0;

        bool HasWhammyOD = false;
        TimeSeconds WhammyODTime = 0.0;
        /// Set to the sustain end time when whammying
        TimeSeconds WhammyMaxTime = 0.0;

        float whammy = 0.0;
        float lastWhammyTriggerPoint = 0.0;
        bool earlyWhammyPrevention = false;
        void UpdateWhammy(TimeSeconds time, float newWhammy);
        void StartWhammy(TimeSeconds time, TimeSeconds max);

        /// Should be called before anything else that affects scoring
        void UpdateOverdrive(TimeSeconds time);

        void GainOverdrive(double amount);

        void ActivateOverdrive(TimeSeconds time);
        void EndOverdrive(TimeSeconds time);

        /// Invalidates iterators, be careful!
        void AddSustain(Sustain sustain);
        /// Invalidates iterators, be careful!
        void ClearSustain(Sustain& sustain);

        void UpdateSustains();

        double GetOverdriveFillAtTime(TimeSeconds time);
        TimeSeconds GetOverdriveEndTime();

        double GetVisualScoreAtTime(TimeSeconds time);

        /// Actually adds the sustain score
        void CompleteSustain(TimeSeconds time, Sustain& sustain);

        /// Adds the sustain score, clears it, and adds the note to dropped sustains.
        /// Invalidates iterators, be careful!
        void DropSustain(TimeSeconds time, Sustain& sustain);

        /// Call *after* updating OverdriveActive
        void UpdateMultiplier(TimeSeconds time);
        void SegmentSustains(TimeSeconds time, double newMultiplier);
        double Multiplier = 1;

        bool deferMultiplierUpdates = false;
        struct MultUpdate {
            TimeSeconds time;
            double mult;
        };
        std::vector<MultUpdate> defferredMultiplierUpdates;


        // bool GetCurrentNote(int lane);
        // virtual bool CanNoteBeHit();
        TimePoint NextNoteTime();
        TimePoint LastNoteTime();
        virtual void Advance(double CurrentTime);
        void CheckMissedNotes(size_t Lane, TimeSeconds time);
        virtual void HitNote(size_t lane);
        void MissNote(size_t lane, TimeSeconds time);
        void Overhit(size_t lane);
        void UpdateStats(int instrument, int difficulty);
        virtual bool UsesNoteMasks() { return false; };
        virtual void OnOverdriveActivate(TimeSeconds time) {};
        int GhostCount;

        void SetSucceeding(bool succeed);

    private:
        virtual void RunHitStateCheck(ControllerEvent &event) = 0;
    };
}


#pragma once
//
// Created by maria on 01/06/2025.
//

#include "BaseStats.h"
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
    class BaseEngine : public EventSource {
    public:
        BaseEngine(auto _chart, auto _stats, Player* _player)
            : chart(_chart), stats(_stats), player(_player) {
            size_t noteCount = 0;
            for (auto& lane : chart->Lanes) noteCount += lane.size();
            stats->accuracies.reserve(noteCount);
        };

        ~BaseEngine() override = default;

        virtual void SetStatsInputState(ControllerEvent &event) {
        };
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
        std::shared_ptr<BaseStats> stats;
        std::unordered_map<std::string, RhythmTimer> Timers;
        double LastUpdateTime;
        bool allowTimestampedInputs = true;

        Player* player;
        float whammy = 0.0;
        bool practice = false;
        double pStartTime = 0.0;
        double pStopTime = 0.0;
        std::pair<int, int> GetNotePoolSize(int lane) const;

        // bool GetCurrentNote(int lane);
        // virtual bool CanNoteBeHit();
        virtual TimePoint NextNoteTime() { return TimePoint(-1,-1); };
        virtual TimePoint LastNoteTime() { return TimePoint(-1,-1); };
        bool IsWithinPracticeSection(double time) const;
        virtual void UpdateOnFrame(double CurrentTime) {};
        void BaseUpdateOnFrame(double CurrentTime);
        void CheckMissedNotes(size_t Lane, double SongTime);
        virtual void HitNote(size_t lane);
        void MissNote(size_t lane);
        void Overhit(size_t lane);
        void UpdateStats(int instrument, int difficulty);
        virtual bool UsesNoteMasks() { return false; };
        void UpdateCalibration(double playerInputOffset) const;
        int GhostCount;

    private:
        virtual bool PlayerIsPaused() = 0;
        virtual void TogglePause() = 0;

        virtual int RunHitStateCheck(ControllerEvent &event) = 0;
        virtual bool ActivateOverdrive(ControllerEvent &event) = 0;

        bool PauseGame(const ControllerEvent &event);
    };
}


//
// Created by maria on 01/06/2025.
//

#ifndef BASEENGINE_H
#define BASEENGINE_H
#include "BaseStats.h"
#include "RhythmTimer.h"
#include "timingvalues.h"
#include "RhythmEngine/NoteVector.h"

#include <memory>
#include <span>
#include <unordered_map>

namespace Encore::RhythmEngine {

    enum HitState {
        HitNote = 0,
        OverhitNote = 1,
        CheckNextInput = 2
    };

    class BaseEngine {
    public:
        BaseEngine(auto _chart, auto _stats) : chart(_chart), stats(_stats) {};
        virtual ~BaseEngine() {};
        virtual void SetStatsInputState(InputChannel channel, Action action) {};
        bool EarlyStrike(double noteStartTime);
        bool InHitwindow(double noteStartTime);
        bool PerfectHit(double noteStartTime);
        void ProcessInput(InputChannel channel, Action action);
        /*
         * Before hitting the note,
         * check to see if note can be hit with the current information
         * (currently ongoing combo, lift note, etc)
         * If that check went well, hit the note.
         * If not, the player overhit the note and should be penalized.
         */

        std::shared_ptr<BaseChart> chart;
        std::shared_ptr<BaseStats<5> > stats;
        std::unordered_map<std::string, RhythmTimer> Timers;

        virtual std::pair<int, int> GetNotePoolSize() { return {0,0};};

        // bool GetCurrentNote(int lane);
        // virtual bool CanNoteBeHit();
        virtual void UpdateOnFrame(double CurrentTime) {

        };
        void CheckMissedNotes(int Lane, double SongTime);
        void HitNote(int lane);
        void MissNote(int lane);
        void Overhit(int lane);
        void UpdateStats(int instrument, int difficulty);
        virtual bool UsesNoteMasks() { return false; };
    private:
        virtual bool PlayerIsPaused() = 0;
        virtual void TogglePause() = 0;

        virtual int RunHitStateCheck(InputChannel channel, Action action) = 0;
        virtual bool ActivateOverdrive(InputChannel channel, Action action) = 0;

        bool PauseGame(InputChannel channel, Action action);
    };
}

#endif // BASEENGINE_H

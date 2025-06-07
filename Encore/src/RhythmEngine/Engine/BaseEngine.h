//
// Created by maria on 01/06/2025.
//

#ifndef BASEENGINE_H
#define BASEENGINE_H
#include "BaseStats.h"
#include "RhythmEngine/NoteVector.h"

#include <memory>

namespace Encore::RhythmEngine {
    enum HitState {
        HitNote = 0,
        OverhitNote = 1,
        CheckNextInput = 2
    };

    class BaseEngine {
    public:
        BaseEngine() {};
        virtual ~BaseEngine() {};

        void ProcessInput(InputChannel channel, Action action);
        /*
         * Before hitting the note,
         * check to see if note can be hit with the current information
         * (currently ongoing combo, lift note, etc)
         * If that check went well, hit the note.
         * If not, the player overhit the note and should be penalized.
         */
        std::shared_ptr<BaseChart<EncNote, 5>> chart;
        std::shared_ptr<BaseStats<5>> stats;
    private:
        virtual bool PlayerIsPaused() = 0;
        virtual void TogglePause() = 0;
        virtual void HitNote() {};
        virtual void Overhit() {};
        virtual int RunHitStateCheck(Action action) = 0;
        virtual bool ActivateOverdrive(InputChannel channel, Action action) = 0;
        virtual void SetStatsInputState(InputChannel channel, Action action) {};

        bool PauseGame(InputChannel channel, Action action);
    };
}

#endif // BASEENGINE_H

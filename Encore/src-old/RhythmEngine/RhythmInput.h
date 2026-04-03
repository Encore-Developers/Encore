//
// Created by maria on 09/05/2025.
//

#ifndef RHYTHMINPUT_H
#define RHYTHMINPUT_H
#include "REenums.h"
#include "gameplay/enctime.h"

namespace Encore::RhythmEngine {

    // run tick check at beginning of frame
    // should be held by playermanager as a pointer perchance?
    // RhythmEngine/Input is per player

    template <typename T>
    class RhythmInput {
        bool Paused = false;
        T *rhythmEngine;
        explicit RhythmInput(const T *enginePointer) { rhythmEngine = enginePointer; };
        bool PauseGame(InputChannel channel) {
            if (channel == InputChannel::PAUSE) {
                Paused = !Paused;
                return true;
            }
            return false;
        }

        void ProcessInput(InputChannel channel, Action action) {
            double time = TheSongTime.GetElapsedTime();
            if (action == Action::REPEAT)
                return;
            if (action == Action::PRESS) {
                if (PauseGame(channel)) {
                    return;
                }
                if (rhythmEngine->ActivateOverdrive(channel)) {
                    return;
                }
            }
            rhythmEngine->UpdateEngineOnInput(channel, action, time);
        }
    };
}

#endif // RHYTHMINPUT_H

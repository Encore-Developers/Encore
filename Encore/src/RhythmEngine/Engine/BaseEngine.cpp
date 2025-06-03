//
// Created by maria on 01/06/2025.
//

#include "BaseEngine.h"


void Encore::RhythmEngine::BaseEngine::ProcessInput(InputChannel channel, Action action) {
    if (PauseGame(channel, action)) {
        return;
    }
    if (PlayerIsPaused())
        return;
    if (ActivateOverdrive(channel, action)) {
        return;
    }
    SetStatsInputState(channel, action);
    // look its really stupid but like. cmon
    switch (RunHitStateCheck(action)) {
    case HitState::HitNote: {
        HitNote();
        break;
    }
    case HitState::OverhitNote: {
        Overhit();
        break;
    }
    case HitState::CheckNextInput: {
        break;
    }
    }
}


bool Encore::RhythmEngine::BaseEngine::PauseGame(InputChannel channel, Action action) {
    if (channel == InputChannel::PAUSE && action == Action::PRESS) {
        TogglePause();
        return true;
    }
    return false;
}

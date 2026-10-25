//
// Created by maria on 01/06/2025.
//

#include "Stats.h"

#include "RhythmEngine/REenums.h"
uint8_t Encore::RhythmEngine::Stats::HeldFretsArrayToMask() const {
    uint8_t mask = 0;
    for (int pressedButtons = 0; pressedButtons < HeldFrets.size(); pressedButtons++) {
        if (HeldFrets[pressedButtons]) {
            mask += PlasticFrets[pressedButtons];
        }
    }
    return mask;
}

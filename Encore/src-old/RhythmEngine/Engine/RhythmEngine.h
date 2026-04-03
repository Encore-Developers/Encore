//
// Created by maria on 21/05/2025.
//

#ifndef RHYTHMENGINE_H
#define RHYTHMENGINE_H

#include <array>
#include <bit>
#include <memory>

namespace Encore::RhythmEngine {

    /**
     * @brief The base class for Encore's RhythmEngine.
     * It handles request based inputs, and frame-based updates.
     * The current player's chart is held here.
     *
     * Note: should NOT exist outside of gameplay/endgame.
     * It should always be a pointer that is created for gameplay,
     * and deleted outside of gameplay.
     *
     * also, please learn how c++ pointers work
     *
     * note: current tick time will be handled externally via some GlobalRhythmManager
     * potentially
     */

    void lol();
}

#endif // RHYTHMENGINE_H

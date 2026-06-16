//
// Created by maria on 03/05/2026.
//

#include "OverdriveTicks.h"

#include "gameplay/enctime.h"
#include "tracy/Tracy.hpp"



void Encore::RhythmEngine::OverdriveTicks::UpdateOverdriveTick() {
    // this is the overdrive code right here
    double CurrentTime = TheSongTime.GetElapsedTime();
    if (ticks.size() == 0)
        return;
    while (CurrentODTickItr < ticks.size() - 1) {
        // if the next tick's time is greater than the current time, stop
        // otherwise, just increase lmfao


        const auto &NextTick = ticks.at(CurrentODTickItr + 1);
        // Encore::EncoreLog(LOG_DEBUG, TextFormat("Next tick time: %4.4f",
        // NextTick.time)); Encore::EncoreLog(LOG_DEBUG, TextFormat("Current time: %4.4f",
        // CurrentTime));
        if (NextTick.time > CurrentTime) {
            // Encore::EncoreLog(LOG_DEBUG, TextFormat("Current Overdrive Tick: %01i",
            // (CurrentODTickItr)));

            break;
        };

        ++CurrentODTickItr;
        // Encore::EncoreLog(LOG_DEBUG, TextFormat("Skipping to next OD tick: %01i",
        // (CurrentODTickItr)));
    }
    LastODTick = CurrentODTick;
    // get the time since the last beat
    double timeDelta = CurrentTime - ticks.at(CurrentODTickItr).time;
    // double tickDelta = CurrentODTick - OverdriveTicks.at(CurrentODTickItr).tick;
    // what if i legit delt with ticks. it would be funny and i could totally do it
    // get the total time of this tick
    // double tickBetweenTicks = OverdriveTicks.at(CurrentODTickItr + 1).tick
    //     - OverdriveTicks.at(CurrentODTickItr).tick;

    // double tickDeltaToPct = tickDelta / tickBetweenTicks;
    if (CurrentODTickItr == ticks.size() - 1) {
        double timeBetweenTicks =
            (TheSongTime.GetElapsedTime() + 10) - ticks.at(CurrentODTickItr).time;

        double deltaMappedToPercentage = timeDelta / timeBetweenTicks;
        CurrentODTick = CurrentODTickItr + deltaMappedToPercentage;
    } else {
        double timeBetweenTicks = ticks.at(CurrentODTickItr + 1).time
            - ticks.at(CurrentODTickItr).time;

        double deltaMappedToPercentage = timeDelta / timeBetweenTicks;
        CurrentODTick = CurrentODTickItr + deltaMappedToPercentage;
    }
}
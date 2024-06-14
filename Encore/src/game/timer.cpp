//
// Created by marie on 13/06/2024.
//

#include "game/timer.h"
#include "raylib.h"

void Timers::StartTimer(Timer *timer, double lifetime)
{
    timer->startTime = GetTime();
    timer->duration = lifetime;
}

bool Timers::FinishedTimer(Timer timer)
{
    return GetTime() - timer.startTime >= timer.duration;
}

double Timers::TimerElapsed(Timer timer)
{
    return GetTime() - timer.startTime;
}
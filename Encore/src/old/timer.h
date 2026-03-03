//
// Created by marie on 13/06/2024.
//

#ifndef ENCORE_TIMER_H
#define ENCORE_TIMER_H

struct Timer {
    double startTime;
    double duration;
};

class Timers {
    void StartTimer(Timer *timer, double duration);
    bool FinishedTimer(Timer timer);
    double TimerElapsed(Timer timer);
};

#endif // ENCORE_TIMER_H

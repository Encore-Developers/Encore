//
// Created by maria on 20/06/2025.
//

#ifndef RHYTHMTIMER_H
#define RHYTHMTIMER_H

namespace Encore::RhythmEngine {
    struct RhythmTimer {
        RhythmTimer() : Duration(0.05), Time(-1), Active(false) {};
        explicit RhythmTimer(double dur) {
            Duration = dur;
            Time = -1;
            Active = false;
        };
        [[nodiscard]] bool CanBeUsedUp(double time) const {
            if (time < Time + Duration && Active)
                return true;
            return false;
        }
        void ActivateTimer(double time) {
            Time = time;
            Active = true;
        }
        void ResetTimer() {
            Time = -1;
            Active = false;
        }
        double Duration;
        double Time;
        bool Active;
    };
}
#endif // RHYTHMTIMER_H

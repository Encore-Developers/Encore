
#ifndef ENCORE_COLORPROFILE_H
#define ENCORE_COLORPROFILE_H
#include "raylib.h"
namespace Encore {

    // SLOT_ prefix to prevent name conflicts
    enum ColorSlot : int {
        SLOT_GREEN = 0,
        SLOT_RED = 1,
        SLOT_YELLOW = 2,
        SLOT_BLUE = 3,
        SLOT_ORANGE = 4,
        SLOT_OPEN = 5,
        SLOT_KICK = 6,
        SLOT_OVERDRIVE = 7,
        SLOT_HIHAT = 8,
        SLOT_RIDE = 9,
        SLOT_CRASH = 10,
        SLOT_MAX = 11
    };

    class ColorProfile {
    public:
        Color colors[SLOT_MAX] = { // Default colors
            GREEN,
            RED,
            YELLOW,
            BLUE,
            ORANGE,
            PURPLE, // OPEN
            ORANGE, // KICK
            WHITE,  // OVERDRIVE
            YELLOW, // HIHAT
            BLUE,   // RIDE
            GREEN   // CRASH
        };
    };

    static ColorProfile defaultProfile;
}


#endif // ENCORE_COLORPROFILE_H

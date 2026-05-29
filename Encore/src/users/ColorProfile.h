
#ifndef ENCORE_COLORPROFILE_H
#define ENCORE_COLORPROFILE_H
#include <string>
#include <nlohmann/json.hpp>
#include "raylib.h"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Color, r, g, b, a);

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
        std::string Name = "Default Profile";
        std::array<Color, SLOT_MAX> colors = {
            // Default colors
            GREEN,
            RED,
            YELLOW,
            BLUE,
            ORANGE,
            PURPLE, // OPEN
            ORANGE, // KICK
            LIGHTGRAY, // OVERDRIVE
            YELLOW, // HIHAT
            BLUE, // RIDE
            GREEN // CRASH
        };
        bool builtin = false;
        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(
            ColorProfile,
            Name,
            colors
        );
    };



    static ColorProfile defaultPlastic;
    static ColorProfile transgender {
        "transgender",
        { SKYBLUE,
            PINK,
            WHITE,
            PINK,
            SKYBLUE,
            PURPLE, // OPEN
            ORANGE, // KICK
            PURPLE , // OVERDRIVE
            YELLOW, // HIHAT
            BLUE, // RIDE
            GREEN // CRASH
        }
    };
    static ColorProfile defaultPad {
        "Default Pad Profile",
        { SKYBLUE,
            SKYBLUE,
            PURPLE,
            PURPLE,
            PURPLE,
            PURPLE, // OPEN
            ORANGE, // KICK
            LIGHTGRAY , // OVERDRIVE
            YELLOW, // HIHAT
            BLUE, // RIDE
            GREEN // CRASH
        }
    };
}


#endif // ENCORE_COLORPROFILE_H

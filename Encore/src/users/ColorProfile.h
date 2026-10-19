
#ifndef ENCORE_COLORPROFILE_H
#define ENCORE_COLORPROFILE_H
#include <string>
#include <nlohmann/json.hpp>
#include "raylib.h"


inline void to_json(nlohmann::json &nlohmann_json_j, const Color &nlohmann_json_t) {
    nlohmann_json_j["r"] = nlohmann_json_t.r;
    nlohmann_json_j["g"] = nlohmann_json_t.g;
    nlohmann_json_j["b"] = nlohmann_json_t.b;
    if (nlohmann_json_t.a != 255) {
        nlohmann_json_j["a"] = nlohmann_json_t.a;
    }
}

inline void from_json(const nlohmann::json &nlohmann_json_j, Color &nlohmann_json_t) {
    nlohmann_json_j.at("r").get_to(nlohmann_json_t.r);
    nlohmann_json_j.at("g").get_to(nlohmann_json_t.g);
    nlohmann_json_j.at("b").get_to(nlohmann_json_t.b);
    if (nlohmann_json_j.contains("a")) {
        nlohmann_json_j.at("a").get_to(nlohmann_json_t.a);
    } else {
        nlohmann_json_t.a = 255;
    }
}
namespace Encore {
    // SLOT_ prefix to prevent name conflicts
    enum ColorSlot : int {
        SLOT_GREEN = 0,
        SLOT_RED,
        SLOT_YELLOW,
        SLOT_BLUE,
        SLOT_ORANGE,
        SLOT_OPEN,
        SLOT_KICK,
        SLOT_OVERDRIVE,
        SLOT_HIHAT,
        SLOT_RIDE,
        SLOT_CRASH,
        SLOT_FRAME,
        SLOT_FRAME_OVERDRIVE,
        SLOT_MAX
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
            GREEN, // CRASH
            WHITE,
            GOLD
        };
        bool builtin = false;
        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(
            ColorProfile,
            Name
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
            WHITE , // OVERDRIVE
            YELLOW, // HIHAT
            BLUE, // RIDE
            GREEN, // CRASH
            WHITE, // FRAME
            PURPLE // FRAME OVERDRIVE
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
            GOLD , // OVERDRIVE
            YELLOW, // HIHAT
            BLUE, // RIDE
            GREEN, // CRASH
            WHITE, // FRAME
            GOLD // FRAME OVERDRIVE
        }
    };
}


#endif // ENCORE_COLORPROFILE_H

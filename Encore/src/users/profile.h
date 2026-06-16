#pragma once
#include "raylib.h"

#include <cassert>
#include <memory>
#include <string>
#include <utility>

// TODO: move modifiers to its own file
//#define MODIFIER(id, incompatible)

#define MODIFIER_MACROS \
    MODIFIER(leftyFlip, 0) \
    MODIFIER(brutalMode, 0) \
    MODIFIER(autoplay, 0) \
    MODIFIER(tapsOnly, strumsOnly | hopoToTaps) \
    MODIFIER(strumsOnly, tapsOnly | hopoToTaps) \
    MODIFIER(hopoToTaps, tapsOnly | strumsOnly) \
    MODIFIER(doubleBass, 0) \
    MODIFIER(yellowCymbal, 0) \
    MODIFIER(blueCymbal, 0) \
    MODIFIER(greenCymbal, 0) \

class Modifiers {
public:
    enum Index : u_int32_t {
        none = 0,
    #define MODIFIER(id, _) id,
        MODIFIER_MACROS
    #undef MODIFIER
        max
    };

    static constexpr std::string GetName(Index value) {
        switch (value) {
        #define MODIFIER(id, _) case id: return #id;
            MODIFIER_MACROS
        #undef MODIFIER
        default: break;
        }
        return "invalid";
    }

    static constexpr u_int32_t GetIncompatible(Index value) {
        switch (value) {
        #define MODIFIER(id, incompatible) case id: return incompatible;
            MODIFIER_MACROS
        #undef MODIFIER
        default: break;
        }
        return none;
    }

    void ToggleModifier(Index toToggle) {
        if (!(value | toToggle)) {
            value &= ~GetIncompatible(toToggle);
        }
        value ^= toToggle;
    }

    bool IsModifierActive(Index toCheck) const {
        return value & toCheck;
    }

    Modifiers(Index value) : value(value) {}
    Modifiers() : value(none) {}

    u_int32_t value = 0;
};

class Profile {
public:
    std::string name = "Guest";
    float noteSpeed = 1;
    float trackLength = 1;
    float inputCalibration = 0;
    Color accentColor = {255, 0, 255, 255};

    Modifiers modifiers = Modifiers::none;

    Profile() = default;
    Profile(std::string name) : name(std::move(name)) {}
};

extern std::shared_ptr<Profile> guestProfile;

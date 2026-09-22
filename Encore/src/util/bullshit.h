#pragma once
#include "Input.h"
#include "SDL3/SDL_gamepad.h"
#include <array>
// we need something between the input channels and the actual SDL inputs for CUSTOM bindings
// to work.
// this feels like a "binding" struct should just be a vector/array of ints instead of This
// no the struct works because we dont use the values *as* ints and we cant just have

namespace Encore {

    struct Bindings {
        std::array<InputChannel, SDL_GAMEPAD_BUTTON_COUNT> menu {
            InputChannel::UI_CONFIRM,
            InputChannel::UI_BACK,
            InputChannel::UI_MENU,
            InputChannel::UI_SECONDARY,
            InputChannel::UI_SELECT,
            InputChannel::INVALID,
            InputChannel::UI_START,
            InputChannel::INVALID,
            InputChannel::INVALID,
        }
        struct Face {
            int8_t confirm = SDL_GAMEPAD_BUTTON_SOUTH;
            int8_t back = SDL_GAMEPAD_BUTTON_EAST;
            int8_t secondary = SDL_GAMEPAD_BUTTON_NORTH;
            int8_t menu = SDL_GAMEPAD_BUTTON_WEST;
            int8_t modifier = SDL_GAMEPAD_BUTTON_LEFT_SHOULDER;
        } face;
        struct System {
            int8_t start = SDL_GAMEPAD_BUTTON_START;
            int8_t select = SDL_GAMEPAD_BUTTON_START;
            int8_t guide = SDL_GAMEPAD_BUTTON_GUIDE;
        } sys;
        struct DirectionalPad {
            int8_t left = SDL_GAMEPAD_BUTTON_DPAD_LEFT;
            int8_t right = SDL_GAMEPAD_BUTTON_DPAD_RIGHT;
            int8_t up = SDL_GAMEPAD_BUTTON_DPAD_UP;
            int8_t down = SDL_GAMEPAD_BUTTON_DPAD_DOWN;
        } dpad;
    };
}
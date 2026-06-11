#pragma once
#include "SDL3/SDL_joystick.h"

namespace Encore {
    enum class InputChannel : int8_t {
        LANE_1 = 0,
        LANE_2 = 1,
        LANE_3 = 2,
        LANE_4 = 3,
        LANE_5 = 4,
        STRUM_UP = 7,
        STRUM_DOWN = 8,
        PAUSE = 9,
        OVERDRIVE = 10,
        WHAMMY = 11,
        INPUT_LEFT = 12,
        INPUT_RIGHT = 13,
        INVALID = -1
    };
    inline InputChannel IntIC(int lane) {
        switch (lane) {
        case 0:
            return InputChannel::LANE_1;
        case 1:
            return InputChannel::LANE_2;
        case 2:
            return InputChannel::LANE_3;
        case 3:
            return InputChannel::LANE_4;
        case 4:
            return InputChannel::LANE_5;
        default:
            return InputChannel::INVALID;
        }
    }
    inline int ICInt(InputChannel channel) {
        switch (channel) {
        case InputChannel::LANE_1:
            return 0;
        case InputChannel::LANE_2:
            return 1;
        case InputChannel::LANE_3:
            return 2;
        case InputChannel::LANE_4:
            return 3;
        case InputChannel::LANE_5:
            return 4;
        default:
            return 0;
        }
    }

    // Temporary stopgap to let people do things on this old version
    enum ControllerBindingType {
        GUITAR,
        GUITAR_GHPS3,
        PAD,
        DRUMS
    };

    enum class ControllerType : int {
        NONE = 0,
        KEYBOARD,
        SDL,
        MIDI,
        REMOTE
    };

    struct ControllerIdentity {
        ControllerType type = ControllerType::SDL;
        union {
            unsigned int raw;
            SDL_JoystickID sdlJoystickId;
            // MIDI isn't implemented yet, putting this in the union for future-proofing
            unsigned int midiDeviceId;
        };

        ControllerIdentity(SDL_JoystickID id) : type(ControllerType::SDL), sdlJoystickId(id) {}
        ControllerIdentity() : raw(0) {}
    };

    enum class Action : int8_t {
        INVALID = -1,
        PRESS = 1,
        RELEASE = 0,
        REPEAT = 2 // not needed but whatever
    };

    class ControllerEvent {
    public:
        InputChannel channel : 8 = InputChannel::INVALID;
        Action action : 8 = Action::INVALID;
        unsigned int axis : 8 = 0;
        ControllerIdentity controller;
        double timestamp = 0;

        bool IsAccept() {
            return action == Action::PRESS && channel == InputChannel::LANE_1;
        }
    };
}
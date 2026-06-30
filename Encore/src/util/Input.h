#pragma once
#include "SDL3/SDL_joystick.h"

namespace Encore {
    enum class InputChannel : int8_t {
        LANE_1 = 0,
        LANE_2,
        LANE_3,
        LANE_4,
        LANE_5,
        LANE_6,
        STRUM_UP,
        STRUM_DOWN,
        PAUSE,
        OVERDRIVE,
        WHAMMY,
        INPUT_LEFT,
        INPUT_RIGHT,
        CHANNEL_MAX,
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
        case 5:
            return InputChannel::LANE_6;
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
        case InputChannel::LANE_6:
            return 5;
        case InputChannel::STRUM_UP:
            return 7;
        case InputChannel::STRUM_DOWN:
            return 8;
        case InputChannel::PAUSE:
            return 9;
        case InputChannel::OVERDRIVE:
            return 10;
        case InputChannel::WHAMMY:
            return 11;
        case InputChannel::INPUT_LEFT:
            return 12;
        case InputChannel::INPUT_RIGHT:
            return 13;
        default:
            return 0;
        }

    }
    enum class Action : int8_t {
        INVALID = -1,
        PRESS = 1,
        RELEASE = 0,
        REPEAT = 2 // not needed but whatever
    };

    class ControllerEvent {
    public:
        InputChannel channel = InputChannel::INVALID;
        Action action = Action::INVALID;
        unsigned char axis;
        SDL_JoystickID slot;
        double timestamp;

        bool IsAccept() {
            return action == Action::PRESS && channel == InputChannel::LANE_1;
        }
    };
}
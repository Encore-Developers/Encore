#pragma once
#include "SDL3/SDL_joystick.h"
#include "users/controller.h"

namespace Encore {
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
        unsigned char axis = 0;
        ControllerIdentity controller;
        double timestamp = 0;

        bool IsAccept() {
            return action == Action::PRESS && channel == InputChannel::LANE_1;
        }
    };
}
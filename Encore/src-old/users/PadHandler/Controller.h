//
// Created by maria on 27/08/2025.
//

#ifndef ENCORE_CONTROLLER_H
#define ENCORE_CONTROLLER_H
#include "GLFW/glfw3.h"

#include <array>

namespace Encore {
    class Controller {
    public:
        std::array<int, 5> FacePadLayout {
            GLFW_GAMEPAD_BUTTON_DPAD_LEFT,
            GLFW_GAMEPAD_BUTTON_DPAD_UP,
            GLFW_GAMEPAD_BUTTON_X,
            GLFW_GAMEPAD_BUTTON_Y,
            GLFW_GAMEPAD_BUTTON_B
        };
        std::array<int, 15> previousFaceValues {
            0, // GLFW_GAMEPAD_BUTTON_A
            0, // GLFW_GAMEPAD_BUTTON_B
            0, // GLFW_GAMEPAD_BUTTON_X
            0, // GLFW_GAMEPAD_BUTTON_Y
            0, // GLFW_GAMEPAD_BUTTON_LEFT_BUMPER
            0, // GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER
            0, // GLFW_GAMEPAD_BUTTON_BACK
            0, // GLFW_GAMEPAD_BUTTON_START
            0, // GLFW_GAMEPAD_BUTTON_GUIDE
            0, // GLFW_GAMEPAD_BUTTON_LEFT_THUMB
            0, // GLFW_GAMEPAD_BUTTON_RIGHT_THUMB
            0, // GLFW_GAMEPAD_BUTTON_DPAD_UP
            0, // GLFW_GAMEPAD_BUTTON_DPAD_RIGHT
            0, // GLFW_GAMEPAD_BUTTON_DPAD_DOWN
            0 // GLFW_GAMEPAD_BUTTON_DPAD_LEFT
        };
        static bool IsButtonPress(const GLFWgamepadstate &state, const int button) {
            if (state.buttons[button] == GLFW_PRESS)
                return true;
            return false;
        };
        static bool IsButtonRelease(const GLFWgamepadstate &state, const int button) {
            if (state.buttons[button] == GLFW_RELEASE)
                return true;
            return false;
        };
        [[nodiscard]] int GetButtonState(const GLFWgamepadstate &state, const int button) {
            if (state.buttons[button] != previousFaceValues[button]) {
                previousFaceValues.at(button) = state.buttons[button];
                if (IsButtonPress(state, button))
                    return GLFW_PRESS;
                if (IsButtonRelease(state, button))
                    return GLFW_RELEASE;
            }
            return 3;
        }
    };
};

#endif // ENCORE_CONTROLLER_H

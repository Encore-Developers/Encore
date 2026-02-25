//
// Created by maria on 17/12/2024.
//

#ifndef INPUTCALLBACKS_H
#define INPUTCALLBACKS_H
#include "GLFW/glfw3.h"
#include "RhythmEngine/REenums.h"

// what to check when a key changes states (what was the change? was it pressed? or
// released? what time? what window? were any modifiers pressed?)
void keyCallback(GLFWwindow *wind, int key, int scancode, int action, int mods);
void gamepadStateCallback(Encore::RhythmEngine::ControllerEvent event);
void PollSDL3ControllerInputs();



/*
static void gamepadStateCallbackSetControls(int jid, GLFWgamepadstate state) {
    for (int i = 0; i < 6; i++) {
        axesValues2[i] = state.axes[i];
    }
    if (changingKey || changingOverdrive || changingPause) {
        for (int i = 0; i < 15; i++) {
            if (state.buttons[i] == 1) {
                if (buttonValues[i] == 0) {
                    controllerID = jid;
                    pressedGamepadInput = i;
                    return;
                } else {
                    buttonValues[i] = state.buttons[i];
                }
            }
        }
        for (int i = 0; i < 6; i++) {
            if (state.axes[i] == 1.0f || (i <= 3 && state.axes[i] == -1.0f)) {
                axesValues[i] = 0.0f;
                if (state.axes[i] == 1.0f) axisDirection = 1;
                else axisDirection = -1;
                controllerID = jid;
                pressedGamepadInput = -(1 + i);
                return;
            } else {
                axesValues[i] = 0.0f;
            }
        }
    } else {
        for (int i = 0; i < 15; i++) {
            buttonValues[i] = state.buttons[i];
        }
        for (int i = 0; i < 6; i++) {
            axesValues[i] = state.axes[i];
        }
        pressedGamepadInput = -999;
    }
}
*/

#endif //INPUTCALLBACKS_H

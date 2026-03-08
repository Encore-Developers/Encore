//
// Created by maria on 17/12/2024.
//
#include "inputCallbacks.h"

#include "enctime.h"
#include "menus/MenuManager.h"
#include "util/enclog.h"
#include "raylib.h"
#include "settings/settings.h"
#include "debug/EncoreDebug.h"
#include "rlImGui.h"
#include "SDL3/SDL.h"
#include <mutex>

#include <cstring>

#include "RhythmEngine/REenums.h"

using namespace std::chrono_literals;

void keyCallback(GLFWwindow *wind, int key, int scancode, int action, int mods) {
    // Encore::EncoreLog(LOG_DEBUG, TextFormat("Keyboard key %01i inputted on menu %s",
    // key, ToString(TheMenuManager.currentScreen)) );

    if (action == GLFW_PRESS) {
        if (key == KEY_F11 || (key == KEY_ENTER && mods & GLFW_MOD_ALT)) {
            TheGameSettings.Fullscreen = !TheGameSettings.Fullscreen;
            TheGameSettings.UpdateFullscreen();
            return;
        }

        if (key == KEY_F3) {
            EncoreDebug::showDebug = !EncoreDebug::showDebug;
        }
    }

    rlImGuiPushKeyEvent(key, scancode, action, mods);
    if (ImGui::GetIO().WantCaptureKeyboard) {
        return;
    }

    TheMenuManager.ActiveMenu->KeyboardInputCallback(key, scancode, action, mods);
}


void gamepadStateCallback(Encore::RhythmEngine::ControllerEvent event) {
    //Encore::EncoreLog(
    //    LOG_DEBUG,
    //    TextFormat(
    //        "Gamepad %01i inputted on menu %s",
    //        joypadID,
    //        ToString(TheMenuManager.currentScreen)
    //    )
    //);
    switch (TheMenuManager.currentScreen) {
    // NOTE: when adding a new Menu
    // derivative, you
    // must put its enum value in Screens, and its
    // assignment in this switch/case. You will also
    // add its case to the `ActiveMenu->Draw();`
    // cases.
    default: {
        // TheMenuManager.ActiveMenu->ControllerInputCallback(joypadID, state);
        break;
    }
    }
}

Encore::RhythmEngine::ControllerEvent TranslateEvent(SDL_Event *event) {
    Encore::RhythmEngine::ControllerEvent outevent = {};


    if (event->type == SDL_EVENT_GAMEPAD_BUTTON_UP || event->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
        switch (event->gbutton.button) {
        default:
            outevent.channel = Encore::RhythmEngine::InputChannel::INVALID;
            break;
        case(SDL_GAMEPAD_BUTTON_SOUTH):
            outevent.channel = Encore::RhythmEngine::InputChannel::LANE_1;
            break;
        case(SDL_GAMEPAD_BUTTON_EAST):
            outevent.channel = Encore::RhythmEngine::InputChannel::LANE_2;
            break;
        case(SDL_GAMEPAD_BUTTON_NORTH):
            outevent.channel = Encore::RhythmEngine::InputChannel::LANE_3;
            break;
        case(SDL_GAMEPAD_BUTTON_WEST):
            outevent.channel = Encore::RhythmEngine::InputChannel::LANE_4;
            break;
        case(SDL_GAMEPAD_BUTTON_LEFT_SHOULDER):
            outevent.channel = Encore::RhythmEngine::InputChannel::LANE_5;
            break;
        case(SDL_GAMEPAD_BUTTON_DPAD_UP):
            outevent.channel = Encore::RhythmEngine::InputChannel::STRUM_UP;
            break;
        case(SDL_GAMEPAD_BUTTON_DPAD_DOWN):
            outevent.channel = Encore::RhythmEngine::InputChannel::STRUM_DOWN;
            break;
        case(SDL_GAMEPAD_BUTTON_START):
            outevent.channel = Encore::RhythmEngine::InputChannel::PAUSE;
            break;

        case(SDL_GAMEPAD_BUTTON_BACK):
            outevent.channel = Encore::RhythmEngine::InputChannel::OVERDRIVE;
            break;
        }
        if (event->type == SDL_EVENT_GAMEPAD_BUTTON_UP)
            outevent.action = Encore::RhythmEngine::Action::RELEASE;
        else if (event->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN)
            outevent.action = Encore::RhythmEngine::Action::PRESS;

        outevent.slot = SDL_GetGamepadPlayerIndexForID(event->gdevice.which);
    }

    return outevent;
}

void PollQueuedInputs(ControllerPoller& poller) {
    while (poller.readIndex < poller.writeIndex) {
        auto event = poller.getEvent(poller.readIndex);
        if (TheMenuManager.ActiveMenu) {
            TheMenuManager.ActiveMenu->ControllerInputCallback(event);
        }
        poller.readIndex++;
    }
}

void _PollQueuedInputs() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Encore::EncoreLog(LOG_INFO, TextFormat("SDL event %i", event.type));
        switch (event.type) {
        case SDL_EVENT_GAMEPAD_ADDED:
            SDL_OpenGamepad(event.gdevice.which);
            Encore::EncoreLog(LOG_INFO,
                              TextFormat("SDL gamepad name %s",
                                         SDL_GetGamepadNameForID(event.gdevice.which)));
            break;
        case SDL_EVENT_GAMEPAD_REMOVED:
            // TODO: device removal/saving gamepad pointer to player
            Encore::EncoreLog(LOG_INFO,
                              TextFormat("SDL gamepad name %s",
                                         SDL_GetGamepadNameForID(event.gdevice.which)));
            break;
            // case SDL_EVENT_QUIT:
            //    return 0;
        }
        if (TheMenuManager.ActiveMenu && (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN || event.type == SDL_EVENT_GAMEPAD_BUTTON_UP)) {
            TheMenuManager.ActiveMenu->ControllerInputCallback(TranslateEvent(&event));
        }
    }
}

int ControllerPoller::controllerPollRate = 1000;
ControllerPoller* ControllerPoller::instance;

void ControllerPoller::Run() {
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    if (!SDL_Init(SDL_INIT_GAMEPAD | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_EVENTS)) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return;
    }
    Encore::EncoreLog(LOG_INFO, TextFormat("SDL Initialzed: revision %s", SDL_GetRevision()));

    while (active) {
        auto start = std::chrono::high_resolution_clock::now();
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_GAMEPAD_ADDED:
                SDL_OpenGamepad(event.gdevice.which);
                Encore::EncoreLog(LOG_INFO,
                                  TextFormat("SDL gamepad name %s",
                                             SDL_GetGamepadNameForID(event.gdevice.which)));
                break;
            case SDL_EVENT_GAMEPAD_REMOVED:
                // TODO: device removal/saving gamepad pointer to player
                Encore::EncoreLog(LOG_INFO,
                                  TextFormat("SDL gamepad name %s",
                                             SDL_GetGamepadNameForID(event.gdevice.which)));
                break;
            }
            if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN || event.type == SDL_EVENT_GAMEPAD_BUTTON_UP) {
                if (TheMenuManager.currentScreen == GAMEPLAY) {
                    auto time = TheSongTime.GetElapsedTime();
                    auto encEvent = TranslateEvent(&event);
                    encEvent.timestamp = time;
                    getEvent(writeIndex) = encEvent;
                    writeIndex++;
                }
            }
        }

        requestMutex.lock();
        for (auto &func : funcRequests) {
            func();
        }
        funcRequests.clear();
        requestMutex.unlock();

        auto end = std::chrono::high_resolution_clock::now();
        auto span = std::chrono::milliseconds(1000/controllerPollRate) - (end - start);
        std::this_thread::sleep_for(span);
    }
}
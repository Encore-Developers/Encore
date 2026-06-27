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
#include "../menus/overshell/OvershellMenu.h"
#include "song/song.h"
#include "song/songlist.h"
#include "tracy/Tracy.hpp"
#include "tracy/TracyC.h"
#include "users/player.h"
#include "users/playerManager.h"

using namespace std::chrono_literals;

void keyCallback(SDL_KeyboardEvent* event) {
    // Encore::EncoreLog(LOG_DEBUG, TextFormat("Keyboard key %01i inputted on menu %s",
    // key, ToString(TheMenuManager.currentScreen)) );

    if (event->down) {
        if (event->key == SDLK_F11 || (event->key == SDLK_RETURN && event->mod & SDL_KMOD_ALT)) {
            TheGameSettings.Fullscreen = !TheGameSettings.Fullscreen;
            TheGameSettings.UpdateFullscreen();
            return;
        }

        if (event->key == SDLK_F3) {
            EncoreDebug::showDebug = !EncoreDebug::showDebug;
        }
    }

    rlImGuiPushKeyEvent(*event);
    if (ImGui::GetIO().WantCaptureKeyboard) {
        return;
    }

    if (OvershellMenu* menu = dynamic_cast<OvershellMenu *>(TheMenuManager.ActiveMenu.get())) {
        if (OvershellKeyboardInputCallback(menu, event)) {
            return;
        }
    }
    TheMenuManager.ActiveMenu->KeyboardInputCallback(event);
}


void gamepadStateCallback(Encore::ControllerEvent event) {
    // this is a noop for now TODO remove
}

double syncAudioTime;
uint64_t syncSDLTicks;
double lastTranslatedTime = 0;

void SyncSDLWithAudio() {
    if (TheAudioManager.loadedStreams.empty()) {
        syncAudioTime = 0;
        syncSDLTicks = 0;
    }
    syncSDLTicks = SDL_GetTicksNS();
    syncAudioTime = TheAudioManager.GetMusicTimePlayed();
}

double SDLTimeToAudioTime(uint64_t ticks) {
    int64_t tickDelta = ticks - syncSDLTicks;
    double secondsDelta = (float)tickDelta * 0.000000001;
    lastTranslatedTime = syncAudioTime + secondsDelta;
    return lastTranslatedTime;
}

std::unordered_map<SDL_JoystickID, std::pair<bool, bool>> ControllerTriggerState;

Encore::ControllerEvent TranslateSDLEvent(SDL_Event *event) {
    Encore::ControllerEvent outevent = {};


    Player* player = ThePlayerManager.GetPlayerForJoystick(event->gbutton.which);
    ControllerBindingType bindingType = GUITAR;
    if (player) {
        bindingType = player->bindingType;
    }
    if (TheMenuManager.ActiveMenu && TheMenuManager.ActiveMenu->UIInput) {
        bindingType = GUITAR;
    }


    if (event->type == SDL_EVENT_GAMEPAD_BUTTON_UP || event->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
        outevent.channel = Encore::InputChannel::INVALID;
        outevent.timestamp = SDLTimeToAudioTime(event->gbutton.timestamp);

        // Generic inputs
        switch (event->gbutton.button) {
        case(SDL_GAMEPAD_BUTTON_DPAD_UP):
            outevent.channel = Encore::InputChannel::STRUM_UP;
            break;
        case(SDL_GAMEPAD_BUTTON_DPAD_DOWN):
            outevent.channel = Encore::InputChannel::STRUM_DOWN;
            break;
        case(SDL_GAMEPAD_BUTTON_DPAD_RIGHT):
            outevent.channel = Encore::InputChannel::INPUT_RIGHT;
            break;
        case(SDL_GAMEPAD_BUTTON_DPAD_LEFT):
            outevent.channel = Encore::InputChannel::INPUT_LEFT;
            break;
        case(SDL_GAMEPAD_BUTTON_START):
            outevent.channel = Encore::InputChannel::PAUSE;
            break;
        case(SDL_GAMEPAD_BUTTON_BACK):
            outevent.channel = Encore::InputChannel::OVERDRIVE;
            break;
        }

        switch (bindingType) {
        case GUITAR: {
            switch (event->gbutton.button) {
            case(SDL_GAMEPAD_BUTTON_SOUTH):
                outevent.channel = Encore::InputChannel::LANE_1;
                break;
            case(SDL_GAMEPAD_BUTTON_EAST):
                outevent.channel = Encore::InputChannel::LANE_2;
                break;
            case(SDL_GAMEPAD_BUTTON_NORTH):
                outevent.channel = Encore::InputChannel::LANE_3;
                break;
            case(SDL_GAMEPAD_BUTTON_WEST):
                outevent.channel = Encore::InputChannel::LANE_4;
                break;
            case(SDL_GAMEPAD_BUTTON_LEFT_SHOULDER):
                outevent.channel = Encore::InputChannel::LANE_5;
                break;
            }
            break;
        }
        case GUITAR_GHPS3: {
            switch (event->gbutton.button) {
            case(SDL_GAMEPAD_BUTTON_SOUTH):
                outevent.channel = Encore::InputChannel::LANE_1;
                break;
            case(SDL_GAMEPAD_BUTTON_EAST):
                outevent.channel = Encore::InputChannel::LANE_2;
                break;
            case(SDL_GAMEPAD_BUTTON_NORTH):
                outevent.channel = Encore::InputChannel::LANE_4;
                break;
            case(SDL_GAMEPAD_BUTTON_WEST):
                outevent.channel = Encore::InputChannel::LANE_3;
                break;
            case(SDL_GAMEPAD_BUTTON_LEFT_SHOULDER):
                outevent.channel = Encore::InputChannel::LANE_5;
                break;
            }
            break;
        }
        case DRUMS: {
            switch (event->gbutton.button) {
            case(SDL_GAMEPAD_BUTTON_SOUTH):
                outevent.channel = Encore::InputChannel::LANE_5;
                break;
            case(SDL_GAMEPAD_BUTTON_EAST):
                outevent.channel = Encore::InputChannel::LANE_2;
                break;
            case(SDL_GAMEPAD_BUTTON_NORTH):
                outevent.channel = Encore::InputChannel::LANE_3;
                break;
            case(SDL_GAMEPAD_BUTTON_WEST):
                outevent.channel = Encore::InputChannel::LANE_4;
                break;
            case(SDL_GAMEPAD_BUTTON_LEFT_SHOULDER):
                outevent.channel = Encore::InputChannel::LANE_1;
                break;
            }
            break;
        }
        case PAD: {
            if (player->Instrument > PartVocals) {
                switch (event->gbutton.button) {
                case (SDL_GAMEPAD_BUTTON_LEFT_SHOULDER):
                    outevent.channel = Encore::InputChannel::LANE_2;
                    break;
                case (SDL_GAMEPAD_BUTTON_SOUTH):
                    outevent.channel = Encore::InputChannel::LANE_5;
                    break;
                case (SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER):
                    outevent.channel = Encore::InputChannel::LANE_3;
                    break;
                }
            }
            else {
                switch (event->gbutton.button) {
                case (SDL_GAMEPAD_BUTTON_DPAD_LEFT):
                    outevent.channel = Encore::InputChannel::LANE_1;
                    break;
                case (SDL_GAMEPAD_BUTTON_DPAD_UP):
                    outevent.channel = Encore::InputChannel::LANE_2;
                    break;
                case (SDL_GAMEPAD_BUTTON_DPAD_RIGHT):
                case (SDL_GAMEPAD_BUTTON_WEST):
                    outevent.channel = Encore::InputChannel::LANE_3;
                    break;
                case (SDL_GAMEPAD_BUTTON_NORTH):
                    outevent.channel = Encore::InputChannel::LANE_4;
                    break;
                case (SDL_GAMEPAD_BUTTON_EAST):
                    outevent.channel = Encore::InputChannel::LANE_5;
                    break;
                case (SDL_GAMEPAD_BUTTON_SOUTH):
                    outevent.channel = Encore::InputChannel::OVERDRIVE;
                    break;
                }
            }
            break;
        }
        }

        if (event->type == SDL_EVENT_GAMEPAD_BUTTON_UP)
            outevent.action = Encore::Action::RELEASE;
        else if (event->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN)
            outevent.action = Encore::Action::PRESS;

        outevent.slot = event->gdevice.which;
    }
    if (event->type == SDL_EVENT_GAMEPAD_AXIS_MOTION) {
        if (event->gaxis.axis == SDL_GAMEPAD_AXIS_RIGHTX) {
            outevent.channel = Encore::InputChannel::WHAMMY;
            outevent.axis = int(((float(event->gaxis.value) + 32768.0f) / 65535.0f) * 255.0f);
            if (bindingType == GUITAR_GHPS3) {
                outevent.axis = int(((float(event->gaxis.value)) / 32768.0f) * 255.0f);
            }
            outevent.slot = event->gdevice.which;
        }
        if (bindingType == PAD && player->Instrument > PartVocals) {
            if (ControllerTriggerState.find(event->gaxis.which) == ControllerTriggerState.end()) {
                ControllerTriggerState.insert({event->gaxis.axis, {false, false}});
            }
            if (event->gaxis.axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER) {
                if (event->gaxis.value > SDL_JOYSTICK_AXIS_MAX / 2 && !ControllerTriggerState[event->gaxis.which].first) {
                    ControllerTriggerState[event->gaxis.which].first = true;
                    outevent.channel = Encore::InputChannel::LANE_1;
                    outevent.action = Encore::Action::PRESS;
                } else if (ControllerTriggerState[event->gaxis.which].first){
                    ControllerTriggerState[event->gaxis.which].first = false;
                    outevent.channel = Encore::InputChannel::LANE_1;
                    outevent.action = Encore::Action::RELEASE;
                }
            }
            else if (event->gaxis.axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) {
                if (event->gaxis.value > SDL_JOYSTICK_AXIS_MAX / 2 && !ControllerTriggerState[event->gaxis.which].second) {
                    ControllerTriggerState[event->gaxis.which].second = true;
                    outevent.channel = Encore::InputChannel::LANE_4;
                    outevent.action = Encore::Action::PRESS;
                } else if (ControllerTriggerState[event->gaxis.which].second) {
                    ControllerTriggerState[event->gaxis.which].second = false;
                    outevent.channel = Encore::InputChannel::LANE_4;
                    outevent.action = Encore::Action::RELEASE;
                }
            }

            outevent.slot = event->gdevice.which;
        }
    }
    return outevent;
}

void ProcessControllerEvent(const Encore::ControllerEvent &event) {
    ZoneScoped;
    if (TheMenuManager.ActiveMenu) {
        if (OvershellMenu* menu = dynamic_cast<OvershellMenu *>(TheMenuManager.ActiveMenu.get())) {
            if (menu->hasOvershell && OvershellControllerInputCallback(menu, event)) {
                return;
            }
        }
        if (!ScanningSongs)
            TheMenuManager.ActiveMenu->ControllerInputCallback(event);
    }
}

int controllerPollRate = 1000;

void PollControllers(std::stop_token token) {
    // Mapping for Xbox One guitars under the xone linux module
    SDL_AddGamepadMapping("060074ae6f0e00004802000000010000,PDP Rock Band 4 Jaguar,a:b3,b:b4,y:b5,x:b6,leftshoulder:b7,back:b0,start:b1,guide:b2,dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,rightx:a0");

    while (!token.stop_requested()) {
        ZoneScopedN("Controller Poll Thread")
        auto start = std::chrono::high_resolution_clock::now();
        SDL_UpdateJoysticks();

        auto end = std::chrono::high_resolution_clock::now();
        auto span = std::chrono::milliseconds(1000/controllerPollRate) - (end - start);
        {
            ZoneScopedN("Sleep")
            std::this_thread::sleep_for(span);
        }
    }
}
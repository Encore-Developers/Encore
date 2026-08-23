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

bool wantsCharacterInput;

void SyncSDLWithAudio() {
    if (TheAudioManager.loadedStreams.empty()) {
        syncAudioTime = 0;
        syncSDLTicks = 0;
    }
    syncSDLTicks = SDL_GetTicksNS();
    syncAudioTime = TheSongTime.GetElapsedTime();
}

double SDLTimeToAudioTime(uint64_t ticks) {
    int64_t tickDelta = ticks - syncSDLTicks;
    double secondsDelta = (float)tickDelta * 0.000000001;
    lastTranslatedTime = syncAudioTime + secondsDelta;
    return lastTranslatedTime;
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
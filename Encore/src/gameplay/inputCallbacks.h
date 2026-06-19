//
// Created by maria on 17/12/2024.
//

#ifndef INPUTCALLBACKS_H
#define INPUTCALLBACKS_H
#include "SDL3/SDL_events.h"

#include <functional>
#include <thread>
#include <vector>
#include <mutex>

#include "util/Input.h"

// what to check when a key changes states (what was the change? was it pressed? or
// released? what time? what window? were any modifiers pressed?)
void keyCallback(SDL_KeyboardEvent* event);
void gamepadStateCallback(Encore::ControllerEvent event);
void SyncSDLWithAudio();
double SDLTimeToAudioTime(uint64_t ticks);

extern double syncAudioTime;
extern uint64_t syncSDLTicks;

extern double lastTranslatedTime;
extern int controllerPollRate;

void PollControllers(std::stop_token token);

#define MAX_EVENTS 2000

void ProcessControllerEvent(const Encore::ControllerEvent &event);
Encore::ControllerEvent TranslateSDLEvent(SDL_Event *event);
#endif //INPUTCALLBACKS_H

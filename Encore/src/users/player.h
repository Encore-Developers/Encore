#pragma once

//
// Created by marie on 04/05/2024.
//

#include "ColorPreset.h"
#include "profile.h"

#include <string>
#include <filesystem>
#include "RhythmEngine/Engine/BaseEngine.h"
#include "SDL3/SDL_gamepad.h"
#include "session/client.h"
#include "util/Input.h"

typedef unsigned int PlayerID;

enum NoteHitType {
    STANDARD, // strums/ptaps
    ALTERNATIVE // hopos/ctaps/lifts
};

namespace Encore::RhythmEngine {
    class BaseEngine;
}

enum class SignalState {
    UNSIGNALED = 0,
    SIGNALED,
    CANCELLED
};

class Player {
public:
    Player();

    Encore::ControllerBindingType bindingType = Encore::GUITAR;
    Encore::ControllerIdentity controller;
    std::shared_ptr<Encore::RhythmEngine::BaseEngine>
            engine = nullptr;
    std::shared_ptr<Profile> profile = nullptr;

    class Session* session = nullptr;
    SignalState signal = SignalState::UNSIGNALED;
    ClientID client = 0;
    PlayerID index = 0;
    int difficulty = 0;
    int instrument = 0;

    operator PlayerID() const {
        return index;
    }
};

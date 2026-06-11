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
#include "util/Input.h"



enum NoteHitType {
    STANDARD, // strums/ptaps
    ALTERNATIVE // hopos/ctaps/lifts
};

namespace Encore::RhythmEngine {
    class BaseEngine;
}


class Player {
public:
    Player();

    Encore::ControllerBindingType bindingType = Encore::GUITAR;
    Encore::ControllerIdentity controller;
    std::shared_ptr<Encore::RhythmEngine::BaseEngine>
            engine = nullptr;
    std::shared_ptr<Profile> profile = nullptr;

    // to be properly filled out later™
    class Session* session = nullptr;
    int clientId = 0;
    int difficulty = 0;
    int instrument = 0;
};

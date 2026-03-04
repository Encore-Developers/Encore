//
// Created by marie on 04/05/2024.
//
#include <fstream>
#include <iostream>
#include "player.h"

#include "uuid.h"
#include "song/scoring.h"
#include "song/song.h"

#include <random>
#include <nlohmann/json.hpp>

Player::Player() {
    Name = "New Player";
    Difficulty = 0;
    Instrument = 0;
    ReadiedUpBefore = false;
    Bot = false;

    InputCalibration = 0;
    NoteSpeed = 1;
    ProDrums = false;
    HighwayLength = 1;
    BrutalMode = false;


    joypadID = nullptr;
    LeftyFlip = false;
    Online = false;
    ClassicMode = false;
};


Encore::ColorProfile *Player::GetColorProfile() const {
    if (colorProfile) {
        return colorProfile.get();
    } else {
        return &Encore::defaultProfile;
    }
}

Color Player::QueryColorProfile(Encore::ColorSlot slot) {
    return GetColorProfile()->colors[slot];
}


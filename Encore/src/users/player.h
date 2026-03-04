#pragma once

//
// Created by marie on 04/05/2024.
//

#include "ColorProfile.h"

#include <string>
#include <filesystem>

#include "raylib.h"
#include "timingvalues.h"
#include "RhythmEngine/Engine/BaseEngine.h"
#include "song/chart.h"
#include "song/scoring.h"

#include <span>

#include "SDL3/SDL_gamepad.h"

#define PLAYER_JSON_SETTINGS                                                             \
    SETTING_ACTION(int, Difficulty, "diff")                                              \
    SETTING_ACTION(int, Instrument, "inst")                                              \
    SETTING_ACTION(float, InputCalibration, "inputOffset")                               \
    SETTING_ACTION(float, NoteSpeed, "NoteSpeed")                                        \
    SETTING_ACTION(bool, ProDrums, "proDrums")                                           \
    SETTING_ACTION(bool, Bot, "bot")                                                     \
    SETTING_ACTION(float, HighwayLength, "length")                                       \
    SETTING_ACTION(bool, ClassicMode, "classic")                                         \
    SETTING_ACTION(bool, LeftyFlip, "lefty")                                             \
    SETTING_ACTION(bool, BrutalMode, "BrutalMode")

#define PLAYER_CONFIG_LIST                                                               \
    SETTING_ACTION(int, Difficulty)                                                      \
    SETTING_ACTION(int, Instrument)                                                      \
    SETTING_ACTION(float, InputCalibration)                                              \
    SETTING_ACTION(float, NoteSpeed)                                                     \
    SETTING_ACTION(bool, ProDrums)                                                       \
    SETTING_ACTION(bool, Bot)                                                            \
    SETTING_ACTION(float, HighwayLength)                                                 \
    SETTING_ACTION(bool, ClassicMode)                                                    \
    SETTING_ACTION(bool, LeftyFlip)                                                      \
    SETTING_ACTION(bool, BrutalMode)
class Player {
    /**
     * @brief Player information. What else could be said?
     */
private:
    std::shared_ptr<Encore::ColorProfile> colorProfile;
public:
    Player();

    std::string Name; // display name
    // std::filesystem::path SettingsFile;
    // PlayerGameplayStats *stats;
    Color AccentColor = { 255, 0, 255, 255 };
#define SETTING_ACTION(type, name) type name;
    PLAYER_CONFIG_LIST;
#undef SETTING_ACTION
    std::string playerJsonObjectName;
    // todo: controller manager for assigning players to gamepads
    SDL_Gamepad *joypadID;
    bool ReadiedUpBefore;
    bool Online;
    int ActiveSlot {};
    std::shared_ptr <Encore::RhythmEngine::BaseEngine>
            engine = nullptr;

    Encore::ColorProfile *GetColorProfile() const;
    Color QueryColorProfile(Encore::ColorSlot slot);

    enum ReadyUpStates {
        PREVIEW,
        INSTRUMENT,
        DIFFICULTY
    };
    int ReadyUpMenuState = 1;

    bool instSelected = false;
    bool diffSelected = false;
    // zero indexed. local would be 0-3, online would be 4-7.
    // NOTE! this is only for like. local information and
    // not actually shared information. i was thinking of a UUID system for online
};
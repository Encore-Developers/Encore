#pragma once

//
// Created by marie on 04/05/2024.
//

#include "ColorProfile.h"

#include <string>
#include <filesystem>

#include "raylib.h"
#include "timingvalues.h"
#include "../../../out/_deps/json-src/include/nlohmann/json.hpp"
#include "PadHandler/Controller.h"
#include "RhythmEngine/Engine/BaseEngine.h"
#include "song/chart.h"
#include "song/scoring.h"

#include <span>

#include "SDL3/SDL_gamepad.h"

// #include "libstud-uuid/uuid/uuid.hxx"

class Band {
    /**
     * @brief Do Not Use. Outdated.
     * Already achieved with Band Gameplay Stats and the Band shit
     */
    std::filesystem::path ScoreFile;
    bool SoloGameplay = true; // to be true until multiple players
};

enum NoteHitType {
    STANDARD, // strums/ptaps
    ALTERNATIVE // hopos/ctaps/lifts
};
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
    Player(nlohmann::json& json) : Player() {
        LoadJSON(json);
    }

    std::string Name; // display name
    std::string PlayerID; // UUID
    // std::filesystem::path SettingsFile;
    // PlayerGameplayStats *stats;
    Color AccentColor = { 255, 0, 255, 255 };
#define SETTING_ACTION(type, name) type name;
    PLAYER_CONFIG_LIST;
#undef SETTING_ACTION
    int SongsPlayed;
    // todo: controller manager for assigning players to gamepads
    SDL_Gamepad *joypadID;
    bool ReadiedUpBefore;
    bool Online;
    int ActiveSlot {};
    Encore::Controller padState;
    std::shared_ptr <Encore::RhythmEngine::BaseEngine>
            engine = nullptr;

    nlohmann::json ToJSON();
    operator nlohmann::json() {
        return ToJSON();
    }

    void LoadJSON(nlohmann::json&);


    void ResetGameplayStats();

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

/*
class BandGameplayStats : public PlayerGameplayStats {
public:
    BandGameplayStats();

    void ResetBandGameplayStats();
    bool EligibleForGoldStars = false;
    bool Multiplayer = false;
    std::vector<int> OverdriveMultiplier { 1, 2, 4, 6, 8 };
    int PlayersInOverdrive = 0;
    bool GoldStars() {
        float starPercent = (float)Score / (float)BaseScore;
        if (starPercent >= BAND_STAR_THRESHOLD[5])
            return true;
        return false;
    };
    void AddNotePoint(bool perfect, int playerMult);
    int Stars() {
        float starPercent = (float)Score / (float)BaseScore;
        if (starPercent < BAND_STAR_THRESHOLD[0]) {
            return 0;
        } else if (starPercent < BAND_STAR_THRESHOLD[1]) {
            return 1;
        } else if (starPercent < BAND_STAR_THRESHOLD[2]) {
            return 2;
        } else if (starPercent < BAND_STAR_THRESHOLD[3]) {
            return 3;
        } else if (starPercent < BAND_STAR_THRESHOLD[4]) {
            return 4;
        } else if (starPercent < BAND_STAR_THRESHOLD[5]) {
            return 5;
        }

        return 0;
    }
    void AddClassicNotePoint(bool perfect, int playerMult, int chordSize);

    void DrumNotePoint(bool perfect, int playerMult, bool cymbal);
};
*/

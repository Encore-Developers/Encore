#pragma once

#include "ColorProfile.h"

#include <string>
#include <filesystem>

#include "raylib.h"
#include "RhythmEngine/Engine/BaseEngine.h"


#include "profiles/ProfileManager.h"


enum ControllerBindingType : uint8_t {
    GUITAR = 0,
    GUITAR_GHPS3,
    PAD,
    DRUMS
};
namespace Encore::RhythmEngine {
    class Replay;
    class ReplayPlayer;
}

namespace Encore::RhythmEngine {
    class BaseEngine;
}


#define PLAYER_JSON_SETTINGS                                                             \
    SETTING_ACTION(int, Difficulty, "diff")                                              \
    SETTING_ACTION(std::string, Name, "name")                                              \
    SETTING_ACTION(std::string, PlayerID, "UUID")                                              \
    SETTING_ACTION(int, Instrument, "inst")                                              \
    SETTING_ACTION(float, InputCalibration, "inputOffset")                               \
    SETTING_ACTION(float, NoteSpeed, "NoteSpeed")                                        \
    SETTING_ACTION(bool, Bot, "bot")                                                     \
    SETTING_ACTION(float, HighwayLength, "length")                                       \
    SETTING_ACTION(bool, LeftyFlip, "lefty")                                             \
    SETTING_ACTION(bool, BrutalMode, "BrutalMode")                                       \
    SETTING_ACTION(Color, AccentColor, "accentColor")

#define PLAYER_CONFIG_LIST                                                               \
    SETTING_ACTION(int, Difficulty)                                                      \
    SETTING_ACTION(int, Instrument)                                                      \
    SETTING_ACTION(float, InputCalibration)                                              \
    SETTING_ACTION(float, NoteSpeed)                                                     \
    SETTING_ACTION(bool, Bot)                                                            \
    SETTING_ACTION(float, HighwayLength)                                                 \
    SETTING_ACTION(bool, LeftyFlip)                                                      \
    SETTING_ACTION(bool, BrutalMode)
class Player {
    /**
     * @brief Player information. What else could be said?
     */
private:
    std::array<std::string, 3> ColorProfiles {
        "Default Profile",
        "Default Pad Profile",
        "Default Profile"
    };
public:
    Player();

    std::string Name; // display name
    std::string PlayerID; // UUID
    // std::filesystem::path SettingsFile;
    // PlayerGameplayStats *stats;
    Color AccentColor = { 255, 0, 255, 255 };
#define SETTING_ACTION(type, name) type name;
    PLAYER_CONFIG_LIST;
#undef SETTING_ACTION
    std::string playerJsonObjectName;
    int SongsPlayed;

    // If this is nonnull, this player is playing a replay back and should be considered
    // temporary
    std::shared_ptr<Encore::RhythmEngine::Replay> PlaybackReplay = nullptr;
    std::shared_ptr<Encore::RhythmEngine::ReplayPlayer> ReplayPlayer = nullptr;
    int ReplaySlot;

    ControllerBindingType bindingType = GUITAR;
    // -1 == keyboard
    // -2 == all
    SDL_JoystickID joypadID = -1;
    bool ReadiedUpBefore;
    bool Online;
    int ActiveSlot {};
    std::shared_ptr <Encore::RhythmEngine::BaseEngine>
            engine = nullptr;


    Encore::ColorProfile *GetColorProfile(Encore::ProfileManager::ColorProfileType type) const;
    void SetColorProfile(const std::string &profile, Encore::ProfileManager::ColorProfileType type);
    Color QueryColorProfile(Encore::ColorSlot slot, Encore::ProfileManager::ColorProfileType type);
};

void to_json(nlohmann::json& j, const Player& p);
void from_json(const nlohmann::json& j, Player& p);

//
// Created by marie on 04/05/2024.
//
#include <fstream>
#include "player.h"

#include "uuid.h"

#include <random>
#include <nlohmann/json.hpp>

#include "profiles/ProfileManager.h"

Player::Player() {
    Name = "New Player";
    Difficulty = 0;
    Instrument = 0;
    ReadiedUpBefore = false;
    Bot = false;

    InputCalibration = 0;
    NoteSpeed = 1;
    HighwayLength = 1;
    BrutalMode = false;
    // gen stuff, move to own function (thanks https://github.com/mariusbancila/stduuid)
    std::random_device rd;
    auto seed_data = std::array<int, std::mt19937::state_size>{};
    std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
    std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
    std::mt19937 generator(seq);
    uuids::uuid_random_generator gen{ generator };
    uuids::uuid const id = gen();

    PlayerID = uuids::to_string(id);
    SongsPlayed = 0;
    LeftyFlip = false;
    Online = false;
};

Encore::ColorProfile *Player::GetColorProfile(
    Encore::ProfileManager::ColorProfileType type) const {
    if (TheProfileManager.ColorProfiles.contains(ColorProfiles.at(type))) {
        return &TheProfileManager.ColorProfiles.find(ColorProfiles.at(type))->second;
    }
    switch (type) {
    case Encore::ProfileManager::ColorProfileType::DRUMS:
    case Encore::ProfileManager::ColorProfileType::PLASTIC:
        return &Encore::defaultPlastic;
    case Encore::ProfileManager::ColorProfileType::PAD:
        return &Encore::defaultPad;
    }
    return &Encore::defaultPlastic;
}

void Player::SetColorProfile(const std::string &profile,
                             Encore::ProfileManager::ColorProfileType type) {
    ColorProfiles.at(type) = profile;
}

Color Player::QueryColorProfile(Encore::ColorSlot slot,
                                Encore::ProfileManager::ColorProfileType type) {
    return GetColorProfile(type)->colors[slot];
}
void to_json(nlohmann::json &j, const Player &p) {
    j = nlohmann::json{
#define SETTING_ACTION(type, field, key) {key, p.field},
        PLAYER_JSON_SETTINGS
        {"colorProfiles", {
            {"plastic", p.GetColorProfile(Encore::ProfileManager::PLASTIC)->Name},
            {"pad", p.GetColorProfile(Encore::ProfileManager::PAD)->Name},
            {"drums", p.GetColorProfile(Encore::ProfileManager::DRUMS)->Name},
        }}
    #undef SETTING_ACTION
    };
}
void from_json(const nlohmann::json &j, Player &p) {
#define SETTING_ACTION(type, field, key) if (j.contains(key)) p.field = j[key];
    PLAYER_JSON_SETTINGS
#undef SETTING_ACTION
    if (j.contains("colorProfiles")) {
        auto& colorProfiles = j["colorProfiles"];
#define COLORPROFILE(key, type) if (colorProfiles.contains(key)) p.SetColorProfile(colorProfiles[key], Encore::ProfileManager::type);
        COLORPROFILE("plastic", PLASTIC)
        COLORPROFILE("drums", DRUMS)
        COLORPROFILE("pad", PAD)
#undef COLORPROFILE
    }
}
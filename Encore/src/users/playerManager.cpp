//
// Created by marie on 20/10/2024.
//

#include "playerManager.h"

#include "settings/settings.h"

#include <nlohmann/json.hpp>

#include "profiles/ProfileManager.h"
#include "profiles/ProfileManager.h"

using json = nlohmann::json;

PlayerManager::PlayerManager() {
    for (auto &p : ActivePlayers) {
        p = -1;
    }
};
PlayerManager::~PlayerManager() = default;

void PlayerManager::LoadPlayerList() {
    try {
        std::ifstream f(PlayerListSaveFile);
        json PlayerListJson = json::parse(f);
        Encore::Log::Info("Loading player list");
        for (auto &jsonObject : PlayerListJson.items()) {
            Player newPlayer;
            newPlayer.playerJsonObjectName = jsonObject.key();
            newPlayer.Name = jsonObject.value().at("name").get<std::string>();
            newPlayer.PlayerID = jsonObject.value().at("UUID").get<std::string>();

#define SETTING_ACTION(type, name, key)                                                  \
newPlayer.name = jsonObject.value().at(key).get<type>();
            PLAYER_JSON_SETTINGS;
#undef SETTING_ACTION
            if (!jsonObject.value()["colorProfiles"].is_null()) {
                std::string plasticName = jsonObject.value()["colorProfiles"].value("plastic", "Default Profile");
                std::string padName = jsonObject.value()["colorProfiles"].value("pad", "Default Pad Profile");
                std::string drumName = jsonObject.value()["colorProfiles"].value("drums", "Default Profile");
                // for (int i = 0; i < TheProfileManager.ColorProfiles.size(); i++) {
                newPlayer.SetColorProfile(plasticName, Encore::ProfileManager::PLASTIC);
                newPlayer.SetColorProfile(padName, Encore::ProfileManager::PAD);
                newPlayer.SetColorProfile(drumName, Encore::ProfileManager::DRUMS);
                // }
            }

            if (!jsonObject.value()["accentColor"].is_null()) {
                int r, g, b;
                r = jsonObject.value()["accentColor"].at("r").get<int>();
                g = jsonObject.value()["accentColor"].at("g").get<int>();
                b = jsonObject.value()["accentColor"].at("b").get<int>();
                newPlayer.AccentColor = Color{ static_cast<unsigned char>(r),
                                               static_cast<unsigned char>(g),
                                               static_cast<unsigned char>(b),
                                               255 };
            } else
                newPlayer.AccentColor = { 255, 0, 255, 255 };

            TraceLog(LOG_INFO, ("Successfully loaded player " + newPlayer.Name).c_str());
            if (newPlayer.PlayerID == "3" || newPlayer.PlayerID == "6"
                || newPlayer.PlayerID == "1") {
                // FOR GOOD MEASURE SO PEOPLE DONT HAVE TO ASK

                Encore::Log::Error("WE'RE DELETING YOUR PLAYER FILE. MAKE YOUR OWN PLAYERS.");
                Encore::Log::Error("WE'RE DELETING YOUR PLAYER FILE. MAKE YOUR OWN PLAYERS.");
                Encore::Log::Error("WE'RE DELETING YOUR PLAYER FILE. MAKE YOUR OWN PLAYERS.");
                Encore::Log::Error("WE'RE DELETING YOUR PLAYER FILE. MAKE YOUR OWN PLAYERS.");
                Encore::Log::Error("WE'RE DELETING YOUR PLAYER FILE. MAKE YOUR OWN PLAYERS.");

                remove(PlayerListSaveFile);
            } else {
                PlayerList.push_back(std::move(newPlayer));
            }
        };
    } catch (const std::exception &e) {

        Encore::Log::Error("Failed to load players. Reason: {}", e.what());
    }
}; // make player, load player stuff to PlayerList

void PlayerManager::SavePlayerList() {
    for (size_t i = 0; i < PlayerList.size(); i++) {
        SaveSpecificPlayer(i, false);
    }
}; // ough this is gonna be complicated

void PlayerManager::SaveSpecificPlayer(const int slot, bool active) {
    json PlayerListJson;
    if (exists(PlayerListSaveFile)) {
        std::ifstream f(PlayerListSaveFile);
        PlayerListJson = json::parse(f);
        f.close();
    }
    Player *player;
    if (active) {
        player = &GetActivePlayer(slot);
    } else {
        player = &PlayerList.at(slot);
    }
    if (!PlayerListJson.contains(player->playerJsonObjectName)) {
        PlayerListJson[player->playerJsonObjectName] = {
            { "name", player->Name },
            { "UUID", player->PlayerID },
#define SETTING_ACTION(type, name, key) { key, player->name },
            PLAYER_JSON_SETTINGS
#undef SETTING_ACTION
            { "colorProfiles",
              { { "plastic", player->
                GetColorProfile(Encore::ProfileManager::ColorProfileType::PLASTIC)->Name },
                { "pad", player->
                GetColorProfile(Encore::ProfileManager::ColorProfileType::PAD)->Name },
                { "drums", player->
                GetColorProfile(Encore::ProfileManager::ColorProfileType::DRUMS)->Name }
                }
            },
            { "accentColor",
              { { "r", player->AccentColor.r },
                { "g", player->AccentColor.g },
                { "b", player->AccentColor.b } } }
        };
    } else {
        PlayerListJson.at(player->playerJsonObjectName)["name"] = player->Name;
        PlayerListJson.at(player->playerJsonObjectName)["UUID"] = player->PlayerID;

#define SETTING_ACTION(type, name, key)                                                  \
    PlayerListJson.at(player->playerJsonObjectName)[key] = player->name;
        PLAYER_JSON_SETTINGS;
#undef SETTING_ACTION
        PlayerListJson.at(player->playerJsonObjectName)["colorProfiles"]["plastic"] = player->
            GetColorProfile(Encore::ProfileManager::ColorProfileType::PLASTIC)->Name;
        PlayerListJson.at(player->playerJsonObjectName)["colorProfiles"]["pad"] = player->
            GetColorProfile(Encore::ProfileManager::ColorProfileType::PAD)->Name;
        PlayerListJson.at(player->playerJsonObjectName)["colorProfiles"]["drums"] = player->
            GetColorProfile(Encore::ProfileManager::ColorProfileType::DRUMS)->Name;
        PlayerListJson.at(player->playerJsonObjectName)["accentColor"]["r"] =
            player->AccentColor.r;
        PlayerListJson.at(player->playerJsonObjectName)["accentColor"]["g"] =
            player->AccentColor.g;
        PlayerListJson.at(player->playerJsonObjectName)["accentColor"]["b"] =
            player->AccentColor.b;
    }

    Encore::Log::Info("Saved player {}", player->Name);
    Encore::WriteJsonFile(PlayerListSaveFile, PlayerListJson);
}

Player *PlayerManager::GetPlayerForJoystick(SDL_JoystickID id) {
    for (size_t i = 0; i < MAX_PLAYERS; i++) {
        auto player = ActivePlayers[i];
        if (player == -1) {
            continue;
        }
        if (GetActivePlayer(i).joypadID == id) {
            return &GetActivePlayer(i);
        }
    }
    return nullptr;
}

void PlayerManager::CreatePlayer(const std::string &name) {
    Player newPlayer;
    newPlayer.Name = name;
    newPlayer.playerJsonObjectName = name;
    PlayerList.push_back(std::move(newPlayer));
}; // set it as the next one in PlayerList

void PlayerManager::DeletePlayer(const Player &PlayerToDelete) {
    for (size_t i = 0; i < MAX_PLAYERS; i++) {
        auto player = ActivePlayers[i];
        if (player == -1) {
            continue;
        }
        if (&PlayerList[player] == &PlayerToDelete) {
            RemoveActivePlayer(player);
            ActivePlayers[i] = -1;
            // HACK: force the slot to -1 in case the player was assigned more than once
        }
    }
    for (auto iter = PlayerList.begin(); iter != PlayerList.end(); iter++) {
        if (&*iter == &PlayerToDelete) {
            PlayerList.erase(iter++);
            break;
        }
    }
}; // remove player, reload playerlist
void PlayerManager::RenamePlayer(const Player &PlayerToRename) {
}; // rename player

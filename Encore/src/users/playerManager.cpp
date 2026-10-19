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
    ActivePlayers.resize(MAX_PLAYERS);
    for (auto &p : ActivePlayers) {
        p = nullptr;
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
            from_json(jsonObject.value(), newPlayer);
            newPlayer.playerJsonObjectName = jsonObject.key();
            TraceLog(LOG_INFO, ("Successfully loaded player " + newPlayer.Name).c_str());
            PlayerList.push_back(std::make_shared<Player>(newPlayer));
        };
    } catch (const std::exception &e) {

        Encore::Log::Error("Failed to load players. Reason: {}", e.what());
    }
}; // make player, load player stuff to PlayerList

void PlayerManager::SavePlayerList() {
    json json;
    for (size_t i = 0; i < PlayerList.size(); i++) {
        json[PlayerList[i]->playerJsonObjectName] = *PlayerList[i];
    }
    Encore::Log::Info("Saved players");
    Encore::WriteJsonFile(PlayerListSaveFile, json);
}

Player *PlayerManager::GetPlayerForJoystick(SDL_JoystickID id) {
    for (size_t i = 0; i < ThePlayerManager.ActivePlayers.size(); i++) {
        auto player = ActivePlayers[i];
        if (!player) {
            continue;
        }
        if (player->joypadID == id) {
            return player.get();
        }
    }
    return nullptr;
}

void PlayerManager::CreatePlayer(const std::string &name) {
    Player newPlayer;
    newPlayer.Name = name;
    newPlayer.playerJsonObjectName = name;
    PlayerList.push_back(std::make_shared<Player>(newPlayer));
}; // set it as the next one in PlayerList

void PlayerManager::DeletePlayer(const Player &PlayerToDelete) {
    for (size_t i = 0; i < ThePlayerManager.ActivePlayers.size(); i++) {
        auto player = ActivePlayers[i];
        if (!player) {
            continue;
        }
        if (player.get() == &PlayerToDelete) {
            RemoveActivePlayer(i);
            // HACK: force the slot to -1 in case the player was assigned more than once
        }
    }
    for (auto iter = PlayerList.begin(); iter != PlayerList.end(); iter++) {
        if (iter->get() == &PlayerToDelete) {
            PlayerList.erase(iter++);
            break;
        }
    }
}; // remove player, reload playerlist
void PlayerManager::RenamePlayer(const Player &PlayerToRename) {
}; // rename player

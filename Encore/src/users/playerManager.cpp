//
// Created by marie on 20/10/2024.
//

#include "playerManager.h"

#include "settings/settings.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

PlayerManager::PlayerManager() = default;
PlayerManager::~PlayerManager() = default;

void PlayerManager::LoadPlayerList() {
    try {
        std::ifstream f(PlayerListSaveFile);
        json PlayerListJson = json::parse(f);
        TraceLog(LOG_INFO, "Loading player list");
        for (auto& [key, value] : PlayerListJson.items()) {
            SavedPlayers.push_back(value);
        }
    } catch (const std::exception &e) {
        Encore::EncoreLog(
            LOG_ERROR, TextFormat("Failed to load players. Reason: %s", e.what())
        );
    }
}; // make player, load player stuff to PlayerList

void PlayerManager::SavePlayerList() {
    json out;
    for (int i = 0; i < SavedPlayers.size(); i++) {
        auto &player = SavedPlayers[i];
        out.push_back(player);
    }
    WriteJsonFile(PlayerListSaveFile, out);
}


void PlayerManager::CreatePlayer(const std::string &name) {
    Player newPlayer;
    newPlayer.Name = name;
    SavedPlayers.push_back(std::move(newPlayer));
}; // set it as the next one in PlayerList

void PlayerManager::DeletePlayer(Player &PlayerToDelete) {
    for (auto it = SavedPlayers.begin(); it != SavedPlayers.end(); it++) {
        if (&*it == &PlayerToDelete) {
            SavedPlayers.erase(it);
            break;
        }
    }
}; // remove player, reload playerlist
void PlayerManager::RenamePlayer(const Player &PlayerToRename) {

}; // rename player

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
        for (auto &jsonObject : PlayerListJson.items()) {
            Player newPlayer;
            newPlayer.playerJsonObjectName = jsonObject.key();
            newPlayer.Name = jsonObject.value().at("name").get<std::string>();
            newPlayer.PlayerID = jsonObject.value().at("UUID").get<std::string>();

#define SETTING_ACTION(type, name, key)                                                  \
newPlayer.name = jsonObject.value().at(key).get<type>();
            PLAYER_JSON_SETTINGS;
#undef SETTING_ACTION

            if (!jsonObject.value()["accentColor"].is_null()) {
                int r, g, b;
                r = jsonObject.value()["accentColor"].at("r").get<int>();
                g = jsonObject.value()["accentColor"].at("g").get<int>();
                b = jsonObject.value()["accentColor"].at("b").get<int>();
                newPlayer.AccentColor = Color { static_cast<unsigned char>(r),
                                                static_cast<unsigned char>(g),
                                                static_cast<unsigned char>(b),
                                                255 };
            } else
                newPlayer.AccentColor = { 255, 0, 255, 255 };

            TraceLog(LOG_INFO, ("Successfully loaded player " + newPlayer.Name).c_str());
            if (newPlayer.PlayerID == "3" || newPlayer.PlayerID == "6"
                || newPlayer.PlayerID == "1") {
                // FOR GOOD MEASURE SO PEOPLE DONT HAVE TO ASK
                Encore::EncoreLog(
                    LOG_ERROR, "WE'RE DELETING YOUR PLAYER FILE. MAKE YOUR OWN PLAYERS."
                );
                Encore::EncoreLog(
                    LOG_ERROR, "WE'RE DELETING YOUR PLAYER FILE. MAKE YOUR OWN PLAYERS."
                );
                Encore::EncoreLog(
                    LOG_ERROR, "WE'RE DELETING YOUR PLAYER FILE. MAKE YOUR OWN PLAYERS."
                );
                Encore::EncoreLog(
                    LOG_ERROR, "WE'RE DELETING YOUR PLAYER FILE. MAKE YOUR OWN PLAYERS."
                );
                Encore::EncoreLog(
                    LOG_ERROR, "WE'RE DELETING YOUR PLAYER FILE. MAKE YOUR OWN PLAYERS."
                );

                remove(PlayerListSaveFile);
            } else {
                PlayerList.push_back(std::move(newPlayer));
            }
        };
    } catch (const std::exception &e) {
        Encore::EncoreLog(
            LOG_ERROR, TextFormat("Failed to load players. Reason: %s", e.what())
        );
    }
}; // make player, load player stuff to PlayerList

void PlayerManager::SavePlayerList() {
    for (int i = 0; i < PlayerList.size(); i++) {
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

        PlayerListJson.at(player->playerJsonObjectName)["accentColor"]["r"] =
            player->AccentColor.r;
        PlayerListJson.at(player->playerJsonObjectName)["accentColor"]["g"] =
            player->AccentColor.g;
        PlayerListJson.at(player->playerJsonObjectName)["accentColor"]["b"] =
            player->AccentColor.b;
    }

    Encore::WriteJsonFile(PlayerListSaveFile, PlayerListJson);
}

void PlayerManager::CreatePlayer(const std::string &name) {
    Player newPlayer;
    newPlayer.Name = name;
    newPlayer.playerJsonObjectName = name;
    PlayerList.push_back(std::move(newPlayer));
}; // set it as the next one in PlayerList

void PlayerManager::DeletePlayer(const Player &PlayerToDelete) {

}; // remove player, reload playerlist
void PlayerManager::RenamePlayer(const Player &PlayerToRename) {

}; // rename player

//
// Created by marie on 20/10/2024.
//

#include "playerManager.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

PlayerManager::PlayerManager() {}
PlayerManager::~PlayerManager() {}

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
            newPlayer.Difficulty = jsonObject.value().at("diff").get<int>();
            newPlayer.Instrument = jsonObject.value().at("inst").get<int>();
            newPlayer.NoteSpeed = jsonObject.value().at("NoteSpeed").get<float>();
            newPlayer.InputCalibration =
                jsonObject.value().at("inputOffset").get<float>();

            if (!jsonObject.value()["length"].is_null())
                newPlayer.HighwayLength = jsonObject.value().at("length").get<float>();
            else
                newPlayer.HighwayLength = 1.0f;

            newPlayer.Bot = jsonObject.value().at("bot").get<bool>();
            newPlayer.ClassicMode = jsonObject.value().at("classic").get<bool>();
            newPlayer.ProDrums = jsonObject.value().at("proDrums").get<bool>();
            newPlayer.LeftyFlip = jsonObject.value().at("lefty").get<bool>();
            if (!jsonObject.value()["accentColor"].is_null()) {
                int r = 0, g = 0, b = 0;
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
            PlayerList.push_back(std::move(newPlayer));
        };
    } catch (const std::exception &e) {
        Encore::EncoreLog(
            LOG_ERROR, TextFormat("Failed to load players. Reason: %s", e.what())
        );
    }
}; // make player, load player stuff to PlayerList

void PlayerManager::SavePlayerList() {
    for (int i = 0; i < PlayerList.size(); i++) {
        SaveSpecificPlayer(i);
    }
}; // ough this is gonna be complicated

void PlayerManager::SaveSpecificPlayer(int slot) {
    json PlayerListJson;
    if (exists(PlayerListSaveFile)) {
        std::ifstream f(PlayerListSaveFile);
        PlayerListJson = json::parse(f);
        f.close();
    }
    Player *player = GetActivePlayer(slot);
    if (!PlayerListJson.contains(player->playerJsonObjectName)) {
        PlayerListJson[player->playerJsonObjectName] = {
            {"name", player->Name},
            {"UUID", player->PlayerID},
#define SETTING_ACTION(type, name, key) {key, player->name},
            PLAYER_JSON_SETTINGS
#undef SETTING_ACTION
            {"accentColor",{
                {"r", player->AccentColor.r},
                {"g", player->AccentColor.g},
                {"b", player->AccentColor.b}
            }}
        };
    } else {
        PlayerListJson.at(player->playerJsonObjectName)["name"] = player->Name;
        PlayerListJson.at(player->playerJsonObjectName)["UUID"] = player->PlayerID;

#define SETTING_ACTION(type, name, key) PlayerListJson.at(player->playerJsonObjectName)[key] = player->name;
        PLAYER_JSON_SETTINGS;
#undef SETTING_ACTION

        PlayerListJson.at(player->playerJsonObjectName)["accentColor"]["r"] = player->AccentColor.r;
        PlayerListJson.at(player->playerJsonObjectName)["accentColor"]["g"] = player->AccentColor.g;
        PlayerListJson.at(player->playerJsonObjectName)["accentColor"]["b"] = player->AccentColor.b;
    }


    std::ofstream o(PlayerListSaveFile, std::ios::out | std::ios::trunc);
    o << PlayerListJson.dump(2, ' ', false, nlohmann::detail::error_handler_t::strict);
    o.close();
}

void PlayerManager::CreatePlayer(std::string name) {
    Player newPlayer;
    newPlayer.Name = name;
    newPlayer.playerJsonObjectName = name;
    PlayerList.push_back(std::move(newPlayer));
}; // set it as the next one in PlayerList

void PlayerManager::DeletePlayer(Player PlayerToDelete) {

}; // remove player, reload playerlist
void PlayerManager::RenamePlayer(Player PlayerToRename) {

}; // rename player

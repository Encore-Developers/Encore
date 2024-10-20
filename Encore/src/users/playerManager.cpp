//
// Created by marie on 20/10/2024.
//

#include "playerManager.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

PlayerManager::PlayerManager() {}
PlayerManager::~PlayerManager() {}

void PlayerManager::LoadPlayerList(std::filesystem::path PlayerListSaveFile) {
    std::ifstream f(PlayerListSaveFile);
    json PlayerListJson = json::parse(f);
    TraceLog(LOG_INFO, "Loading player list");
    for (auto jsonObject : PlayerListJson) {
        Player newPlayer;
        newPlayer.Name = jsonObject.at("name").get<std::string>();
        newPlayer.PlayerID = jsonObject.at("UUID").get<std::string>();
        newPlayer.Difficulty = jsonObject.at("diff").get<int>();
        newPlayer.Instrument = jsonObject.at("inst").get<int>();
        newPlayer.NoteSpeed = jsonObject.at("NoteSpeed").get<float>();
        newPlayer.InputCalibration = jsonObject.at("inputOffset").get<float>();

        if (!jsonObject["length"].is_null())
            newPlayer.HighwayLength = jsonObject.at("length").get<float>();
        else newPlayer.HighwayLength = 1.0f;

        newPlayer.Bot = jsonObject.at("bot").get<bool>();
        newPlayer.ClassicMode = jsonObject.at("classic").get<bool>();
        newPlayer.ProDrums = jsonObject.at("proDrums").get<bool>();
        newPlayer.LeftyFlip = jsonObject.at("lefty").get<bool>();
        if (!jsonObject["accentColor"].is_null()) {
            int r = 0, g = 0, b = 0;
            r = jsonObject["accentColor"].at("r").get<int>();
            g = jsonObject["accentColor"].at("g").get<int>();
            b = jsonObject["accentColor"].at("b").get<int>();
            newPlayer.AccentColor = Color { static_cast<unsigned char>(r),
                                            static_cast<unsigned char>(g),
                                            static_cast<unsigned char>(b),
                                            255 };
        } else
            newPlayer.AccentColor = { 255, 0, 255, 255 };

        TraceLog(LOG_INFO, ("Successfully loaded player " + newPlayer.Name).c_str());
        PlayerList.push_back(std::move(newPlayer));
    };
}; // make player, load player stuff to PlayerList

void PlayerManager::SavePlayerList(std::filesystem::path PlayerListSaveFile) {
    json PlayerListJson;
    for (auto &player : PlayerList) {
        PlayerListJson;
    }
}; // ough this is gonna be complicated

void PlayerManager::CreatePlayer(Player player) {

}; // set it as the next one in PlayerList

void PlayerManager::DeletePlayer(Player PlayerToDelete) {

}; // remove player, reload playerlist
void PlayerManager::RenamePlayer(Player PlayerToRename) {

}; // rename player

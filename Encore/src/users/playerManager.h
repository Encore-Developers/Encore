#pragma once
//
// Created by marie on 20/10/2024.
//

#include "player.h"
#include "rapidjson/document.h"

class PlayerManager {
public:
    PlayerManager();
    ~PlayerManager();
    void MakePlayerDirectory(); // run on initialization?
    void LoadPlayerList(std::filesystem::path PlayerListSaveFile); // make player, load
    // player stuff to
    // PlayerList
    void SavePlayerList(std::filesystem::path PlayerListSaveFile);
    BandGameplayStats BandStats = BandGameplayStats();
    rapidjson::Document PlayerListFile;
    std::vector<Player> PlayerList;
    std::vector<int> ActivePlayers { -1, -1, -1, -1 };
    int PlayersActive = 0;

    Player *GetActivePlayer(int slot) { return &PlayerList.at(ActivePlayers.at(slot)); }

    void AddActivePlayer(int playerNum, int slot) {
        ActivePlayers.at(slot) = playerNum;
        GetActivePlayer(slot)->joypadID = slot;
        PlayersActive += 1;
    }

    void RemoveActivePlayer(int slot) {
        ActivePlayers.at(slot) = -1;
        PlayersActive -= 1;
    }

    bool IsGamepadActive(int joystickID) {
        for (int playesr = 0; playesr < PlayersActive; playesr++) {
            if (GetActivePlayer(playesr)->joypadID == joystickID) {
                return true;
            }
        }
        return false;
    }

    Player *GetPlayerGamepad(int joystickID) {
        for (int playesr = 0; playesr < PlayersActive; playesr++) {
            if (GetActivePlayer(playesr)->joypadID == joystickID) {
                return GetActivePlayer(playesr);
            }
        }
        return nullptr;
    }

    void CreatePlayer(Player player);
    void DeletePlayer(Player PlayerToDelete); // remove player, reload playerlist
    void RenamePlayer(Player PlayerToRename); // rename player
};

extern PlayerManager ThePlayerManager;

#pragma once
//
// Created by marie on 20/10/2024.
//

#include "player.h"
#include "overshell/Overshell.h"
#include "overshell/OvershellSlot.h"
#include "util/discord.h"

using namespace Encore;

class PlayerManager {
public:
    PlayerManager();
    ~PlayerManager();
    void MakePlayerDirectory(); // run on initialization?
    void LoadPlayerList(); // make player, load
    // player stuff to
    // PlayerList
    void SavePlayerList();
    // BandGameplayStats *BandStats;
    std::filesystem::path PlayerListSaveFile;
    std::vector<Player> SavedPlayers;
    std::vector<OvershellSlot> ActivePlayers;

    Overshell overshell = Overshell(ActivePlayers);

    void SetPlayerListSaveFileLocation(std::filesystem::path file) {
        PlayerListSaveFile = file;
    }

    void AddActivePlayer(Player* player) {
        ActivePlayers.emplace_back(player);
        TheGameRPC.DiscordUpdatePresence("In the menus", "In the menus", ActivePlayers.size());
    }

    void RemoveActivePlayer(Player* player) {
        for (auto it = ActivePlayers.begin(); it != ActivePlayers.end(); ++it) {
            if (it->player == player) {
                ActivePlayers.erase(it);
                break;
            }
        }
        TheGameRPC.DiscordUpdatePresence("In the menus", "In the menus", ActivePlayers.size());
    }

    void CreatePlayer(const std::string &name);
    void DeletePlayer(Player &PlayerToDelete); // remove player, reload playerlist
    void RenamePlayer(const Player &PlayerToRename); // rename player
};

extern PlayerManager ThePlayerManager;

#pragma once
//
// Created by marie on 20/10/2024.
//

#include "player.h"
#include "util/Presence.h"
#include "util/enclog.h"

#define MAX_PLAYERS 4



class PlayerManager {
public:
    PlayerManager();
    ~PlayerManager();
    void MakePlayerDirectory(); // run on initialization?
    void LoadPlayerList(); // make player, load
    // player stuff to
    // PlayerList
    void SavePlayerList();
    void SaveSpecificPlayer(std::shared_ptr<Player> player, bool active);
    // BandGameplayStats *BandStats;
    std::filesystem::path PlayerListSaveFile;
    std::vector<std::shared_ptr<Player>> PlayerList;
    std::array<std::shared_ptr<Player>, MAX_PLAYERS> ActivePlayers;
    int PlayersActive = 0;

    Player* GetPlayerForJoystick(SDL_JoystickID id);

    Player &GetActivePlayer(int slot) {
        auto player = ActivePlayers.at(slot);
        assert(player >= 0);
        return *player;
    }

    void SetPlayerListSaveFileLocation(std::filesystem::path file) {
        PlayerListSaveFile = file;
    }

    void AddActivePlayer(std::shared_ptr<Player> player, int slot) {
        ActivePlayers.at(slot) = player;
        // GetActivePlayer(slot).joypadID = slot;
        PlayersActive += 1;
        TheGameRPC.DiscordUpdatePresence("In the menus", "In the menus",PlayersActive);
    }

    void RemoveActivePlayer(int slot) {
        ActivePlayers.at(slot) = nullptr;
        PlayersActive -= 1;
        TheGameRPC.DiscordUpdatePresence("In the menus", "In the menus",PlayersActive);
    }

    void CullTempPlayers() {
        for (auto &player : ActivePlayers) {
            if (!player) continue;
            if (player->PlaybackReplay) {
                player = nullptr;
                PlayersActive --;
            }
        }
    }

    bool IsGamepadActive(int joystickID) {
        for (int playesr = 0; playesr < PlayersActive; playesr++) {
            // if (GetActivePlayer(playesr).joypadID == joystickID) {
            //     return true;
            // }
        }
        return false;
    }

    void CreatePlayer(const std::string &name);
    void DeletePlayer(const Player &PlayerToDelete); // remove player, reload playerlist
    void RenamePlayer(const Player &PlayerToRename); // rename player
};

extern PlayerManager ThePlayerManager;

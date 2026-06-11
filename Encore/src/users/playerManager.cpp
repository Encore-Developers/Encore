//
// Created by marie on 20/10/2024.
//

#include "playerManager.h"

#include "settings/settings.h"

#include <nlohmann/json.hpp>

#include "profiles/ProfileManager.h"
#include "profiles/ProfileManager.h"

using json = nlohmann::json;

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

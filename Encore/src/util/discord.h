//
// Created by maria on 16/12/2024.
//

#ifndef DISCORD_H
#define DISCORD_H


#include <memory>
#include <string>


#ifdef STEAM
#include "isteamfriends.h"
#include "steam_api_common.h"
#endif

namespace Encore {
    class Discord {

        int64_t startTime;
#ifdef STEAM
        STEAM_CALLBACK( Discord, OnOverlayOpen, GameOverlayActivated_t);
#endif
    public:
        bool IsOverlayOpen = false;
        bool Initialized = false;
        void Initialize(std::string discordBoot);
        ~Discord();
        void DiscordUpdatePresence(
            const std::string &title, const std::string &details, int players
        );
        void DiscordUpdatePresenceSong(const std::string &title, const std::string &details, int instrument, int players);
        void Update();
        void SteamUpdatePresence(const char* key, const char* value);
        std::string GetSteamNickname();
        void SteamShowKeyboard();
        void SteamOverlayPosition(bool top);
    };
};

extern Encore::Discord TheGameRPC;

#endif //DISCORD_H

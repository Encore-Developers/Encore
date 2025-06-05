//
// Created by maria on 16/12/2024.
//

#ifndef DISCORD_H
#define DISCORD_H
#include "discord-rpc/discord_rpc.h"

#include <string>

namespace Encore {
    class Discord {

        DiscordRichPresence presence;
    public:
        Discord();
        ~Discord();
        void DiscordUpdatePresence(
            const std::string &title, const std::string &details
        );
        void DiscordUpdatePresenceSong(const std::string &title, const std::string &details, int instrument);
    };
};

extern Encore::Discord TheGameRPC;

#endif //DISCORD_H

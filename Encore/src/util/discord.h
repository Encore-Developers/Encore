//
// Created by maria on 16/12/2024.
//

#ifndef DISCORD_H
#define DISCORD_H


#include <memory>
#include <string>


namespace Encore {
    class Discord {

        int64_t startTime;
    public:
        bool Initialized = false;
        void Initialize(std::string discordBoot);
        ~Discord();
        void DiscordUpdatePresence(
            const std::string &title, const std::string &details, int players
        );
        void DiscordUpdatePresenceSong(const std::string &title, const std::string &details, int instrument, int players);
        void Update();
        void SteamUpdatePresence(const char* key, const char* value);
    };
};

extern Encore::Discord TheGameRPC;

#endif //DISCORD_H

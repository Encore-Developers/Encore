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
        void Initialize();
        ~Discord();
        void DiscordUpdatePresence(
            const std::string &title, const std::string &details, int players
        );
        void DiscordUpdatePresenceSong(const std::string &title, const std::string &details, int instrument, int players);
        void Update();
    };
};

extern Encore::Discord TheGameRPC;

#endif //DISCORD_H

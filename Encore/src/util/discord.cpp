//
// Created by maria on 16/12/2024.
//

#include "discord.h"

#include "discord-rpc/discord_rpc.h"

#include <array>
#include <ctime>

Encore::Discord::Discord() : presence() {
    DiscordEventHandlers Handlers {};
    Discord_Initialize("1216298119457804379", &Handlers, 1, nullptr);
}

Encore::Discord::~Discord() {
    Discord_Shutdown();
}

void Encore::Discord::DiscordUpdatePresence(const std::string &title, const std::string &details) {
    presence.largeImageKey = "encore";
    presence.state = title.c_str();
    presence.details = details.c_str();
    Discord_UpdatePresence(&presence);
}

std::array<std::string, 11> AssetNames = {
    "pad_drums",    "pad_bass",       "pad_guitar",    "pad_keys",
    "pad_vocals",   "classic_drums",  "classic_bass",  "classic_guitar",
    "classic_keys", "classic_vocals", "classic_vocals"
};

void Encore::Discord::DiscordUpdatePresenceSong(
    const std::string &title, const std::string &details, int instrument
) {
    presence.smallImageKey = AssetNames[instrument].c_str();
    presence.largeImageKey = "encore";
    presence.state = title.c_str();
    presence.details = details.c_str();
    Discord_UpdatePresence(&presence);
}

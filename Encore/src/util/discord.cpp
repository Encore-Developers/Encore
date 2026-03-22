//
// Created by maria on 16/12/2024.
//

#include "discord.h"
#include "discord-rpc/core.h"
#include <array>
#include <ctime>
#include <iostream>
#include <chrono>

discord::Core* core{};

void Encore::Discord::Initialize(std::string discordBoot) {
    if (discordBoot != "false") {
        auto result = discord::Core::Create(1216298119457804379, DiscordCreateFlags_Default, &core);
        if (!core) {
            std::cout << "Failed to instantiate discord core! (err " << static_cast<int>(result)
                      << ")\n";
            Initialized = false;
            return;
        }
        const auto p0 = std::chrono::system_clock::now();
        startTime = std::chrono::duration_cast<std::chrono::seconds>(p0.time_since_epoch()).count();
        /*
        try {
            discord::Core::Create(1216298119457804379, DiscordCreateFlags_Default, &core);
            //DiscordEventHandlers Handlers {};
            // Discord_Initialize("1216298119457804379", &Handlers, 1, nullptr);
        }
        catch (const std::exception& e) {
            Initialized = false;
        //     return;
        }
        */
        Initialized = true;
    }
}

Encore::Discord::~Discord() {
    if (!Initialized)
        return;
    Initialized = false;
    core->ActivityManager().ClearActivity([](discord::Result result){ });
    core = nullptr;
}
void Encore::Discord::Update() {
    if (!Initialized)
        return;
    core->RunCallbacks();
}

void Encore::Discord::DiscordUpdatePresence(const std::string &title, const std::string &details, int players) {
    if (!Initialized)
        return;
    discord::Activity activity{};
    // activity.SetState(title.c_str());
    if (players == 1) {
        activity.SetState("Playing solo");
    }
    else {
        activity.SetState("In a band");
    }
    activity.SetDetails((details).c_str());
    activity.SetName("encore");
    activity.GetAssets().SetLargeImage("encore");
    activity.SetType(discord::ActivityType::Playing);
    activity.GetTimestamps().SetStart(startTime);
    core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
        std::cout << ((result == discord::Result::Ok) ? "Succeeded" : "Failed")
                  << " updating activity!\n";
    });
}

std::array<std::string, 11> AssetNames = {
    "pad_drums",    "pad_bass",       "pad_guitar",    "pad_keys",
    "pad_vocals",   "classic_drums",  "classic_bass",  "classic_guitar",
    "classic_keys", "classic_vocals", "classic_vocals"
};

std::array<std::string, 11> PartNames = {
    "Pad Drums",    "Pad Bass",       "Pad_guitar",    "Pad keys",
    "Pad vocals",   "Classic Drums",  "Classic Bass",  "Classic Guitar",
    "Classic Keys", "Classic Vocals", "Classic Vocals"
};

void Encore::Discord::DiscordUpdatePresenceSong(
    const std::string &title, const std::string &details, int instrument, int length
) {
    if (!Initialized)
        return;
    discord::Activity activity{};
    activity.SetDetails((details).c_str());
    if (length == 1) {
        activity.SetState("Playing solo");
    }
    else {
        activity.SetState("In a band");
    }
    activity.SetName("encore");
    activity.GetAssets().SetLargeImage("encore");
    activity.GetAssets().SetSmallImage(AssetNames[instrument].c_str());
    activity.GetAssets().SetSmallText(PartNames[instrument].c_str());
    activity.SetType(discord::ActivityType::Playing);
    activity.GetTimestamps().SetStart(startTime);
    //const auto p1 = std::chrono::system_clock::now();
    //const auto end = p1 + std::chrono::seconds(length);
    //int64_t endTime = std::chrono::duration_cast<std::chrono::seconds>(end.time_since_epoch()).count();
    //int64_t sStartTime = std::chrono::duration_cast<std::chrono::seconds>(p1.time_since_epoch()).count();
    core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
        std::cout << ((result == discord::Result::Ok) ? "Succeeded" : "Failed")
                  << " updating activity!\n";
    });
}

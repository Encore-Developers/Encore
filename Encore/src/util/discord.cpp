//
// Created by maria on 16/12/2024.
//

#include "discord.h"

#ifdef STEAM
#include "isteamfriends.h"
#include "isteamutils.h"
#include "isteaminput.h"
#include "isteamuser.h"
#endif

#include "discord-rpc/core.h"
#include <array>
#include <ctime>
#include <iostream>
#include <chrono>


discord::Core* core{};

std::string discordCommitHash = GIT_COMMIT_HASH;
std::string discordVersion = ENCORE_VERSION;
std::string discordgitBranch = GIT_BRANCH;
std::string discordBuildDate = BUILDDATE;

#ifdef STEAM
void Encore::Discord::OnOverlayOpen( GameOverlayActivated_t* callback ) {
    if (callback->m_bActive) {
        IsOverlayOpen = true;
    }
}
#endif
void Encore::Discord::Initialize(std::string discordBoot) {
    if (discordBoot != "false") {
        auto result = discord::Core::Create(1216298119457804379, DiscordCreateFlags_NoRequireDiscord, &core);
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
void Encore::Discord::SteamUpdatePresence(const char *key, const char *value) {
#ifdef STEAM
    SteamFriends()->SetRichPresence(key, value);
#endif
}

std::string Encore::Discord::GetSteamNickname() {
#ifdef STEAM
    if (SteamFriends()->GetPersonaName()) {
        return SteamFriends()->GetPersonaName();
    }
    return "Player";
#endif
}

void Encore::Discord::SteamShowKeyboard() {
#ifdef STEAM
    SteamUtils()->ShowFloatingGamepadTextInput(k_EFloatingGamepadTextInputModeModeEmail, 1920/4, 0, 1920/2, 1080/2 );
#endif
}
void Encore::Discord::SteamOverlayPosition(bool top) {
    #ifdef STEAM
    ENotificationPosition notificationPosition = k_EPositionBottomRight;
    if (top) {
        notificationPosition = k_EPositionTopRight;
    }
    SteamUtils()->SetOverlayNotificationPosition(notificationPosition);
    #endif
}

/*
const char *Encore::Discord::GetGlyphPath(Uint64 GamepadSteamHandle,
                                          RhythmEngine::InputChannel channel) {
#ifdef STEAM

    std::string ActionName;
    switch (channel) {
    case RhythmEngine::InputChannel::LANE_1: {
        ActionName = "menu_confirm";
        break;
    }
    case RhythmEngine::InputChannel::LANE_2: {
        ActionName = "menu_back";
        break;
    }
    case RhythmEngine::InputChannel::LANE_3: {
        ActionName = "menu_secondary";
        break;
    }
    case RhythmEngine::InputChannel::LANE_4: {
        ActionName = "menu_tertiary";
        break;
    }
    case RhythmEngine::InputChannel::LANE_5: {
        ActionName = "menu_modifier";
        break;
    }
    case RhythmEngine::InputChannel::STRUM_UP: {
            ActionName = "menu_up";
        break;
        }
    case RhythmEngine::InputChannel::STRUM_DOWN: {
            ActionName = "menu_down";
        break;
        }
    case RhythmEngine::InputChannel::INPUT_LEFT: {
            ActionName = "menu_left";
        break;
        }
    case RhythmEngine::InputChannel::INPUT_RIGHT: {
            ActionName = "menu_right";
        break;
        }
    }
    InputDigitalActionHandle_t DigitalActionHandle = SteamInput()->GetDigitalActionHandle(ActionName.c_str());
    InputActionSetHandle_t ActionSet = SteamInput()->GetActionSetHandle("MenuControls");
    EInputActionOrigin ActionOrigin[STEAM_INPUT_MAX_ORIGINS];
    int result = SteamInput()->GetDigitalActionOrigins(2, ActionSet, DigitalActionHandle, ActionOrigin);
    //if (result) {
        return SteamInput()->GetGlyphPNGForActionOrigin(ActionOrigin[0], k_ESteamInputGlyphSize_Small, ESteamInputGlyphStyle_Knockout);
    //}
    //return "";

    // look. this doesnt work. id like to see it work, but i doubt it will, considering
    // making it working would require a full ass switch to Using SteamInput.
    // could be cool at some point, ESPECIALLY for rebinding, but i dont like the idea
    // of limiting an accessibility tool (controller rebinding) behind installing the game
    // on Steam and persistently using (and hoping) the platform doesnt hit EOL.

#endif
}
*/
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
    discordCommitHash.resize(6);
    activity.GetAssets().SetLargeText((discordVersion + "-" + discordgitBranch + ":" + discordCommitHash + " " + discordBuildDate).c_str());
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
        activity.GetAssets().SetSmallText(PartNames[instrument].c_str());
        activity.GetAssets().SetSmallImage(AssetNames[instrument].c_str());
    }
    else {
        activity.SetState("In a band");
    }
    activity.SetName("encore");
    activity.GetAssets().SetLargeImage("encore");
    // "%s-%s:%s", menuVersion.c_str(), gitBranch.c_str(), menuCommitHash.c_str()3.
    discordCommitHash.resize(6);
    activity.GetAssets().SetLargeText((discordVersion + "-" + discordgitBranch + ":" + discordCommitHash + " " + discordBuildDate).c_str());
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

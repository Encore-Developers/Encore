#ifndef SETTINGS_H
#define SETTINGS_H

#include <array>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <nlohmann/json.hpp>

#ifndef PLATFORM_NX
#include <filesystem>
#endif

#define SETTINGS_OPTIONS \
    OPTION(float, avMainVolume, 0.5f) \
    OPTION(float, avActiveInstrumentVolume, 0.75f) \
    OPTION(float, avInactiveInstrumentVolume, 0.5f) \
    OPTION(float, avSoundEffectVolume, 0.5f) \
    OPTION(float, avMuteVolume, 0.15f) \
    OPTION(float, avMenuMusicVolume, 0.15f) \
    OPTION(bool, Fullscreen, false) \
    OPTION(int, AudioOffset, 0) \
    OPTION(bool, DiscordRichPresence, true) \
    OPTION(int, Framerate, 60) \
    OPTION(bool, VerticalSync, true) \
    OPTION(bool, BackgroundBeatFlash, true)

namespace Encore {
    class Settings {
    public:
        #define OPTION(type, value, default) type value = default;
        SETTINGS_OPTIONS
        #undef OPTION
        #ifndef PLATFORM_NX
        std::vector<std::filesystem::path> SongPaths;
        #else
        std::vector<std::string> SongPaths;
        #endif
        void SaveToFile(const std::string& filename) const;
        void LoadFromFile(const std::string& filename);
    };

    #ifndef PLATFORM_NX
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
        Settings,
        avMainVolume,
        avActiveInstrumentVolume,
        avInactiveInstrumentVolume,
        avSoundEffectVolume,
        avMuteVolume,
        avMenuMusicVolume,
        Fullscreen,
        Framerate,
        VerticalSync,
        AudioOffset,
        DiscordRichPresence,
        SongPaths,
        BackgroundBeatFlash
    );
    #else
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
        Settings,
        avMainVolume,
        avActiveInstrumentVolume,
        avInactiveInstrumentVolume,
        avSoundEffectVolume,
        avMuteVolume,
        avMenuMusicVolume,
        Fullscreen,
        Framerate,
        VerticalSync,
        AudioOffset,
        DiscordRichPresence,
        BackgroundBeatFlash
    );
    #endif
};

extern Encore::Settings TheGameSettings;

#endif
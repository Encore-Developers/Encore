// Created by marie on 02/10/2024.
//

#ifndef SETTINGS_H
#define SETTINGS_H
#include "GLFW/glfw3.h"
#include <array>
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>

#define SETTINGS_OPTIONS                                                                 \
    OPTION(float, avMainVolume, 0.5f)                                                    \
    OPTION(float, avActiveInstrumentVolume, 0.75f)                                       \
    OPTION(float, avInactiveInstrumentVolume, 0.5f)                                      \
    OPTION(float, avSoundEffectVolume, 0.5f)                                             \
    OPTION(float, avMuteVolume, 0.15f)                                                   \
    OPTION(float, avMenuMusicVolume, 0.15f)                                              \
    OPTION(bool, Fullscreen, false)                                                      \
    OPTION(int, AudioOffset, 0)                                                          \
    OPTION(bool, DiscordRichPresence, true)                                              \
    OPTION(int, Framerate, 60)                                                           \
    OPTION(bool, VerticalSync, true)                                                     \
    OPTION(bool, BackgroundBeatFlash, true)
namespace Encore {
    inline void WriteJsonFile(const std::filesystem::path &FileToWrite, const nlohmann::json &JSONobject) {
        std::ofstream o(FileToWrite, std::ios::out | std::ios::trunc);
        o << JSONobject.dump(2, ' ', false, nlohmann::detail::error_handler_t::strict);
        o.close();
    }
    class Settings {
    public:
#define OPTION(type, value, default) type value = default;
        SETTINGS_OPTIONS
#undef OPTION
        std::vector<std::filesystem::path> SongPaths;
        void SaveToFile(const std::string& filename) const;
        void LoadFromFile(const std::string& filename);
    };

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

    class SettingsInit {
        std::filesystem::path directory;
        void ReadSettings();
        void CreateSettings();
        void MigrateSettings();
        void LegacyMigrateSettings();
    public:
        void InitSettings(std::filesystem::path directory);
    };
}

extern Encore::Settings TheGameSettings;
extern Encore::SettingsInit TheSettingsInitializer;

#endif // SETTINGS_H
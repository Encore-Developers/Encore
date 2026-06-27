//
// Created by marie on 02/10/2024.
//

#include "settings.h"
#include "util/enclog.h"

#include <raylib.h>
#include <fstream>
#include <nlohmann/json.hpp>

namespace Encore {
    void Settings() {
        std::filesystem::path executablePath = GetApplicationDirectory();
        std::filesystem::path directory = executablePath.parent_path();
    }
    void Settings::SaveToFile(const std::string& filename) const {
        try {
            nlohmann::json j = *this;
            std::ofstream file(filename);
            if (!file.is_open()) {
                Log::Error("Failed to open settings file for writing: {}", filename);
                return;
            }
            file << j.dump(4);
            file.close();
            Log::Info("Settings saved to to {}: {}", filename);
        } catch (const std::exception& e) {
            Log::Error("Error saving settings to {}: {}", filename, e.what());
        }
    }

    void Settings::LoadFromFile(const std::string& filename) {
        try {
            std::ifstream file(filename);
            if (!file.is_open()) {
                Log::Warn("Settings file {} not found, using defaults", filename);
                return;
            }
            nlohmann::json j;
            file >> j;
            file.close();
            j.get_to(*this);

            Log::Info("Settings loaded from {}", filename);
        } catch (const std::exception& e) {
            Log::Error("Error loading settings from {}: {}", filename, e.what());
        }
    }
    void Settings::UpdateFullscreen() {
        SDL_SetWindowFullscreen(GetSDLWindow(), Fullscreen);
    }

    // SettingsInit methods
    void SettingsInit::InitSettings(std::filesystem::path directory) {
        this->directory = directory;
        // SettingsOld& settingsMain = SettingsOld::getInstance();
        TheGameSettings.directory = directory;

        /*
         * Do not use old settings for song paths. Again, use new settings.
         */

        // if (exists(directory / "settings-old.json")) {
        //    settingsMain.loadOldSettings(directory / "settings-old.json");
        //    TheGameSettings.SongPaths = settingsMain.songPaths;
        //    TraceLog(LOG_INFO, "Initialized SongPaths from settings-old.json:");
        //    for (const auto& path : TheGameSettings.SongPaths) {
        //        TraceLog(LOG_INFO, "  - %s", path.string().c_str());
        //    }
        // } else {
        //    TheGameSettings.SongPaths = {directory / "Songs"};
        //    TraceLog(LOG_WARNING, "settings-old.json not found in %s, using default SongPaths: %s",
        //             directory.string().c_str(), (directory / "Songs").string().c_str());
        // }
        if (exists(directory / "settings.json")) {
            this->ReadSettings();
            Log::Info("Successfully read settings.json");
        } else {
            this->CreateSettings();
            Log::Info("Created new settings.json");
        }
    }

    void SettingsInit::ReadSettings() {
        nlohmann::json SettingsFile;
        std::ifstream f(directory / "settings.json");
        if (!f.is_open()) {
            Log::Error("Failed to open settings.json for reading in %s", directory.string());
            return;
        }
        SettingsFile = nlohmann::json::parse(f);
        f.close();
        Encore::from_json(SettingsFile, TheGameSettings);
    }

    void SettingsInit::CreateSettings() {
        nlohmann::json SettingsFile = TheGameSettings;
        Encore::WriteJsonFile(directory / "settings.json", SettingsFile);
    }
}
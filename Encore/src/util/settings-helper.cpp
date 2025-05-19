//
// Created by maria on 14/02/2025.
//

#include "settings-helper.h"
#include "settings.h"
#include "json-helper.h"
#include "settings-old.h"
#include "users/playerManager.h"

#include <nlohmann/json_fwd.hpp>

namespace Encore {
    void SettingsInit::InitSettings(std::filesystem::path directory) {

        this->directory = directory;

        TheGameSettings.SongPaths = {directory / "Songs"};

        if (exists(directory / "settings.json")) {
            this->ReadSettings();
            EncoreLog(LOG_INFO, "Successfully read settings");
        } else {
            this->CreateSettings();
            EncoreLog(LOG_INFO, "Created new settings file");
        }

        this->LegacyMigrateSettings();
    }

    void SettingsInit::ReadSettings() {
        nlohmann::json SettingsFile;
        std::ifstream f(directory / "settings.json");
        if (!f.is_open()) {
            EncoreLog(LOG_ERROR, "Failed to open settings.json for reading");
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

    void SettingsInit::LegacyMigrateSettings() {
        SettingsOld& settingsMain = SettingsOld::getInstance();
        settingsMain.setDirectory(directory);
        if (exists(directory / "keybinds.json")) {
            settingsMain.migrateKeybindsToOldSettings(
                directory / "keybinds.json", directory / "settings-old.json");
            EncoreLog(LOG_INFO, "Migrated keybinds.json to settings-old.json");
        }
        if (exists(directory / "settings-old.json")) {
            settingsMain.loadOldSettings(directory / "settings-old.json");
            EncoreLog(LOG_INFO, "Loaded settings-old.json for keybinds and offsets");
        }
    }
    std::filesystem::path directory;
} // Encore
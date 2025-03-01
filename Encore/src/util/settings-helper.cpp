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

        // todo: move to Encore::SettingsHelper
        TheGameSettings.SongPaths = { directory / "Songs" };
        if (exists(directory / "settings.json")
            && !exists(directory / "settings-old.json")) {
            this->MigrateSettings();
            EncoreLog(LOG_INFO, "Moved old settings file");
        }

        // todo: move to own init helper
        if (exists((directory / "settings.json"))) {
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
        SettingsFile = nlohmann::json::parse(f);
        f.close();
        Encore::from_json(SettingsFile, TheGameSettings);
    }
    void SettingsInit::CreateSettings() {
        nlohmann::json SettingsFile = TheGameSettings;
        Encore::WriteJsonFile(directory / "settings.json", SettingsFile);
    }
    void SettingsInit::MigrateSettings() {
        rename(directory / "settings.json", directory / "settings-old.json");
    }
    void SettingsInit::LegacyMigrateSettings() {
        SettingsOld &settingsMain = SettingsOld::getInstance();
        settingsMain.setDirectory(directory);
        if (exists(directory / "keybinds.json")) {
            settingsMain.migrateSettings(
                directory / "keybinds.json", directory / "settings-old.json"
            );
            EncoreLog(LOG_INFO, "Moved old keybinds file");
        }
        settingsMain.loadSettings(directory / "settings-old.json");
        EncoreLog(LOG_INFO, "Loaded old settings file");
    }
} // Encore
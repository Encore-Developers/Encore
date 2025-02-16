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
#ifdef __APPLE__
        CFBundleRef bundle = CFBundleGetMainBundle();
        if (bundle != NULL) {
            // get the Resources directory for our binary for the Assets handling
            CFURLRef resourceURL = CFBundleCopyResourcesDirectoryURL(bundle);
            if (resourceURL != NULL) {
                char resourcePath[PATH_MAX];
                if (CFURLGetFileSystemRepresentation(
                        resourceURL, true, (UInt8 *)resourcePath, PATH_MAX
                    ))
                    assets.setDirectory(resourcePath);
                CFRelease(resourceURL);
            }
            // do the next step manually (settings/config handling)
            // "directory" is our executable directory here, hop up to the external dir
            if (directory.filename().compare("MacOS") == 0)
                directory = directory.parent_path().parent_path().parent_path(); // hops
            // "MacOS",
            // "Contents",
            // "Encore.app"
            // into
            // containing
            // folder

            CFRelease(bundle);
        }
#endif
        // todo: move to Encore::SettingsHelper
        TheGameSettings.SongPaths = { directory / "Songs" };
        if (exists(directory / "settings.json")
            && !exists(directory / "settings-old.json")) {
            this->MigrateSettings();
        }

        // todo: move to own init helper
        if (exists((directory / "settings.json"))) {
            this->ReadSettings();
        } else {
            this->CreateSettings();
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
        }
        settingsMain.loadSettings(directory / "settings-old.json");
    }
} // Encore
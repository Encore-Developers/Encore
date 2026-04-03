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
                TraceLog(LOG_ERROR, "Failed to open settings file for writing: %s", filename.c_str());
                return;
            }
            file << j.dump(4);
            file.close();
            TraceLog(LOG_INFO, "Settings saved to %s", filename.c_str());
        } catch (const std::exception& e) {
            TraceLog(LOG_ERROR, "Error saving settings to %s: %s", filename.c_str(), e.what());
        }
    }

    void Settings::LoadFromFile(const std::string& filename) {
        try {
            std::ifstream file(filename);
            if (!file.is_open()) {
                TraceLog(LOG_WARNING, "Settings file not found, using defaults: %s", filename.c_str());
                return;
            }
            nlohmann::json j;
            file >> j;
            file.close();
            j.get_to(*this);
            TraceLog(LOG_INFO, "Settings loaded from %s", filename.c_str());
        } catch (const std::exception& e) {
            TraceLog(LOG_ERROR, "Error loading settings from %s: %s", filename.c_str(), e.what());
        }
    }
    void Settings::UpdateFullscreen() {
        if (!Fullscreen) {
            const GLFWvidmode *vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            int x, y, width, height;
            glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), &x, &y, &width, &height);
            EncoreLog(LOG_INFO, TextFormat("Workarea of monitor %s: %i %i %i %i", glfwGetMonitorName(glfwGetPrimaryMonitor()), x, y, width, height));
            int windowWidth = width * 0.75;
            int windowHeight = height * 0.75;
            SetWindowPosition(width/2 - windowWidth/2 + x, height/2 - windowHeight/2 + y);
            SetWindowSize(windowWidth, windowHeight);
            glfwSetWindowMonitor(glfwGetCurrentContext(), NULL, width/2 - windowWidth/2 + x, height/2 - windowHeight/2 + y, windowWidth, windowHeight, vidmode->refreshRate);
            ClearWindowState(FLAG_WINDOW_UNDECORATED);
            MaximizeWindow();
        } else {
            auto monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode *vidmode = glfwGetVideoMode(monitor);
            int x, y, width, height;
            glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), &x, &y, &width, &height);

            SetWindowPosition(x, y);
            SetWindowSize(vidmode->width, vidmode->height);
            glfwSetWindowMonitor(glfwGetCurrentContext(), monitor, x, y, vidmode->width, vidmode->height, vidmode->refreshRate);
        }
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
            TraceLog(LOG_INFO, "Successfully read settings.json");
        } else {
            this->CreateSettings();
            TraceLog(LOG_INFO, "Created new settings.json");
        }
    }

    void SettingsInit::ReadSettings() {
        nlohmann::json SettingsFile;
        std::ifstream f(directory / "settings.json");
        if (!f.is_open()) {
            TraceLog(LOG_ERROR, "Failed to open settings.json for reading in %s", directory.string().c_str());
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
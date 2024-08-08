//
// Created by marie on 08/08/2024.
//

#ifndef SETTINGS_H
#define SETTINGS_H
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

#include "raylib.h"
#include "settingObjects.h"

class Settings {
private:
	Settings() {
		// volume
		settings["mainVolume"] = std::make_unique<FloatSetting>(0.5f);
		settings["instVolume"] = std::make_unique<FloatSetting>(0.75f);
		settings["bandVolume"] = std::make_unique<FloatSetting>(0.5f);
		settings["sfxVolume"] = std::make_unique<FloatSetting>(0.5f);
		settings["missVolume"] = std::make_unique<FloatSetting>(0.15f);
		settings["menuVolume"] = std::make_unique<FloatSetting>(0.15f);

		settings["audioOffset"] = std::make_unique<IntSetting>(0);
		settings["videoOffset"] = std::make_unique<IntSetting>(0);
		settings["fullscreen"] = std::make_unique<BoolSetting>(true);
		settings["songPaths"] = std::make_unique<ArraySetting<std::filesystem::path>>(std::vector<std::filesystem::path>{directory / "Songs"});
		// settings go here
	}
	std::filesystem::path executablePath = GetApplicationDirectory();
	std::filesystem::path directory = executablePath.parent_path();

	std::unordered_map<std::string, std::unique_ptr<SettingObject>> settings;

public:
	void setDirectory(std::filesystem::path appConfigDirectory) {
		directory = appConfigDirectory;
		// HACK!! to fix defaultSongPaths being assigned earlier
		// TODO(InvoxiPlayGames): do it proper
		settings["songPaths"] = std::make_unique<ArraySetting<std::filesystem::path>>(std::vector<std::filesystem::path>{directory / "Songs"});
	}

	static Settings& GetInstance() {
		static Settings instance;
		return instance;
	}
	Settings(const Settings&) = delete;
	void operator=(const Settings&) = delete;

	void LoadSettings(const std::filesystem::path& settingsFile);
	void SaveSettings(const std::filesystem::path& settingsFile);

	void setInt(const std::string& key, int value) {
        if (settings.find(key) != settings.end()) {
            IntSetting* intSetting = dynamic_cast<IntSetting*>(settings[key].get());
            if (intSetting) {
                intSetting->SetValue(rapidjson::Value(value));
            } else {
                std::cerr << "Setting is not an IntSetting." << std::endl;
            }
        } else {
            std::cerr << "Setting not found." << std::endl;
        }
    }

    void setFloat(const std::string& key, float value) {
        if (settings.find(key) != settings.end()) {
            FloatSetting* floatSetting = dynamic_cast<FloatSetting*>(settings[key].get());
            if (floatSetting) {
                floatSetting->SetValue(rapidjson::Value(value));
            } else {
                std::cerr << "Setting is not a FloatSetting." << std::endl;
            }
        } else {
            std::cerr << "Setting not found." << std::endl;
        }
    }

    void setBool(const std::string& key, bool value) {
        if (settings.find(key) != settings.end()) {
            BoolSetting* boolSetting = dynamic_cast<BoolSetting*>(settings[key].get());
            if (boolSetting) {
                boolSetting->SetValue(rapidjson::Value(value));
            } else {
                std::cerr << "Setting is not a BoolSetting." << std::endl;
            }
        } else {
            std::cerr << "Setting not found." << std::endl;
        }
    }

    template <typename T>
    void setArray(const std::string& key, const std::vector<T>& value) {
        if (settings.find(key) != settings.end()) {
            ArraySetting<T>* arraySetting = dynamic_cast<ArraySetting<T>*>(settings[key].get());
            if (arraySetting) {
                rapidjson::Value array(rapidjson::kArrayType);
                for (const auto& v : value) {
                    array.PushBack(v, rapidjson::Document().GetAllocator());
                }
                arraySetting->SetValue(array);
            } else {
                std::cerr << "Setting is not an ArraySetting of the correct type." << std::endl;
            }
        } else {
            std::cerr << "Setting not found." << std::endl;
        }
    }
};



#endif //SETTINGS_H

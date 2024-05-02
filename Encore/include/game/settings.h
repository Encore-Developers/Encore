#pragma once
#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include <filesystem>
#include "GLFW/glfw3.h"
#include "raylib.h"
#include <iostream>
#include <fstream>

#ifndef ENCORE_SETTINGS_H
#define ENCORE_SETTINGS_H

class Settings {
private:
	rapidjson::Value vectorToJsonArray(const std::vector<int>& vec, rapidjson::Document::AllocatorType& allocator);
	static void ensureValuesExist();
    std::filesystem::path executablePath = GetApplicationDirectory();
    std::filesystem::path directory = executablePath.parent_path();
public:
	static rapidjson::Document settings;

    static std::vector<int> defaultKeybinds4K;
    static std::vector<int> defaultKeybinds5K;
    static std::vector<int> defaultKeybinds4KAlt;
    static std::vector<int> defaultKeybinds5KAlt;
    static int defaultKeybindOverdrive;
    static int defaultKeybindOverdriveAlt;
    static std::vector<int> defaultController4K;
    static std::vector<int> defaultController5K;
    static std::vector<int> defaultController5KAxisDirection;
    static std::vector<int> defaultController4KAxisDirection;
    static int defaultControllerOverdrive;
    static int defaultControllerOverdriveAxisDirection;
    static int defaultControllerType;
    static std::vector<int> keybinds4K;
    static std::vector<int> keybinds5K;
    static std::vector<int> keybinds4KAlt;
    static std::vector<int> keybinds5KAlt;
    static std::vector<int> controller4K;
    static std::vector<int> controller5K;
    static std::vector<int> controller4KAxisDirection;
    static std::vector<int> controller5KAxisDirection;
    static int controllerType;
    static int keybindOverdrive;
    static int keybindOverdriveAlt;
    static int controllerOverdrive;
    static int controllerOverdriveAxisDirection;
    static std::vector<int> prev4K;
    static std::vector<int> prev5K;
    static std::vector<int> prevController4K;
    static std::vector<int> prevController5K;
    static std::vector<int> prevController4KAxisDirection;
    static std::vector<int> prevController5KAxisDirection;
    static std::vector<int> prev4KAlt;
    static std::vector<int> prev5KAlt;
    static int prevOverdrive;
    static int prevOverdriveAlt;
    static int prevControllerOverdrive;
    static int prevControllerOverdriveAxisDirection;
    static int prevControllerType;
	static std::vector<float> defaultTrackSpeedOptions;
	static std::vector<float> trackSpeedOptions;
    static std::vector<std::filesystem::path> defaultSongPaths;
    static std::vector<std::filesystem::path> songPaths;
    static std::vector<std::filesystem::path> prevSongPaths;
	static int trackSpeed;
    static int prevTrackSpeed;
    static int avOffsetMS;
    static int prevAvOffsetMS;
    static int inputOffsetMS;
    static int prevInputOffsetMS;
    static bool defaultMirrorMode;
    static bool mirrorMode;
    static bool prevMirrorMode;
    static bool changing4k;
    static bool changingAlt;
    static bool missHighwayDefault;
    static bool prevMissHighwayColor;

    static float highwayLengthMult;
    static float prevHighwayLengthMult;

    static void writeDefaultSettings(const std::filesystem::path& settingsFile, bool migrate = false);
    static void loadSettings(const std::filesystem::path& settingsFile);
    static void saveSettings(const std::filesystem::path& settingsFile);
	static void migrateSettings(const std::filesystem::path& keybindsFile, const std::filesystem::path& settingsFile);
};

#endif

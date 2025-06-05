#pragma once // recoded for keyboard and gamepad keybind settings
#include "settings.h"
#include "keybinds.h"
#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/error/en.h"
#include "raylib.h"
#include "GLFW/glfw3.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include "raylib.h"

class SettingsOld {
private:
    static const int KEY_COUNT = 512;
    SettingsOld() {
        std::filesystem::path oldSettingsFile = directory / "settings-old.json";
        std::filesystem::path newSettingsFile = directory / "settings.json";

        if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
            std::cerr << "Error: Invalid directory " << directory << ". Using current directory as fallback." << std::endl;
            directory = std::filesystem::current_path();
            oldSettingsFile = directory / "settings-old.json";
            newSettingsFile = directory / "settings.json";
        }

        try {
            TheGameSettings.LoadFromFile(newSettingsFile.string());
        };

        if (!file.is_open()) {
            TraceLog(LOG_ERROR, "Failed to load %s", newSettingsFile.c_str());
        }
        MainVolume = settings.avMainVolume;
        PlayerVolume = settings.avActiveInstrumentVolume;
        BandVolume = settings.avInactiveInstrumentVolume;
        SFXVolume = settings.avSoundEffectVolume;
        MissVolume = settings.avMuteVolume;
        MenuVolume = settings.avMenuMusicVolume;
        fullscreen = settings.Fullscreen;
        songPaths = settings.SongPaths;

#ifndef PLATFORM_NX
#include <filesystem>
        using PathType = std::filesystem::path;
#else
        using PathType = std::string;
#endif
        std::vector<PathType> songPaths;

        if (std::filesystem::exists(oldSettingsFile)) {
            loadOldSettings(oldSettingsFile);
        } else {
            std::cerr << "Warning: " << oldSettingsFile << " does not exist. Creating with defaults." << std::endl;
            saveOldSettings(oldSettingsFile);
        }

        syncKeybindsToGame();
    }

    Encore::Settings& settings = TheGameSettings;
    std::filesystem::path executablePath = GetApplicationDirectory();
    std::filesystem::path directory = executablePath.parent_path();
    rapidjson::Document tempSettings;

public:
    static SettingsOld& getInstance() {
        static SettingsOld instance;
        return instance;
    }

    SettingsOld(const SettingsOld&) = delete;
    void operator=(const SettingsOld &) = delete;void rebindKey(const std::string & bind_type, int index, int i);

    std::vector<int> defaultKeybinds4K = {KEY_D, KEY_F, KEY_J, KEY_K};
    std::vector<int> defaultKeybinds5K = {KEY_D, KEY_F, KEY_J, KEY_K, KEY_L};
    std::vector<int> defaultKeybinds4KAlt = {-2, -2, -2, -2};
    std::vector<int> defaultKeybinds5KAlt = {-2, -2, -2, -2, -2};
    int defaultKeybindStrumUp = KEY_UP;
    int defaultKeybindStrumDown = KEY_DOWN;
    int defaultKeybindOverdrive = KEY_SPACE;
    int defaultKeybindOverdriveAlt = -2;
    int defaultKeybindPause = KEY_ESCAPE;
    std::vector<int> defaultController4K = {
        GLFW_GAMEPAD_BUTTON_DPAD_LEFT, GLFW_GAMEPAD_BUTTON_DPAD_RIGHT,
        GLFW_GAMEPAD_BUTTON_X, GLFW_GAMEPAD_BUTTON_B};
    std::vector<int> defaultController5K = {
        GLFW_GAMEPAD_BUTTON_DPAD_LEFT, GLFW_GAMEPAD_BUTTON_DPAD_RIGHT,
        GLFW_GAMEPAD_BUTTON_X, GLFW_GAMEPAD_BUTTON_Y, GLFW_GAMEPAD_BUTTON_B};
    std::vector<int> defaultController4KAxisDirection = {0, 0, 0, 0};
    std::vector<int> defaultController5KAxisDirection = {0, 0, 0, 0, 0};
    int defaultControllerOverdrive = -1 - GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER;
    int defaultControllerOverdriveAxisDirection = 1;
    int defaultControllerType = 0;
    int defaultControllerPause = GLFW_GAMEPAD_BUTTON_START;
    int defaultControllerPauseAxisDirection = 0;
    std::vector<float> defaultTrackSpeedOptions = {0.5f, 0.75f, 1.0f, 1.25f, 1.5f, 1.75f, 2.0f};
    bool defaultMirrorMode = false;
    bool missHighwayDefault = false;
    float defaultMainVolume = 0.5f;
    float defaultPlayerVolume = 0.75f;
    float defaultBandVolume = 0.5f;
    float defaultSFXVolume = 0.5f;
    float defaultMissVolume = 0.15f;
    float defaultMenuVolume = 0.15f;
    bool fullscreenDefault = true;

    std::vector<int> keybinds4K = defaultKeybinds4K;
    std::vector<int> keybinds5K = defaultKeybinds5K;
    std::vector<int> keybinds4KAlt = defaultKeybinds4KAlt;
    std::vector<int> keybinds5KAlt = defaultKeybinds5KAlt;
    int keybindStrumUp = defaultKeybindStrumUp;
    int keybindStrumDown = defaultKeybindStrumDown;
    int keybindOverdrive = defaultKeybindOverdrive;
    int keybindOverdriveAlt = defaultKeybindOverdriveAlt;
    int keybindPause = defaultKeybindPause;
    std::vector<int> controller4K = defaultController4K;
    std::vector<int> controller5K = defaultController5K;
    std::vector<int> controller4KAxisDirection = defaultController4KAxisDirection;
    std::vector<int> controller5KAxisDirection = defaultController5KAxisDirection;
    int controllerOverdrive = defaultControllerOverdrive;
    int controllerOverdriveAxisDirection = defaultControllerOverdriveAxisDirection;
    int controllerType = defaultControllerType;
    int controllerPause = defaultControllerPause;
    int controllerPauseAxisDirection = defaultControllerPauseAxisDirection;
    std::vector<int> prev4K = keybinds4K;
    std::vector<int> prev5K = keybinds5K;
    std::vector<int> prev4KAlt = keybinds4KAlt;
    std::vector<int> prev5KAlt = keybinds5KAlt;
    std::vector<int> prevController4K = controller4K;
    std::vector<int> prevController5K = controller5K;
    std::vector<int> prevController4KAxisDirection = controller4KAxisDirection;
    std::vector<int> prevController5KAxisDirection = controller5KAxisDirection;
    int prevKeybindStrumUp = keybindStrumUp;
    int prevKeybindStrumDown = keybindStrumDown;
    int prevOverdrive = keybindOverdrive;
    int prevOverdriveAlt = keybindOverdriveAlt;
    int prevKeybindPause = keybindPause;
    int prevControllerOverdrive = controllerOverdrive;
    int prevControllerOverdriveAxisDirection = controllerOverdriveAxisDirection;
    int prevControllerType = controllerType;
    int prevControllerPause = controllerPause;
    int prevControllerPauseAxisDirection = controllerPauseAxisDirection;
    std::vector<float> trackSpeedOptions = defaultTrackSpeedOptions;
    int trackSpeed = 4;
    int prevTrackSpeed = trackSpeed;
    int avOffsetMS = 0;
    int prevAvOffsetMS = avOffsetMS;
    int inputOffsetMS = 0;
    int prevInputOffsetMS = inputOffsetMS;
    bool mirrorMode = defaultMirrorMode;
    bool prevMirrorMode = mirrorMode;
    bool missHighwayColor = missHighwayDefault;
    bool prevMissHighwayColor = missHighwayColor;
    float highwayLengthMult = 1.0f;
    float prevHighwayLengthMult = highwayLengthMult;
    float MainVolume = defaultMainVolume;
    float PlayerVolume = defaultPlayerVolume;
    float BandVolume = defaultBandVolume;
    float SFXVolume = defaultSFXVolume;
    float MissVolume = defaultMissVolume;
    float MenuVolume = defaultMenuVolume;
    float prevMainVolume = MainVolume;
    float prevPlayerVolume = PlayerVolume;
    float prevBandVolume = BandVolume;
    float prevSFXVolume = SFXVolume;
    float prevMissVolume = MissVolume;
    float prevMenuVolume = MenuVolume;
    bool fullscreen = fullscreenDefault;
    bool fullscreenPrev = fullscreen;
#ifndef PLATFORM_NX
#include <filesystem>
    using PathType = std::filesystem::path;
#else
    using PathType = std::string;
#endif
    std::vector<std::filesystem::path> defaultSongPaths = {directory / "Songs"};
    std::vector<std::filesystem::path> songPaths = defaultSongPaths;
    std::vector<std::filesystem::path> prevSongPaths = songPaths;

    // new rebinding and unbinding. still gotta fix a little bit
    void rebindKey(const std::string& bindType, int index = -1) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            if (bindType == "keybinds4K" && index >= 0 && index < keybinds4K.size()) {
                keybinds4K[index] = -2;
                prev4K[index] = keybinds4K[index];
            } else if (bindType == "keybinds5K" && index >= 0 && index < keybinds5K.size()) {
                keybinds5K[index] = -2;
                prev5K[index] = keybinds5K[index];
            } else if (bindType == "keybinds4KAlt" && index >= 0 && index < keybinds4KAlt.size()) {
                keybinds4KAlt[index] = -2;
                prev4KAlt[index] = keybinds4KAlt[index];
            } else if (bindType == "keybinds5KAlt" && index >= 0 && index < keybinds5KAlt.size()) {
                keybinds5KAlt[index] = -2;
                prev5KAlt[index] = keybinds5KAlt[index];
            } else if (bindType == "keybindStrumUp") {
                keybindStrumUp = -2;
                prevKeybindStrumUp = keybindStrumUp;
            } else if (bindType == "keybindStrumDown") {
                keybindStrumDown = -2;
                prevKeybindStrumDown = keybindStrumDown;
            } else if (bindType == "keybindOverdrive") {
                keybindOverdrive = -2;
                prevOverdrive = keybindOverdrive;
            } else if (bindType == "keybindOverdriveAlt") {
                keybindOverdriveAlt = -2;
                prevOverdriveAlt = keybindOverdriveAlt;
            } else if (bindType == "keybindPause") {
                keybindPause = -2;
                prevKeybindPause = keybindPause;
            } else if (bindType == "controller4K" && index >= 0 && index < controller4K.size()) {
                controller4K[index] = -2;
                prevController4K[index] = controller4K[index];
            } else if (bindType == "controller5K" && index >= 0 && index < controller5K.size()) {
                controller5K[index] = -2;
                prevController5K[index] = controller5K[index];
            } else if (bindType == "controllerOverdrive") {
                controllerOverdrive = -2;
                prevControllerOverdrive = controllerOverdrive;
            } else if (bindType == "controllerPause") {
                controllerPause = -2;
                prevControllerPause = controllerPause;
            } else {
                std::cerr << "Error: Invalid bindType or index for " << bindType << "[" << index << "]" << std::endl;
                return;
            }

            syncKeybindsToGame();
            saveOldSettings(directory / "settings-old.json");
            std::cout << "Info: Keybind " << bindType << (index >= 0 ? "[" + std::to_string(index) + "]" : "")
                      << " unbound via right-click." << std::endl;
            return;
        }

        for (int key = 0; key < KEY_COUNT; ++key) {
            if (IsKeyPressed(key)) {
                if (bindType == "keybinds4K" && index >= 0 && index < keybinds4K.size()) {
                    keybinds4K[index] = key;
                    prev4K[index] = keybinds4K[index];
                } else if (bindType == "keybinds5K" && index >= 0 && index < keybinds5K.size()) {
                    keybinds5K[index] = key;
                    prev5K[index] = keybinds5K[index];
                } else if (bindType == "keybinds4KAlt" && index >= 0 && index < keybinds4KAlt.size()) {
                    keybinds4KAlt[index] = key;
                    prev4KAlt[index] = keybinds4KAlt[index];
                } else if (bindType == "keybinds5KAlt" && index >= 0 && index < keybinds5KAlt.size()) {
                    keybinds5KAlt[index] = key;
                    prev5KAlt[index] = keybinds5KAlt[index];
                } else if (bindType == "keybindStrumUp") {
                    keybindStrumUp = key;
                    prevKeybindStrumUp = keybindStrumUp;
                } else if (bindType == "keybindStrumDown") {
                    keybindStrumDown = key;
                    prevKeybindStrumDown = keybindStrumDown;
                } else if (bindType == "keybindOverdrive") {
                    keybindOverdrive = key;
                    prevOverdrive = keybindOverdrive;
                } else if (bindType == "keybindOverdriveAlt") {
                    keybindOverdriveAlt = key;
                    prevOverdriveAlt = keybindOverdriveAlt;
                } else if (bindType == "keybindPause") {
                    keybindPause = key;
                    prevKeybindPause = keybindPause;
                } else {
                    std::cerr << "Error: Invalid bindType or index for " << bindType << "[" << index << "]" << std::endl;
                    return;
                }

                syncKeybindsToGame();
                saveOldSettings(directory / "settings-old.json");
                std::cout << "Info: Keybind " << bindType << (index >= 0 ? "[" + std::to_string(index) + "]" : "")
                          << " set to " << key << std::endl;
                return;
            }
        }
    }

    void migrateKeybindsToOldSettings(const std::filesystem::path& keybindsFile, const std::filesystem::path& settingsFile) {
        std::ifstream ifs(keybindsFile, std::ios::binary);
        if (!ifs.is_open()) {
            std::cerr << "Error: Failed to open " << keybindsFile << " for migration." << std::endl;
            saveOldSettings(settingsFile);
            return;
        }
        std::string jsonString((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();
        if (jsonString.empty()) {
            std::cerr << "Error: " << keybindsFile << " is empty." << std::endl;
            saveOldSettings(settingsFile);
            return;
        }
        rapidjson::Document keybinds;
        if (keybinds.Parse(jsonString.c_str()).HasParseError()) {
            std::cerr << "Error: Failed to parse " << keybindsFile << ": "
                      << rapidjson::GetParseError_En(keybinds.GetParseError()) << std::endl;
            saveOldSettings(settingsFile);
            return;
        }
        if (!keybinds.IsObject()) {
            std::cerr << "Error: " << keybindsFile << " root is not an object." << std::endl;
            saveOldSettings(settingsFile);
            return;
        }

        bool keybindsError = false;
        bool keybinds4KError = false;
        bool keybinds5KError = false;
        bool avOffsetError = false;
        bool inputOffsetError = false;

        if (keybinds.HasMember("avOffset") && keybinds["avOffset"].IsInt()) {
            avOffsetMS = keybinds["avOffset"].GetInt();
            prevAvOffsetMS = avOffsetMS;
        } else {
            avOffsetError = true;
        }
        if (keybinds.HasMember("inputOffset") && keybinds["inputOffset"].IsInt()) {
            inputOffsetMS = keybinds["inputOffset"].GetInt();
            prevInputOffsetMS = inputOffsetMS;
        } else {
            inputOffsetError = true;
        }
        if (keybinds.HasMember("keybinds") && keybinds["keybinds"].IsObject()) {
            const auto& kb = keybinds["keybinds"];
            if (kb.HasMember("4k") && kb["4k"].IsArray() && kb["4k"].Size() == 4) {
                keybinds4K.clear();
                for (const auto& key : kb["4k"].GetArray()) {
                    if (key.IsInt()) keybinds4K.push_back(key.GetInt());
                    else keybinds4KError = true;
                }
                prev4K = keybinds4K;
            } else {
                keybinds4KError = true;
            }
            if (kb.HasMember("5k") && kb["5k"].IsArray() && kb["5k"].Size() == 5) {
                keybinds5K.clear();
                for (const auto& key : kb["5k"].GetArray()) {
                    if (key.IsInt()) keybinds5K.push_back(key.GetInt());
                    else keybinds5KError = true;
                }
                prev5K = keybinds5K;
            } else {
                keybinds5KError = true;
            }
            if (kb.HasMember("4kAlt") && kb["4kAlt"].IsArray() && kb["4kAlt"].Size() == 4) {
                keybinds4KAlt.clear();
                for (const auto& key : kb["4kAlt"].GetArray()) {
                    if (key.IsInt()) keybinds4KAlt.push_back(key.GetInt());
                    else std::cerr << "Warning: Invalid key in keybinds.4kAlt array in " << keybindsFile << std::endl;
                }
                prev4KAlt = keybinds4KAlt;
            }
            if (kb.HasMember("5kAlt") && kb["5kAlt"].IsArray() && kb["5kAlt"].Size() == 5) {
                keybinds5KAlt.clear();
                for (const auto& key : kb["5kAlt"].GetArray()) {
                    if (key.IsInt()) keybinds5KAlt.push_back(key.GetInt());
                    else std::cerr << "Warning: Invalid key in keybinds.5kAlt array in " << keybindsFile << std::endl;
                }
                prev5KAlt = keybinds5KAlt;
            }
            if (kb.HasMember("overdrive") && kb["overdrive"].IsInt()) {
                keybindOverdrive = kb["overdrive"].GetInt();
                prevOverdrive = keybindOverdrive;
            }
            if (kb.HasMember("overdriveAlt") && kb["overdriveAlt"].IsInt()) {
                keybindOverdriveAlt = kb["overdriveAlt"].GetInt();
                prevOverdriveAlt = keybindOverdriveAlt;
            }
            if (kb.HasMember("pause") && kb["pause"].IsInt()) {
                keybindPause = kb["pause"].GetInt();
                prevKeybindPause = keybindPause;
            }
            if (kb.HasMember("strumUp") && kb["strumUp"].IsInt()) {
                keybindStrumUp = kb["strumUp"].GetInt();
                prevKeybindStrumUp = keybindStrumUp;
            }
            if (kb.HasMember("strumDown") && kb["strumDown"].IsInt()) {
                keybindStrumDown = kb["strumDown"].GetInt();
                prevKeybindStrumDown = keybindStrumDown;
            }
        } else {
            keybindsError = true;
        }
        if (keybinds.HasMember("controllerbinds") && keybinds["controllerbinds"].IsObject()) {
            const auto& cb = keybinds["controllerbinds"];
            if (cb.HasMember("type") && cb["type"].IsInt()) {
                controllerType = cb["type"].GetInt();
                prevControllerType = controllerType;
            }
            if (cb.HasMember("4k") && cb["4k"].IsArray() && cb["4k"].Size() == 4) {
                controller4K.clear();
                for (const auto& key : cb["4k"].GetArray()) {
                    if (key.IsInt()) controller4K.push_back(key.GetInt());
                    else std::cerr << "Warning: Invalid key in controllerbinds.4k array in " << keybindsFile << std::endl;
                }
                prevController4K = controller4K;
            }
            if (cb.HasMember("5k") && cb["5k"].IsArray() && cb["5k"].Size() == 5) {
                controller5K.clear();
                for (const auto& key : cb["5k"].GetArray()) {
                    if (key.IsInt()) controller5K.push_back(key.GetInt());
                    else std::cerr << "Warning: Invalid key in controllerbinds.5k array in " << keybindsFile << std::endl;
                }
                prevController5K = controller5K;
            }
            if (cb.HasMember("4k_direction") && cb["4k_direction"].IsArray() && cb["4k_direction"].Size() == 4) {
                controller4KAxisDirection.clear();
                for (const auto& dir : cb["4k_direction"].GetArray()) {
                    if (dir.IsInt()) controller4KAxisDirection.push_back(dir.GetInt());
                    else std::cerr << "Warning: Invalid direction in controllerbinds.4k_direction array in " << keybindsFile << std::endl;
                }
                prevController4KAxisDirection = controller4KAxisDirection;
            }
            if (cb.HasMember("5k_direction") && cb["5k_direction"].IsArray() && cb["5k_direction"].Size() == 5) {
                controller5KAxisDirection.clear();
                for (const auto& dir : cb["5k_direction"].GetArray()) {
                    if (dir.IsInt()) controller5KAxisDirection.push_back(dir.GetInt());
                    else std::cerr << "Warning: Invalid direction in controllerbinds.5k_direction array in " << keybindsFile << std::endl;
                }
                prevController5KAxisDirection = controller5KAxisDirection;
            }
            if (cb.HasMember("pause") && cb["pause"].IsInt()) {
                controllerPause = cb["pause"].GetInt();
                prevControllerPause = controllerPause;
            }
            if (cb.HasMember("pause_direction") && cb["pause_direction"].IsInt()) {
                controllerPauseAxisDirection = cb["pause_direction"].GetInt();
                prevControllerPauseAxisDirection = controllerPauseAxisDirection;
            }
            if (cb.HasMember("overdrive") && cb["overdrive"].IsInt()) {
                controllerOverdrive = cb["overdrive"].GetInt();
                prevControllerOverdrive = controllerOverdrive;
            }
            if (cb.HasMember("overdrive_direction") && cb["overdrive_direction"].IsInt()) {
                controllerOverdriveAxisDirection = cb["overdrive_direction"].GetInt();
                prevControllerOverdriveAxisDirection = controllerOverdriveAxisDirection;
            }
        }
        if (keybinds.HasMember("mirror") && keybinds["mirror"].IsBool()) {
            mirrorMode = keybinds["mirror"].GetBool();
            prevMirrorMode = mirrorMode;
        }
        if (keybinds.HasMember("trackSpeed") && keybinds["trackSpeed"].IsInt()) {
            trackSpeed = keybinds["trackSpeed"].GetInt();
            prevTrackSpeed = trackSpeed;
        }
        if (keybinds.HasMember("length") && keybinds["length"].IsFloat()) {
            highwayLengthMult = keybinds["length"].GetFloat();
            prevHighwayLengthMult = highwayLengthMult;
        }
        if (keybinds.HasMember("trackSpeedOptions") && keybinds["trackSpeedOptions"].IsArray()) {
            trackSpeedOptions.clear();
            for (const auto& opt : keybinds["trackSpeedOptions"].GetArray()) {
                if (opt.IsFloat()) trackSpeedOptions.push_back(opt.GetFloat());
                else std::cerr << "Warning: Invalid value in trackSpeedOptions array in " << keybindsFile << std::endl;
            }
        }
        if (keybinds.HasMember("missHighwayColor") && keybinds["missHighwayColor"].IsBool()) {
            missHighwayColor = keybinds["missHighwayColor"].GetBool();
            prevMissHighwayColor = missHighwayColor;
        }

        if (keybindsError || keybinds4KError || keybinds5KError || avOffsetError || inputOffsetError) {
            if (keybinds4KError) keybinds4K = defaultKeybinds4K;
            if (keybinds5KError) keybinds5K = defaultKeybinds5K;
            if (avOffsetError) avOffsetMS = 0;
            if (inputOffsetError) inputOffsetMS = 0;
        }

        saveOldSettings(settingsFile);
        syncKeybindsToGame();
        std::filesystem::remove(keybindsFile);
    }

    void loadOldSettings(const std::filesystem::path& oldFile) {
        std::ifstream ifs(oldFile, std::ios::binary);
        if (!ifs.is_open()) {
            std::cerr << "Error: Failed to open " << oldFile << " for loading." << std::endl;
            return;
        }
        std::string jsonString((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();
        if (jsonString.empty()) {
            std::cerr << "Error: " << oldFile << " is empty." << std::endl;
            return;
        }
        rapidjson::Document oldSettings;
        if (oldSettings.Parse(jsonString.c_str()).HasParseError()) {
            std::cerr << "Error: Failed to parse " << oldFile << ": "
                      << rapidjson::GetParseError_En(oldSettings.GetParseError()) << std::endl;
            return;
        }
        if (!oldSettings.IsObject()) {
            std::cerr << "Error: " << oldFile << " root is not an object." << std::endl;
            return;
        }

        bool keybindsError = false;
        bool keybinds4KError = false;
        bool keybinds5KError = false;
        bool keybinds4KAltError = false;
        bool keybinds5KAltError = false;
        bool keybindsStrumUpError = false;
        bool keybindsStrumDownError = false;
        bool keybindsOverdriveError = false;
        bool keybindsOverdriveAltError = false;
        bool keybindsPauseError = false;
        bool controllerError = false;
        bool controllerTypeError = false;
        bool controller4KError = false;
        bool controller4KDirectionError = false;
        bool controller5KError = false;
        bool controller5KDirectionError = false;
        bool controllerOverdriveError = false;
        bool controllerOverdriveDirectionError = false;
        bool controllerPauseError = false;
        bool controllerPauseDirectionError = false;
        bool avError = false;
        bool inputError = false;
        bool mirrorError = false;
        bool trackSpeedOptionsError = false;
        bool highwayLengthError = false;
        bool trackSpeedError = false;
        bool missHighwayError = false;

        if (oldSettings.HasMember("avOffset") && oldSettings["avOffset"].IsInt()) {
            avOffsetMS = oldSettings["avOffset"].GetInt();
            prevAvOffsetMS = avOffsetMS;
        } else {
            avError = true;
        }
        if (oldSettings.HasMember("inputOffset") && oldSettings["inputOffset"].IsInt()) {
            inputOffsetMS = oldSettings["inputOffset"].GetInt();
            prevInputOffsetMS = inputOffsetMS;
        } else {
            inputError = true;
        }
        if (oldSettings.HasMember("keybinds") && oldSettings["keybinds"].IsObject()) {
            const auto& kb = oldSettings["keybinds"];
            if (kb.HasMember("4k") && kb["4k"].IsArray() && kb["4k"].Size() == 4) {
                keybinds4K.clear();
                for (const auto& key : kb["4k"].GetArray()) {
                    if (key.IsInt()) keybinds4K.push_back(key.GetInt());
                    else keybinds4KError = true;
                }
                prev4K = keybinds4K;
            } else {
                keybinds4KError = true;
            }
            if (kb.HasMember("5k") && kb["5k"].IsArray() && kb["5k"].Size() == 5) {
                keybinds5K.clear();
                for (const auto& key : kb["5k"].GetArray()) {
                    if (key.IsInt()) keybinds5K.push_back(key.GetInt());
                    else keybinds5KError = true;
                }
                prev5K = keybinds5K;
            } else {
                keybinds5KError = true;
            }
            if (kb.HasMember("4kAlt") && kb["4kAlt"].IsArray() && kb["4kAlt"].Size() == 4) {
                keybinds4KAlt.clear();
                for (const auto& key : kb["4kAlt"].GetArray()) {
                    if (key.IsInt()) keybinds4KAlt.push_back(key.GetInt());
                    else keybinds4KAltError = true;
                }
                prev4KAlt = keybinds4KAlt;
            } else {
                keybinds4KAltError = true;
            }
            if (kb.HasMember("5kAlt") && kb["5kAlt"].IsArray() && kb["5kAlt"].Size() == 5) {
                keybinds5KAlt.clear();
                for (const auto& key : kb["5kAlt"].GetArray()) {
                    if (key.IsInt()) keybinds5KAlt.push_back(key.GetInt());
                    else keybinds5KAltError = true;
                }
                prev5KAlt = keybinds5KAlt;
            } else {
                keybinds5KAltError = true;
            }
            if (kb.HasMember("overdrive") && kb["overdrive"].IsInt()) {
                keybindOverdrive = kb["overdrive"].GetInt();
                prevOverdrive = keybindOverdrive;
            } else {
                keybindsOverdriveError = true;
            }
            if (kb.HasMember("overdriveAlt") && kb["overdriveAlt"].IsInt()) {
                keybindOverdriveAlt = kb["overdriveAlt"].GetInt();
                prevOverdriveAlt = keybindOverdriveAlt;
            } else {
                keybindsOverdriveAltError = true;
            }
            if (kb.HasMember("pause") && kb["pause"].IsInt()) {
                keybindPause = kb["pause"].GetInt();
                prevKeybindPause = keybindPause;
            } else {
                keybindsPauseError = true;
            }
            if (kb.HasMember("strumUp") && kb["strumUp"].IsInt()) {
                keybindStrumUp = kb["strumUp"].GetInt();
                prevKeybindStrumUp = keybindStrumUp;
            } else {
                keybindsStrumUpError = true;
            }
            if (kb.HasMember("strumDown") && kb["strumDown"].IsInt()) {
                keybindStrumDown = kb["strumDown"].GetInt();
                prevKeybindStrumDown = keybindStrumDown;
            } else {
                keybindsStrumDownError = true;
            }
        } else {
            keybindsError = true;
        }
        if (oldSettings.HasMember("controllerbinds") && oldSettings["controllerbinds"].IsObject()) {
            const auto& cb = oldSettings["controllerbinds"];
            if (cb.HasMember("type") && cb["type"].IsInt()) {
                controllerType = cb["type"].GetInt();
                prevControllerType = controllerType;
            } else {
                controllerTypeError = true;
            }
            if (cb.HasMember("4k") && cb["4k"].IsArray() && cb["4k"].Size() == 4) {
                controller4K.clear();
                for (const auto& key : cb["4k"].GetArray()) {
                    if (key.IsInt()) controller4K.push_back(key.GetInt());
                    else controller4KError = true;
                }
                prevController4K = controller4K;
            } else {
                controller4KError = true;
            }
            if (cb.HasMember("5k") && cb["5k"].IsArray() && cb["5k"].Size() == 5) {
                controller5K.clear();
                for (const auto& key : cb["5k"].GetArray()) {
                    if (key.IsInt()) controller5K.push_back(key.GetInt());
                    else controller5KError = true;
                }
                prevController5K = controller5K;
            } else {
                controller5KError = true;
            }
            if (cb.HasMember("4k_direction") && cb["4k_direction"].IsArray() && cb["4k_direction"].Size() == 4) {
                controller4KAxisDirection.clear();
                for (const auto& dir : cb["4k_direction"].GetArray()) {
                    if (dir.IsInt()) controller4KAxisDirection.push_back(dir.GetInt());
                    else controller4KDirectionError = true;
                }
                prevController4KAxisDirection = controller4KAxisDirection;
            } else {
                controller4KDirectionError = true;
            }
            if (cb.HasMember("5k_direction") && cb["5k_direction"].IsArray() && cb["5k_direction"].Size() == 5) {
                controller5KAxisDirection.clear();
                for (const auto& dir : cb["5k_direction"].GetArray()) {
                    if (dir.IsInt()) controller5KAxisDirection.push_back(dir.GetInt());
                    else controller5KDirectionError = true;
                }
                prevController5KAxisDirection = controller5KAxisDirection;
            } else {
                controller5KDirectionError = true;
            }
            if (cb.HasMember("pause") && cb["pause"].IsInt()) {
                controllerPause = cb["pause"].GetInt();
                prevControllerPause = controllerPause;
            } else {
                controllerPauseError = true;
            }
            if (cb.HasMember("pause_direction") && cb["pause_direction"].IsInt()) {
                controllerPauseAxisDirection = cb["pause_direction"].GetInt();
                prevControllerPauseAxisDirection = controllerPauseAxisDirection;
            } else {
                controllerPauseDirectionError = true;
            }
            if (cb.HasMember("overdrive") && cb["overdrive"].IsInt()) {
                controllerOverdrive = cb["overdrive"].GetInt();
                prevControllerOverdrive = controllerOverdrive;
            } else {
                controllerOverdriveError = true;
            }
            if (cb.HasMember("overdrive_direction") && cb["overdrive_direction"].IsInt()) {
                controllerOverdriveAxisDirection = cb["overdrive_direction"].GetInt();
                prevControllerOverdriveAxisDirection = controllerOverdriveAxisDirection;
            } else {
                controllerOverdriveDirectionError = true;
            }
        } else {
            controllerError = true;
        }
        if (oldSettings.HasMember("mirror") && oldSettings["mirror"].IsBool()) {
            mirrorMode = oldSettings["mirror"].GetBool();
            prevMirrorMode = mirrorMode;
        } else {
            mirrorError = true;
        }
        if (oldSettings.HasMember("trackSpeed") && oldSettings["trackSpeed"].IsInt()) {
            trackSpeed = oldSettings["trackSpeed"].GetInt();
            prevTrackSpeed = trackSpeed;
        } else {
            trackSpeedError = true;
        }
        if (oldSettings.HasMember("length") && oldSettings["length"].IsFloat()) {
            highwayLengthMult = oldSettings["length"].GetFloat();
            prevHighwayLengthMult = highwayLengthMult;
        } else {
            highwayLengthError = true;
        }
        if (oldSettings.HasMember("trackSpeedOptions") && oldSettings["trackSpeedOptions"].IsArray()) {
            trackSpeedOptions.clear();
            for (const auto& opt : oldSettings["trackSpeedOptions"].GetArray()) {
                if (opt.IsFloat()) trackSpeedOptions.push_back(opt.GetFloat());
                else trackSpeedOptionsError = true;
            }
        } else {
            trackSpeedOptionsError = true;
        }
        if (oldSettings.HasMember("missHighwayColor") && oldSettings["missHighwayColor"].IsBool()) {
            missHighwayColor = oldSettings["missHighwayColor"].GetBool();
            prevMissHighwayColor = missHighwayColor;
        } else {
            missHighwayError = true;
        }

        if (keybindsError || keybinds4KError || keybinds5KError || keybinds4KAltError ||
            keybinds5KAltError || keybindsStrumUpError || keybindsStrumDownError ||
            keybindsOverdriveError || keybindsOverdriveAltError || keybindsPauseError ||
            controllerError || controllerTypeError || controller4KError || controller5KError ||
            controller4KDirectionError || controller5KDirectionError || controllerOverdriveError ||
            controllerOverdriveDirectionError || controllerPauseError || controllerPauseDirectionError ||
            avError || inputError || mirrorError || trackSpeedError || trackSpeedOptionsError ||
            highwayLengthError || missHighwayError) {
            if (keybinds4KError) keybinds4K = defaultKeybinds4K;
            if (keybinds5KError) keybinds5K = defaultKeybinds5K;
            if (keybinds4KAltError) keybinds4KAlt = defaultKeybinds4KAlt;
            if (keybinds5KAltError) keybinds5KAlt = defaultKeybinds5KAlt;
            if (keybindsStrumUpError) keybindStrumUp = defaultKeybindStrumUp;
            if (keybindsStrumDownError) keybindStrumDown = defaultKeybindStrumDown;
            if (keybindsOverdriveError) keybindOverdrive = defaultKeybindOverdrive;
            if (keybindsOverdriveAltError) keybindOverdriveAlt = defaultKeybindOverdriveAlt;
            if (keybindsPauseError) keybindPause = defaultKeybindPause;
            if (controllerTypeError) controllerType = defaultControllerType;
            if (controller4KError) controller4K = defaultController4K;
            if (controller5KError) controller5K = defaultController5K;
            if (controller4KDirectionError) controller4KAxisDirection = defaultController4KAxisDirection;
            if (controller5KDirectionError) controller5KAxisDirection = defaultController5KAxisDirection;
            if (controllerOverdriveError) controllerOverdrive = defaultControllerOverdrive;
            if (controllerOverdriveDirectionError) controllerOverdriveAxisDirection = defaultControllerOverdriveAxisDirection;
            if (controllerPauseError) controllerPause = defaultControllerPause;
            if (controllerPauseDirectionError) controllerPauseAxisDirection = defaultControllerPauseAxisDirection;
            if (avError) avOffsetMS = 0;
            if (inputError) inputOffsetMS = 0;
            if (mirrorError) mirrorMode = defaultMirrorMode;
            if (trackSpeedError) trackSpeed = 4;
            if (trackSpeedOptionsError) trackSpeedOptions = defaultTrackSpeedOptions;
            if (highwayLengthError) highwayLengthMult = 1.0f;
            if (missHighwayError) missHighwayColor = missHighwayDefault;

            prev4K = keybinds4K;
            prev5K = keybinds5K;
            prev4KAlt = keybinds4KAlt;
            prev5KAlt = keybinds5KAlt;
            prevKeybindStrumUp = keybindStrumUp;
            prevKeybindStrumDown = keybindStrumDown;
            prevOverdrive = keybindOverdrive;
            prevOverdriveAlt = keybindOverdriveAlt;
            prevKeybindPause = keybindPause;
            prevController4K = controller4K;
            prevController5K = controller5K;
            prevController4KAxisDirection = controller4KAxisDirection;
            prevController5KAxisDirection = controller5KAxisDirection;
            prevControllerType = controllerType;
            prevControllerOverdrive = controllerOverdrive;
            prevControllerOverdriveAxisDirection = controllerOverdriveAxisDirection;
            prevControllerPause = controllerPause;
            prevControllerPauseAxisDirection = controllerPauseAxisDirection;
            prevAvOffsetMS = avOffsetMS;
            prevInputOffsetMS = inputOffsetMS;
            prevMirrorMode = mirrorMode;
            prevTrackSpeed = trackSpeed;
            prevHighwayLengthMult = highwayLengthMult;
            prevMissHighwayColor = missHighwayColor;

            saveOldSettings(oldFile);
            }

        syncKeybindsToGame();
    }

    void syncToSettings() {
        settings.avMainVolume = MainVolume;
        settings.avActiveInstrumentVolume = PlayerVolume;
        settings.avInactiveInstrumentVolume = BandVolume;
        settings.avSoundEffectVolume = SFXVolume;
        settings.avMuteVolume = MissVolume;
        settings.avMenuMusicVolume = MenuVolume;
        settings.Fullscreen = fullscreen;
        settings.SongPaths = songPaths;
        settings.AudioOffset = avOffsetMS; // Sync avOffsetMS to AudioOffset
    }

    void syncKeybindsToGame() {
        std::cout << "Info: Syncing keybinds to game input system." << std::endl;
        std::cout << "  4K: [" << keybinds4K[0] << "," << keybinds4K[1] << ","
                  << keybinds4K[2] << "," << keybinds4K[3] << "]" << std::endl;
        std::cout << "  5K: [" << keybinds5K[0] << "," << keybinds5K[1] << ","
                  << keybinds5K[2] << "," << keybinds5K[3] << "," << keybinds5K[4] << "]" << std::endl;
        std::cout << "  Overdrive: " << keybindOverdrive << ", Pause: " << keybindPause << std::endl;
        std::cout << "  Controller 4K: [" << controller4K[0] << "," << controller4K[1] << ","
                  << controller4K[2] << "," << controller4K[3] << "]" << std::endl;
    };
{
    void setDirectory(std::filesystem::path appConfigDirectory) {
        directory = appConfigDirectory;
        defaultSongPaths = {directory / "Songs"};
        songPaths = defaultSongPaths;
        prevSongPaths = songPaths;
        settings.SongPaths = songPaths;
        saveOldSettings(directory / "settings-old.json");
    }

    std::filesystem::path getDirectory() {
        return directory;
    };
{
    void loadSettings(std::filesystem::path settingsFile) {
        try {
            settings.LoadFromFile(settingsFile.string());
            if (!file.is_open()) {
                TraceLog(LOG_ERROR, "Failed to load %s", settingsFile.c_str());
            }
            MainVolume = settings.avMainVolume;
            PlayerVolume = settings.avActiveInstrumentVolume;
            BandVolume = settings.avInactiveInstrumentVolume;
            SFXVolume = settings.avSoundEffectVolume;
            MissVolume = settings.avMuteVolume;
            MenuVolume = settings.avMenuMusicVolume;
            fullscreen = settings.Fullscreen;
            songPaths = settings.SongPaths;
            prevMainVolume = MainVolume;
            prevPlayerVolume = PlayerVolume;
            prevBandVolume = BandVolume;
            prevSFXVolume = SFXVolume;
            prevMissVolume = MissVolume;
            prevMenuVolume = MenuVolume;
            prevSongPaths = songPaths;
            prevAvOffsetMS = avOffsetMS;
            prevInputOffsetMS = inputOffsetMS;
            syncKeybindsToGame();
        };

        void saveSettings(std::filesystem::path settingsFile) {
            syncToSettings();
            settings.SaveToFile(settingsFile.string());
        }

        void saveOldSettings(std::filesystem::path oldSettingsFile) {
            tempSettings.SetObject();
            rapidjson::Document::AllocatorType& allocator = tempSettings.GetAllocator();
#ifndef PLATFORM_NX
            songPaths = settings.SongPaths;
            settings.SongPaths = songPaths;
#else
            songPaths.clear();
            for (const auto& path : settings.SongPaths) {
                songPaths.push_back(path);
            }
            settings.SongPaths.clear();
            for (const auto& path : songPaths) {
                settings.SongPaths.push_back(path);
            }
#endif
            rapidjson::Value array4K(rapidjson::kArrayType);
            for (int key : keybinds4K) array4K.PushBack(key, allocator);
            rapidjson::Value array5K(rapidjson::kArrayType);
            for (int key : keybinds5K) array5K.PushBack(key, allocator);
            rapidjson::Value array4KAlt(rapidjson::kArrayType);
            for (int key : keybinds4KAlt) array4KAlt.PushBack(key, allocator);
            rapidjson::Value array5KAlt(rapidjson::kArrayType);
            for (int key : keybinds5KAlt) array5KAlt.PushBack(key, allocator);
            tempSettings.AddMember("keybinds", rapidjson::kObjectType, allocator);
            tempSettings["keybinds"].AddMember("4k", array4K, allocator);
            tempSettings["keybinds"].AddMember("5k", array5K, allocator);
            tempSettings["keybinds"].AddMember("4kAlt", array4KAlt, allocator);
            tempSettings["keybinds"].AddMember("5kAlt", array5KAlt, allocator);
            tempSettings["keybinds"].AddMember("overdrive", keybindOverdrive, allocator);
            tempSettings["keybinds"].AddMember("overdriveAlt", keybindOverdriveAlt, allocator);
            tempSettings["keybinds"].AddMember("pause", keybindPause, allocator);
            tempSettings["keybinds"].AddMember("strumUp", keybindStrumUp, allocator);
            tempSettings["keybinds"].AddMember("strumDown", keybindStrumDown, allocator);

            rapidjson::Value arrayController4K(rapidjson::kArrayType);
            for (int key : controller4K) arrayController4K.PushBack(key, allocator);
            rapidjson::Value arrayController5K(rapidjson::kArrayType);
            for (int key : controller5K) arrayController5K.PushBack(key, allocator);
            rapidjson::Value arrayController4KAxis(rapidjson::kArrayType);
            for (int dir : controller4KAxisDirection) arrayController4KAxis.PushBack(dir, allocator);
            rapidjson::Value arrayController5KAxis(rapidjson::kArrayType);
            for (int dir : controller5KAxisDirection) arrayController5KAxis.PushBack(dir, allocator);
            tempSettings.AddMember("controllerbinds", rapidjson::kObjectType, allocator);
            tempSettings["controllerbinds"].AddMember("type", controllerType, allocator);
            tempSettings["controllerbinds"].AddMember("4k", arrayController4K, allocator);
            tempSettings["controllerbinds"].AddMember("5k", arrayController5K, allocator);
            tempSettings["controllerbinds"].AddMember("4k_direction", arrayController4KAxis, allocator);
            tempSettings["controllerbinds"].AddMember("5k_direction", arrayController5KAxis, allocator);
            tempSettings["controllerbinds"].AddMember("pause", controllerPause, allocator);
            tempSettings["controllerbinds"].AddMember("pause_direction", controllerPauseAxisDirection, allocator);
            tempSettings["controllerbinds"].AddMember("overdrive", controllerOverdrive, allocator);
            tempSettings["controllerbinds"].AddMember("overdrive_direction", controllerOverdriveAxisDirection, allocator);

            tempSettings.AddMember("avOffset", avOffsetMS, allocator);
            tempSettings.AddMember("inputOffset", inputOffsetMS, allocator);
            tempSettings.AddMember("mirror", mirrorMode, allocator);
            tempSettings.AddMember("trackSpeed", trackSpeed, allocator);
            tempSettings.AddMember("length", highwayLengthMult, allocator);
            rapidjson::Value arrayTrackSpeedOptions(rapidjson::kArrayType);
            for (float opt : trackSpeedOptions) arrayTrackSpeedOptions.PushBack(opt, allocator);
            tempSettings.AddMember("trackSpeedOptions", arrayTrackSpeedOptions, allocator);
            tempSettings.AddMember("missHighwayColor", missHighwayColor, allocator);

            std::filesystem::create_directories(oldSettingsFile.parent_path());
            char writeBuffer[8192];
            FILE* fp = fopen(oldSettingsFile.string().c_str(), "wb");
            if (!fp) {
                std::cerr << "Error: Failed to open " << oldSettingsFile << " for writing. Check permissions." << std::endl;
                return;
            }
            rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
            rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
            tempSettings.Accept(writer);
            fclose(fp);
            std::cout << "Info: Successfully saved " << oldSettingsFile << std::endl;
        }
    }
};
#pragma once
#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include "GLFW/glfw3.h"
#include "keybinds.h"
#include "raylib.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include "game/player.h"

class Settings {
private:
	rapidjson::Value vectorToJsonArray(const std::vector<int>& vec, rapidjson::Document::AllocatorType& allocator) {
		rapidjson::Value array(rapidjson::kArrayType);
		for (const auto& value : vec) {
			array.PushBack(value, allocator);
		}
		return array;
	}
	void ensureValuesExist() {
		rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
		if (!settings.HasMember("trackSpeed"))
			settings.AddMember("trackSpeed", rapidjson::Value(), allocator);
		if (!settings.HasMember("avOffset"))
			settings.AddMember("avOffset", rapidjson::Value(), allocator);
		if (!settings.HasMember("inputOffset"))
			settings.AddMember("inputOffset", rapidjson::Value(), allocator);
        if (!settings.HasMember("length"))
            settings.AddMember("length", rapidjson::Value(), allocator);
		if (!settings.HasMember("mirror"))
			settings.AddMember("mirror", rapidjson::Value(), allocator);
		if (!settings.HasMember("keybinds"))
			settings.AddMember("keybinds", rapidjson::kObjectType, allocator);
		if (!settings["keybinds"].HasMember("4k"))
			settings["keybinds"].AddMember("4k", rapidjson::kArrayType, allocator);
		if (!settings["keybinds"].HasMember("5k"))
			settings["keybinds"].AddMember("5k", rapidjson::kArrayType, allocator);
		if (!settings["keybinds"].HasMember("4kAlt"))
			settings["keybinds"].AddMember("4kAlt", rapidjson::kArrayType, allocator);
		if (!settings["keybinds"].HasMember("5kAlt"))
			settings["keybinds"].AddMember("5kAlt", rapidjson::kArrayType, allocator);
		if (!settings["keybinds"].HasMember("overdrive"))
			settings["keybinds"].AddMember("overdrive", rapidjson::Value(), allocator);
		if (!settings["keybinds"].HasMember("overdriveAlt"))
			settings["keybinds"].AddMember("overdriveAlt", rapidjson::Value(), allocator);
		if (!settings.HasMember("controllerbinds"))
			settings.AddMember("controllerbinds", rapidjson::kObjectType, allocator);
		if (!settings["controllerbinds"].HasMember("4k"))
			settings["controllerbinds"].AddMember("4k", rapidjson::kArrayType, allocator);
		if (!settings["controllerbinds"].HasMember("5k"))
			settings["controllerbinds"].AddMember("5k", rapidjson::kArrayType, allocator);
		if (!settings["controllerbinds"].HasMember("overdrive"))
			settings["controllerbinds"].AddMember("overdrive", rapidjson::Value(), allocator);
		if (!settings["controllerbinds"].HasMember("4k_direction"))
			settings["controllerbinds"].AddMember("4k_direction", rapidjson::kArrayType, allocator);
		if (!settings["controllerbinds"].HasMember("5k_direction"))
			settings["controllerbinds"].AddMember("5k_direction", rapidjson::kArrayType, allocator);
		if (!settings["controllerbinds"].HasMember("overdrive"))
			settings["controllerbinds"].AddMember("overdrive", rapidjson::Value(), allocator);
		if (!settings["controllerbinds"].HasMember("type"))
			settings["controllerbinds"].AddMember("type", rapidjson::Value(), allocator);
        if (!settings.HasMember("songDirectories"))
            settings.AddMember("songDirectories", rapidjson::Value(), allocator);
	}
    std::filesystem::path executablePath = GetApplicationDirectory();
    std::filesystem::path directory = executablePath.parent_path();
public:
	rapidjson::Document settings;

	std::vector<int> defaultKeybinds4K{ KEY_D,KEY_F,KEY_J,KEY_K };
	std::vector<int> defaultKeybinds5K{ KEY_D,KEY_F,KEY_J,KEY_K,KEY_L };
	std::vector<int> defaultKeybinds4KAlt{ -1,-1,-1,-1 };
	std::vector<int> defaultKeybinds5KAlt{ -1,-1,-1,-1,-1 };
	int defaultKeybindOverdrive = KEY_SPACE;
	int defaultKeybindOverdriveAlt = -1;
	std::vector<int> defaultController4K{ GLFW_GAMEPAD_BUTTON_DPAD_LEFT,GLFW_GAMEPAD_BUTTON_DPAD_RIGHT,GLFW_GAMEPAD_BUTTON_X,GLFW_GAMEPAD_BUTTON_B };
	std::vector<int> defaultController5K{ GLFW_GAMEPAD_BUTTON_DPAD_LEFT,GLFW_GAMEPAD_BUTTON_DPAD_RIGHT,GLFW_GAMEPAD_BUTTON_X,GLFW_GAMEPAD_BUTTON_Y, GLFW_GAMEPAD_BUTTON_B };
	std::vector<int> defaultController5KAxisDirection{ 0,0,0,0,0 };
	std::vector<int> defaultController4KAxisDirection{ 0,0,0,0 };
	int defaultControllerOverdrive = -1-GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER;
	int defaultControllerOverdriveAxisDirection = 1;
	int defaultControllerType = 0;
	std::vector<int> keybinds4K = defaultKeybinds4K;
	std::vector<int> keybinds5K = defaultKeybinds5K;
	std::vector<int> keybinds4KAlt = defaultKeybinds4KAlt;
	std::vector<int> keybinds5KAlt = defaultKeybinds5KAlt;
	std::vector<int> controller4K = defaultController4K;
	std::vector<int> controller5K = defaultController5K;
	std::vector<int> controller4KAxisDirection = defaultController4KAxisDirection;
	std::vector<int> controller5KAxisDirection = defaultController5KAxisDirection;
	int controllerType = defaultControllerType;
	int keybindOverdrive = defaultKeybindOverdrive;
	int keybindOverdriveAlt = defaultKeybindOverdriveAlt;
	int controllerOverdrive = defaultControllerOverdrive;
	int controllerOverdriveAxisDirection = defaultControllerOverdriveAxisDirection;
	std::vector<int> prev4K = keybinds4K;
	std::vector<int> prev5K = keybinds5K;
	std::vector<int> prevController4K = controller4K;
	std::vector<int> prevController5K = controller5K;
	std::vector<int> prevController4KAxisDirection = controller4KAxisDirection;
	std::vector<int> prevController5KAxisDirection = controller5KAxisDirection;
	std::vector<int> prev4KAlt = keybinds4KAlt;
	std::vector<int> prev5KAlt = keybinds5KAlt;
	int prevOverdrive = keybindOverdrive;
	int prevOverdriveAlt = keybindOverdriveAlt;
	int prevControllerOverdrive = controllerOverdrive;
	int prevControllerOverdriveAxisDirection = controllerOverdriveAxisDirection;
	int prevControllerType = controllerType;
	std::vector<float> defaultTrackSpeedOptions = { 0.5f,0.75f,1.0f,1.25f,1.5f,1.75f,2.0f };
	std::vector<float> trackSpeedOptions = defaultTrackSpeedOptions;
    std::vector<std::filesystem::path> defaultSongPaths{ directory  / "Songs" };
    std::vector<std::filesystem::path> songPaths = defaultSongPaths;
    std::vector<std::filesystem::path> prevSongPaths = songPaths;
	int trackSpeed = 4;
	int prevTrackSpeed = trackSpeed;
	int avOffsetMS = 0;
	int prevAvOffsetMS = avOffsetMS;
	int inputOffsetMS = 0;
	int prevInputOffsetMS = inputOffsetMS;
	bool defaultMirrorMode = false;
	bool mirrorMode = defaultMirrorMode;
	bool prevMirrorMode = mirrorMode;
	bool changing4k = false;
	bool changingAlt = false;
    bool missHighwayDefault = false;
    bool prevMissHighwayColor = missHighwayDefault;

    float highwayLengthMult = 1.0f;
    float prevHighwayLengthMult = highwayLengthMult;

	void writeDefaultSettings(std::filesystem::path settingsFile, bool migrate = false) {
		rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
		settings.SetObject();
		rapidjson::Value array4K(rapidjson::kArrayType);
		rapidjson::Value array5K(rapidjson::kArrayType);
		if (migrate) {
			for (int& key : keybinds4K)
				array4K.PushBack(rapidjson::Value().SetInt(key), allocator);
			for (int& key : keybinds5K)
				array5K.PushBack(rapidjson::Value().SetInt(key), allocator);
		}
		else {
			for (int& key : defaultKeybinds4K)
				array4K.PushBack(rapidjson::Value().SetInt(key), allocator);
			for (int& key : defaultKeybinds5K)
				array5K.PushBack(rapidjson::Value().SetInt(key), allocator);
		}
		rapidjson::Value array4KAlt(rapidjson::kArrayType);
		rapidjson::Value array5KAlt(rapidjson::kArrayType);
		for (int& key : defaultKeybinds4KAlt)
			array4KAlt.PushBack(rapidjson::Value().SetInt(key), allocator);
		for (int& key : defaultKeybinds5KAlt)
			array5KAlt.PushBack(rapidjson::Value().SetInt(key), allocator);
		settings.AddMember("keybinds", rapidjson::Value(rapidjson::kObjectType), allocator);
		settings["keybinds"].AddMember("4k", array4K, allocator);
		settings["keybinds"].AddMember("5k", array5K, allocator);
		settings["keybinds"].AddMember("4kAlt", array4KAlt, allocator);
		settings["keybinds"].AddMember("5kAlt", array5KAlt, allocator);
		rapidjson::Value overdrive(keybindOverdrive);
		rapidjson::Value overdriveAlt(keybindOverdriveAlt);
		settings["keybinds"].AddMember("overdrive", overdrive, allocator);
		settings["keybinds"].AddMember("overdriveAlt", overdriveAlt, allocator); 
		rapidjson::Value arrayController4K(rapidjson::kArrayType);
		rapidjson::Value arrayController5K(rapidjson::kArrayType);
		rapidjson::Value arrayController4KDirection(rapidjson::kArrayType);
		rapidjson::Value arrayController5KDirection(rapidjson::kArrayType);
		for (int& key : defaultController4K)
			arrayController4K.PushBack(rapidjson::Value().SetInt(key), allocator);
		for (int& key : defaultController5K)
			arrayController5K.PushBack(rapidjson::Value().SetInt(key), allocator);
		for (int& key : defaultController4KAxisDirection)
			arrayController4KDirection.PushBack(rapidjson::Value().SetInt(key), allocator);
		for (int& key : defaultController5KAxisDirection)
			arrayController5KDirection.PushBack(rapidjson::Value().SetInt(key), allocator);
		settings.AddMember("controllerbinds", rapidjson::Value(rapidjson::kObjectType), allocator);

		settings["controllerbinds"].AddMember("type", rapidjson::Value().SetInt(defaultControllerType), allocator);
		settings["controllerbinds"].AddMember("4k", arrayController4K, allocator);
		settings["controllerbinds"].AddMember("4k_direction", arrayController4KDirection, allocator);
		settings["controllerbinds"].AddMember("5k", arrayController5K, allocator);
		settings["controllerbinds"].AddMember("5k_direction", arrayController5KDirection, allocator);
		settings["controllerbinds"].AddMember("overdrive", rapidjson::Value().SetInt(defaultControllerOverdrive), allocator);
		settings["controllerbinds"].AddMember("overdrive_direction", rapidjson::Value().SetInt(defaultControllerOverdriveAxisDirection), allocator);
		rapidjson::Value avOffset(avOffsetMS);
		settings.AddMember("avOffset", avOffset, allocator);
		rapidjson::Value inputOffset(inputOffsetMS);
		settings.AddMember("inputOffset", inputOffset, allocator);
		rapidjson::Value mirrorValue(defaultMirrorMode);
		settings.AddMember("mirror",mirrorValue, allocator);
		rapidjson::Value trackSpeedVal(4);
		settings.AddMember("trackSpeed", trackSpeedVal, allocator);
        rapidjson::Value length(1.0f);
        settings.AddMember("length", highwayLengthMult, allocator);
		rapidjson::Value arrayTrackSpeedOptions(rapidjson::kArrayType);
		for (float& speed : defaultTrackSpeedOptions)
			arrayTrackSpeedOptions.PushBack(rapidjson::Value().SetFloat(speed), allocator);
		settings.AddMember("trackSpeedOptions", arrayTrackSpeedOptions, allocator);
        settings.AddMember("missHighwayColor", missHighwayDefault, allocator);
        rapidjson::Value missHighwayColor;

        rapidjson::Value arraySongDirs(rapidjson::kArrayType);
        for (filesystem::path &path: defaultSongPaths)
            arraySongDirs.PushBack(rapidjson::Value().SetString((const char*)(path.c_str()), allocator), allocator);

        settings.AddMember("songDirectories", arraySongDirs, allocator);

		saveSettings(settingsFile);
	}
	void loadSettings(std::filesystem::path settingsFile) {
		bool keybindsError = false;
		bool keybinds4KError = false;
		bool keybinds5KError = false;
		bool keybinds4KAltError = false;
		bool keybinds5KAltError = false;
		bool keybindsOverdriveError = false;
		bool keybindsOverdriveAltError = false;
		bool controllerError = false;
		bool controllerTypeError = false;
		bool controller4KError = false;
		bool controller4KDirectionError = false;
		bool controller5KError = false;
		bool controller5KDirectionError = false;
		bool controllerOverdriveError = false;
		bool controllerOverdriveDirectionError = false;
        bool songDirectoryError = false;
		bool avError = false;
		bool inputError = false;
		bool mirrorError = false;
		bool trackSpeedOptionsError = false;
        bool highwayLengthError = false;
		bool trackSpeedError = false;
        bool MissHighwayError = false;
		if (std::filesystem::exists(settingsFile)) {
			std::ifstream ifs(settingsFile);
			if (!ifs.is_open()) {
				std::cerr << "Failed to open JSON file." << std::endl;
			}
			std::string jsonString((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
			ifs.close();
			settings.Parse(jsonString.c_str());

			if (settings.IsObject())
			{
				if (settings.HasMember("keybinds") && settings["keybinds"].IsObject()) {
					if (settings["keybinds"].HasMember("4k") && settings["keybinds"]["4k"].IsArray()) {
						const rapidjson::Value& arr = settings["keybinds"]["4k"];
						if (arr.Size() == 4) {
							for (int i = 0; i < 4; i++) {
								if (arr[i].IsInt()) {
									keybinds4K[i] = arr[i].GetInt();
								}
								else {
									keybinds4KError = true;
								}
							}
							prev4K = keybinds4K;
						}
						else {
							keybinds4KError = true;
						}
					}
					if (settings["keybinds"].HasMember("5k") && settings["keybinds"]["5k"].IsArray()) {
						const rapidjson::Value& arr = settings["keybinds"]["5k"];
						if (arr.Size() == 5) {
							for (int i = 0; i < 5; i++) {
								if (arr[i].IsInt()) {
									keybinds5K[i] = arr[i].GetInt();
								}
								else {
									keybinds5KError = true;
								}
							}
							prev5K = keybinds5K;
						}
						else {
							keybinds5KError = true;
						}
					}
					if (settings["keybinds"].HasMember("4kAlt") && settings["keybinds"]["4kAlt"].IsArray()) {
						const rapidjson::Value& arr = settings["keybinds"]["4kAlt"];
						if (arr.Size() == 4) {
							for (int i = 0; i < 4; i++) {
								if (arr[i].IsInt()) {
									keybinds4KAlt[i] = arr[i].GetInt();
								}
								else {
									keybinds4KAltError = true;
								}
							}
							prev4KAlt = keybinds4KAlt;
						}
						else {
							keybinds4KAltError = true;
						}
					}
					if (settings["keybinds"].HasMember("5kAlt") && settings["keybinds"]["5kAlt"].IsArray()) {
						const rapidjson::Value& arr = settings["keybinds"]["5kAlt"];
						if (arr.Size() == 5) {
							for (int i = 0; i < 5; i++) {
								if (arr[i].IsInt()) {
									keybinds5KAlt[i] = arr[i].GetInt();
								}
								else {
									keybinds5KAltError = true;
								}
							}
							prev5KAlt = keybinds5KAlt;
						}
						else {
							keybinds5KAltError = true;
						}
					}
					if (settings["keybinds"].HasMember("overdrive") && settings["keybinds"]["overdrive"].IsInt()) {
						keybindOverdrive = settings["keybinds"]["overdrive"].GetInt();
						prevOverdrive = keybindOverdrive;
					}
					else {
						keybindsOverdriveError = true;
					}

					if (settings["keybinds"].HasMember("overdriveAlt") && settings["keybinds"]["overdriveAlt"].IsInt()) {
						keybindOverdriveAlt = settings["keybinds"]["overdriveAlt"].GetInt();
						prevOverdriveAlt = keybindOverdriveAlt;
					}
					else {
						keybindsOverdriveAltError = true;
					}
				}
				else {
					keybindsError = true;
				}
				if (settings.HasMember("controllerbinds") && settings["controllerbinds"].IsObject()) {
					if (settings["controllerbinds"].HasMember("type") && settings["controllerbinds"]["type"].IsInt()) {
						controllerType = settings["controllerbinds"]["type"].GetInt();
						prevControllerType = controllerType;
					}
					else {
						controllerTypeError = true;
					}

					if (settings["controllerbinds"].HasMember("4k") && settings["controllerbinds"]["4k"].IsArray()) {
						const rapidjson::Value& arr = settings["controllerbinds"]["4k"];
						if (arr.Size() == 4) {
							for (int i = 0; i < 4; i++) {
								if (arr[i].IsInt()) {
									controller4K[i] = arr[i].GetInt();
								}
								else {
									controller4KError = true;
								}
							}
							prevController4K = controller4K;
						}
						else {
							controller4KError = true;
						}
					}
					if (settings["controllerbinds"].HasMember("4k_direction") && settings["controllerbinds"]["4k_direction"].IsArray()) {
						const rapidjson::Value& arr = settings["controllerbinds"]["4k_direction"];
						if (arr.Size() == 4) {
							for (int i = 0; i < 4; i++) {
								if (arr[i].IsInt()) {
									controller4KAxisDirection[i] = arr[i].GetInt();
								}
								else {
									controller4KDirectionError = true;
								}
							}
							prevController4KAxisDirection = controller4KAxisDirection;
						}
						else {
							controller4KDirectionError = true;
						}
					}
					if (settings["controllerbinds"].HasMember("5k") && settings["controllerbinds"]["5k"].IsArray()) {
						const rapidjson::Value& arr = settings["controllerbinds"]["5k"];
						if (arr.Size() == 5) {
							for (int i = 0; i < 5; i++) {
								if (arr[i].IsInt()) {
									controller5K[i] = arr[i].GetInt();
								}
								else {
									controller5KError = true;
								}
							}
							prevController5K = controller5K;
						}
						else {
							controller5KError = true;
						}
					}
					if (settings["controllerbinds"].HasMember("5k_direction") && settings["controllerbinds"]["5k_direction"].IsArray()) {
						const rapidjson::Value& arr = settings["controllerbinds"]["5k_direction"];
						if (arr.Size() == 5) {
							for (int i = 0; i < 5; i++) {
								if (arr[i].IsInt()) {
									controller5KAxisDirection[i] = arr[i].GetInt();
								}
								else {
									controller5KDirectionError = true;
								}
							}
							prevController5KAxisDirection = controller5KAxisDirection;
						}
						else {
							controller5KDirectionError = true;
						}
					}
					if (settings["controllerbinds"].HasMember("overdrive") && settings["controllerbinds"]["overdrive"].IsInt()) {
						controllerOverdrive = settings["controllerbinds"]["overdrive"].GetInt();
						prevControllerOverdrive = controllerOverdrive;
					}
					else {
						controllerOverdriveError = true;
					}
					if (settings["controllerbinds"].HasMember("overdrive_direction") && settings["controllerbinds"]["overdrive_direction"].IsInt()) {
						controllerOverdriveAxisDirection = settings["controllerbinds"]["overdrive_direction"].GetInt();
						prevControllerOverdriveAxisDirection = controllerOverdriveAxisDirection;
					}
					else {
						controllerOverdriveDirectionError = true;
					}
				}
				else {
					controllerError = true;
				}

				if (settings.HasMember("avOffset") && settings["avOffset"].IsInt()) {
					avOffsetMS = settings["avOffset"].GetInt();
					prevAvOffsetMS = avOffsetMS;
					VideoOffset = -(float)(avOffsetMS / 1000);
				}
				else {
					avError = true;
				}
				if (settings.HasMember("inputOffset") && settings["inputOffset"].IsInt()) {
					inputOffsetMS = settings["inputOffset"].GetInt();
					prevInputOffsetMS = inputOffsetMS;
					InputOffset = (float)(inputOffsetMS / 1000);
				}
				else {
					inputError = true;
				}
				if (settings.HasMember("trackSpeed") && settings["trackSpeed"].IsInt()) {
					trackSpeed = settings["trackSpeed"].GetInt();
					prevTrackSpeed = trackSpeed;
				}
				else {
					trackSpeedError = true;
				}
				if (settings.HasMember("trackSpeedOptions") && settings["trackSpeedOptions"].IsArray()) {
					trackSpeedOptions = {};
					const rapidjson::Value& arr = settings["trackSpeedOptions"];
					for (int i = 0; i < arr.Size(); i++) {
						if (arr[i].IsFloat()) {
							trackSpeedOptions.push_back(arr[i].GetFloat());
						}
						else {
							trackSpeedOptionsError = true;
						}
					}
				}
				else {
					trackSpeedOptionsError = true;
				}
                if (settings.HasMember("length") && settings["length"].IsFloat()){
                    highwayLengthMult = settings["length"].GetFloat();
                } else {
                    highwayLengthError = true;
                }
                if (settings.HasMember("missHighwayColor") && settings["missHighwayColor"].IsBool()) {
                    MissHighwayColor = settings["missHighwayColor"].GetBool();
                } else {
                    MissHighwayError = true;
                } 
				if (settings.HasMember("mirror") && settings["mirror"].IsBool()) {
					mirrorMode = settings["mirror"].GetBool();
				}
				else {
					mirrorError = true;
				}
                if (settings.HasMember("songDirectories") && settings["songDirectories"].IsArray()){

                        for (auto& songPath : settings["songDirectories"].GetArray()) {
                            songPaths.push_back(songPath.GetString());
                        }


                } else {
                    songDirectoryError = true;
                }
			}
		}
		else {
			writeDefaultSettings(settingsFile);
		}


		if (keybindsError) {
			if (settings.HasMember("keybinds"))
				settings.EraseMember("keybinds");
			rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
			rapidjson::Value array4K(rapidjson::kArrayType);
			for (int& key :defaultKeybinds4K)
				array4K.PushBack(rapidjson::Value().SetInt(key), allocator);
			rapidjson::Value array5K(rapidjson::kArrayType);
			for (int& key :defaultKeybinds5K)
				array5K.PushBack(rapidjson::Value().SetInt(key), allocator);
			rapidjson::Value array4KAlt (rapidjson::kArrayType);
			for (int& key : defaultKeybinds4KAlt)
				array4KAlt.PushBack(rapidjson::Value().SetInt(key), allocator);
			rapidjson::Value array5KAlt (rapidjson::kArrayType);
			for (int& key : defaultKeybinds5KAlt)
				array5KAlt.PushBack(rapidjson::Value().SetInt(key), allocator);
			settings.AddMember("keybinds", rapidjson::Value(rapidjson::kObjectType), allocator);
			settings["keybinds"].AddMember("4k", array4K, allocator);
			settings["keybinds"].AddMember("5k", array5K, allocator);
			settings["keybinds"].AddMember("4kAlt", array4KAlt, allocator);
			settings["keybinds"].AddMember("5kAlt", array5KAlt, allocator);
			rapidjson::Value overdriveKey(defaultKeybindOverdrive);
			settings["keybinds"].AddMember("overdrive", overdriveKey, allocator);
			rapidjson::Value overdriveKeyAlt(defaultKeybindOverdriveAlt);
			settings["keybinds"].AddMember("overdriveAlt", overdriveKeyAlt, allocator);
		}
		if (keybinds4KError) {
			if(settings["keybinds"].HasMember("4k"))
				settings["keybinds"].EraseMember("4k");
			rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
			rapidjson::Value array4K(rapidjson::kArrayType);
			for (int& key :defaultKeybinds4K)
				array4K.PushBack(rapidjson::Value().SetInt(key), allocator);
			settings["keybinds"].AddMember("4k", array4K, allocator);
		}
		if (keybinds5KError) {
			if (settings["keybinds"].HasMember("5k"))
				settings["keybinds"].EraseMember("5k");
			rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
			rapidjson::Value array5K(rapidjson::kArrayType);
			for (int& key :defaultKeybinds5K)
				array5K.PushBack(rapidjson::Value().SetInt(key), allocator);
			settings["keybinds"].AddMember("5k", array5K, allocator);
		}
		if (keybinds4KAltError) {
			if (settings["keybinds"].HasMember("4kAlt"))
				settings["keybinds"].EraseMember("4kAlt");
			rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
			rapidjson::Value array4K(rapidjson::kArrayType);
			for (int& key : defaultKeybinds4KAlt)
				array4K.PushBack(rapidjson::Value().SetInt(key), allocator);
			settings["keybinds"].AddMember("4kAlt", array4K, allocator);
		}
		if (keybinds5KAltError) {
			if (settings["keybinds"].HasMember("5kAlt"))
				settings["keybinds"].EraseMember("5kAlt");
			rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
			rapidjson::Value array5K(rapidjson::kArrayType);
			for (int& key : defaultKeybinds5KAlt)
				array5K.PushBack(rapidjson::Value().SetInt(key), allocator);
			settings["keybinds"].AddMember("5kAlt", array5K, allocator);
		}
		if (keybindsOverdriveError) {
			if (settings["keybinds"].HasMember("overdrive"))
				settings["keybinds"].EraseMember("overdrive");
			rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
			rapidjson::Value overdriveKey(defaultKeybindOverdrive);
			settings["keybinds"].AddMember("overdrive", overdriveKey, allocator);
		}
		if (keybindsOverdriveAltError){
			if (settings["keybinds"].HasMember("overdriveAlt"))
				settings["keybinds"].EraseMember("overdriveAlt");
			rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
			rapidjson::Value overdriveKeyAlt(defaultKeybindOverdriveAlt);
			settings["keybinds"].AddMember("overdriveAlt", overdriveKeyAlt, allocator);
		}
		if (controllerError) {
			if (settings.HasMember("controllerbinds"))
				settings.EraseMember("controllerbinds");
			rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
			rapidjson::Value array4K(rapidjson::kArrayType);
			for (int& key : defaultController4K)
				array4K.PushBack(rapidjson::Value().SetInt(key), allocator);
			rapidjson::Value array5K(rapidjson::kArrayType);
			for (int& key : defaultController5K)
				array5K.PushBack(rapidjson::Value().SetInt(key), allocator);
			rapidjson::Value array4KDirection(rapidjson::kArrayType);
			for (int& key : defaultController4KAxisDirection)
				array4KDirection.PushBack(rapidjson::Value().SetInt(key), allocator);
			rapidjson::Value array5KDirection(rapidjson::kArrayType);
			for (int& key : defaultController5KAxisDirection)
				array5KDirection.PushBack(rapidjson::Value().SetInt(key), allocator);
			settings.AddMember("controllerbinds", rapidjson::Value(rapidjson::kObjectType), allocator);
			settings["controllerbinds"].AddMember("4k", array4K, allocator);
			settings["controllerbinds"].AddMember("4k_direction", array4KDirection, allocator);
			settings["controllerbinds"].AddMember("5k", array5K, allocator);
			settings["controllerbinds"].AddMember("5k_direction", array4KDirection, allocator);
			rapidjson::Value overdriveKey(defaultControllerOverdrive);
			settings["controllerbinds"].AddMember("overdrive", overdriveKey, allocator);
			rapidjson::Value overdriveDirection(defaultControllerOverdriveAxisDirection);
			settings["controllerbinds"].AddMember("overdrive_direction", overdriveDirection, allocator);
		}
		if (controllerTypeError) {
			if (settings["controllerbinds"].HasMember("type"))
				settings["controllerbinds"].EraseMember("type");
			rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
			rapidjson::Value controllerTypeValue(defaultControllerType);
			settings["controllerbinds"].AddMember("type", controllerTypeValue, allocator);
		}
		if (controller4KError) {
			if (settings["controllerbinds"].HasMember("4k"))
				settings["controllerbinds"].EraseMember("4k");
			rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
			rapidjson::Value array4K(rapidjson::kArrayType);
			for (int& key : defaultController4K)
				array4K.PushBack(rapidjson::Value().SetInt(key), allocator);
			settings["controllerbinds"].AddMember("4k", array4K, allocator);
		}
		if (controller5KError) {
			if (settings["controllerbinds"].HasMember("5k"))
				settings["controllerbinds"].EraseMember("5k");
			rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
			rapidjson::Value array5K(rapidjson::kArrayType);
			for (int& key : defaultController5K)
				array5K.PushBack(rapidjson::Value().SetInt(key), allocator);
			settings["controllerbinds"].AddMember("5k", array5K, allocator);
		}
		if (controllerOverdriveError) {
			if (settings["controllerbinds"].HasMember("overdrive"))
				settings["controllerbinds"].EraseMember("overdrive");
			rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
			rapidjson::Value overdriveKey(defaultControllerOverdrive);
			settings["controllerbinds"].AddMember("overdrive", overdriveKey, allocator);
		}
		if (controller4KDirectionError) {
			if (settings["controllerbinds"].HasMember("4k_direction"))
				settings["controllerbinds"].EraseMember("4k_direction");
			rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
			rapidjson::Value array4K(rapidjson::kArrayType);
			for (int& key : defaultController4KAxisDirection)
				array4K.PushBack(rapidjson::Value().SetInt(key), allocator);
			settings["controllerbinds"].AddMember("4k_direction", array4K, allocator);
		}
		if (controller5KDirectionError) {
			if (settings["controllerbinds"].HasMember("5k_direction"))
				settings["controllerbinds"].EraseMember("5k_direction");
			rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
			rapidjson::Value array5K(rapidjson::kArrayType);
			for (int& key : defaultController5KAxisDirection)
				array5K.PushBack(rapidjson::Value().SetInt(key), allocator);
			settings["controllerbinds"].AddMember("5k_direction", array5K, allocator);
		}
		if (controllerOverdriveDirectionError) {
			if (settings["controllerbinds"].HasMember("overdrive_direction"))
				settings["controllerbinds"].EraseMember("overdrive_direction");
			rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
			rapidjson::Value overdriveKey(defaultControllerOverdriveAxisDirection);
			settings["controllerbinds"].AddMember("overdrive_direction", overdriveKey, allocator);
		}
		if (avError) {
			if (settings.HasMember("avOffset"))
				settings.EraseMember("avOffset");
			rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
			rapidjson::Value avOffset;
			avOffset.SetInt(0);
			settings.AddMember("avOffset", avOffset, allocator);
		}
		if (inputError) {
			if (settings.HasMember("inputOffset"))
				settings.EraseMember("inputOffset");
			rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
			rapidjson::Value inputOffset;
			inputOffset.SetInt(0);
			settings.AddMember("inputOffset", inputOffset, allocator);
		}
		if (trackSpeedError) {
			if(settings.HasMember("trackSpeed"))
				settings.EraseMember("trackSpeed");
			rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
			rapidjson::Value trackSpeedVal;
			trackSpeedVal.SetInt(4);
			settings.AddMember("trackSpeed", trackSpeedVal, allocator);
		}
		if (trackSpeedOptionsError) {
			trackSpeedOptions = defaultTrackSpeedOptions;
			if (settings.HasMember("trackSpeedOptions"))
				settings.EraseMember("trackSpeedOptions");
			rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
			rapidjson::Value arrayTrackSpeedOptions(rapidjson::kArrayType);
			for (float& speed : defaultTrackSpeedOptions)
				arrayTrackSpeedOptions.PushBack(rapidjson::Value().SetFloat(speed), allocator);
			settings.AddMember("trackSpeedOptions", arrayTrackSpeedOptions, allocator);
		}
        if (songDirectoryError) {
            if (settings.HasMember("songDirectories"))
                settings.EraseMember("songDirectories");
            rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
            rapidjson::Value arraySongDir(rapidjson::kArrayType);
            for (filesystem::path &path: defaultSongPaths)
                arraySongDir.PushBack(rapidjson::Value().SetString((const char*)(path.c_str()), allocator), allocator);
            settings.AddMember("songDirectories", arraySongDir, allocator);
        }
        if (MissHighwayError) {
            if (settings.HasMember("missHighwayColor"))
                settings.EraseMember("missHighwayColor");
            rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
            rapidjson::Value missHighwayColor;
            missHighwayColor.SetBool(false);
            settings.AddMember("missHighwayColor", missHighwayColor, allocator);
        }
        if (highwayLengthError) {
            if (settings.HasMember("length"))
                settings.EraseMember("length");
            rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
            settings.AddMember("length", highwayLengthMult, allocator);
        }
		if (mirrorError) {
			if (settings.HasMember("mirror"))
				settings.EraseMember("mirror");
			rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
			rapidjson::Value mirrorValue;
			mirrorValue .SetBool(defaultMirrorMode);
			settings.AddMember("mirror", mirrorValue, allocator);
		}
		if (songDirectoryError || highwayLengthError || mirrorError || MissHighwayError || keybindsError || keybinds4KError || keybinds5KError || keybinds4KAltError || keybinds5KAltError|| keybindsOverdriveError || keybindsOverdriveAltError || controllerError || controllerTypeError || controller4KError || controller5KError || controllerOverdriveError || controller4KDirectionError || controller5KDirectionError || controllerOverdriveDirectionError || avError || inputError|| trackSpeedError || trackSpeedOptionsError) {
			ensureValuesExist();
			saveSettings(settingsFile);
		}
	}
	void migrateSettings(std::filesystem::path keybindsFile, std::filesystem::path settingsFile) {
		std::ifstream ifs(keybindsFile);
		if (!ifs.is_open()) {
			std::cerr << "Failed to open JSON file." << std::endl;
		}
		std::string jsonString((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		ifs.close();
		rapidjson::Document keybinds;
		keybinds.Parse(jsonString.c_str());
		bool keybindsError = false;
		bool keybinds4KError = false;
		bool keybinds5KError = false;
		bool avOffsetError = false;
		bool inputOffsetError = false;
		if (keybinds.IsObject())
		{
			if (keybinds.HasMember("4k") && keybinds["4k"].IsArray()) {
				const rapidjson::Value& arr = keybinds["4k"];
				if (arr.Size() == 4) {
					for (int i = 0; i < 4; i++) {
						if (arr[i].IsInt()) {
							keybinds4K[i] = arr[i].GetInt();
						}
						else {
							keybinds4KError = true;
						}
					}
					prev4K = keybinds4K;
				}
				else {
					keybinds4KError = true;
				}
			}
			if (keybinds.HasMember("5k") && keybinds["5k"].IsArray()) {
				const rapidjson::Value& arr = keybinds["5k"];
				if (arr.Size() == 5) {
					for (int i = 0; i < 5; i++) {
						if (arr[i].IsInt()) {
							keybinds5K[i] = arr[i].GetInt();
						}
						else {
							keybinds5KError = true;
						}
					}
					prev5K = keybinds5K;
				}
				else {
					keybinds5KError = true;
				}
			}
			if (keybinds.HasMember("avOffset") && keybinds["avOffset"].IsInt()) {
				avOffsetMS = keybinds["avOffset"].GetInt();
				prevAvOffsetMS = avOffsetMS;
                VideoOffset = -(float)(avOffsetMS / 1000);
				
			}
			else {
				avOffsetError = true;
			}
			if (keybinds.HasMember("inputOffset") && keybinds["inputOffset"].IsInt()) {
				inputOffsetMS = keybinds["inputOffset"].GetInt();
				prevInputOffsetMS = inputOffsetMS;
                InputOffset = (float)(inputOffsetMS / 1000);
			}
			else {
				inputOffsetError = true;
			}
		}
		else {
			keybindsError = true;
		}
		if (keybindsError) {
			writeDefaultSettings(settingsFile);
		}
		else {
			if (keybinds4KError) {
				for (int i = 0; i < 4; i++) {
					keybinds4K[i] = defaultKeybinds4K[i];
				}
			}
			if (keybinds5KError) {
				for (int i = 0; i < 5; i++) {
					keybinds5K[i] = defaultKeybinds5K[i];
				}
			}

			writeDefaultSettings(settingsFile, true);
		}
		std::filesystem::remove(keybindsFile);
	}
	const void saveSettings(std::filesystem::path settingsFile) {
		rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
        rapidjson::Value::MemberIterator lengthMember = settings.FindMember("length");
        lengthMember->value.SetFloat(highwayLengthMult);
		rapidjson::Value::MemberIterator trackSpeedMember = settings.FindMember("trackSpeed");
		trackSpeedMember->value.SetInt(trackSpeed);
		rapidjson::Value::MemberIterator avOffsetMember = settings.FindMember("avOffset");
		avOffsetMember->value.SetInt(avOffsetMS);
		rapidjson::Value::MemberIterator inputOffsetMember = settings.FindMember("inputOffset");
		inputOffsetMember->value.SetInt(inputOffsetMS);
        rapidjson::Value::MemberIterator missHighwayColorMember = settings.FindMember("missHighwayColor");
        missHighwayColorMember->value.SetBool(missHighwayDefault);
		rapidjson::Value::MemberIterator mirrorMember = settings.FindMember("mirror");
		mirrorMember->value.SetBool(mirrorMode);
        rapidjson::Value::MemberIterator songDirMember = settings.FindMember("songDirectories");
        songDirMember->value.Clear();
        for (filesystem::path &path: songPaths)
            songDirMember->value.PushBack(rapidjson::Value().SetString((const char*)(path.c_str()), allocator), allocator);
		rapidjson::Value::MemberIterator keybinds4KMember = settings["keybinds"].FindMember("4k");
		keybinds4KMember->value.Clear();
		for (int& key : keybinds4K)
			keybinds4KMember->value.PushBack(rapidjson::Value().SetInt(key), allocator);
		rapidjson::Value::MemberIterator keybinds5KMember = settings["keybinds"].FindMember("5k");
		keybinds5KMember->value.Clear();
		for (int& key : keybinds5K)
			keybinds5KMember->value.PushBack(rapidjson::Value().SetInt(key), allocator);
		rapidjson::Value::MemberIterator keybinds4KAltMember = settings["keybinds"].FindMember("4kAlt");
		keybinds4KAltMember->value.Clear();
		for (int& key : keybinds4KAlt)
			keybinds4KAltMember->value.PushBack(rapidjson::Value().SetInt(key), allocator);
		rapidjson::Value::MemberIterator keybinds5KAltMember = settings["keybinds"].FindMember("5kAlt");
		keybinds5KAltMember->value.Clear();
		for (int& key : keybinds5KAlt)
			keybinds5KAltMember->value.PushBack(rapidjson::Value().SetInt(key), allocator);
		rapidjson::Value::MemberIterator overdriveKeyMember = settings["keybinds"].FindMember("overdrive");
		overdriveKeyMember->value.SetInt(keybindOverdrive);
		rapidjson::Value::MemberIterator overdriveKeyAltMember = settings["keybinds"].FindMember("overdriveAlt");
		overdriveKeyAltMember->value.SetInt(keybindOverdriveAlt);

		rapidjson::Value::MemberIterator controller4KMember = settings["controllerbinds"].FindMember("4k");
		controller4KMember->value.Clear();
		for (int& key : controller4K)
			controller4KMember->value.PushBack(rapidjson::Value().SetInt(key), allocator);
		rapidjson::Value::MemberIterator controller5KMember = settings["controllerbinds"].FindMember("5k");
		controller5KMember->value.Clear();
		for (int& key : controller5K)
			controller5KMember->value.PushBack(rapidjson::Value().SetInt(key), allocator);
		rapidjson::Value::MemberIterator controller4KDirMember = settings["controllerbinds"].FindMember("4k_direction");
		controller4KDirMember->value.Clear();
		for (int& key : controller4KAxisDirection)
			controller4KDirMember->value.PushBack(rapidjson::Value().SetInt(key), allocator);
		rapidjson::Value::MemberIterator controller5KDirMember = settings["controllerbinds"].FindMember("5k_direction");
		controller5KDirMember->value.Clear();
		for (int& key : controller5KAxisDirection)
			controller5KDirMember->value.PushBack(rapidjson::Value().SetInt(key), allocator);
		rapidjson::Value::MemberIterator overdriveControllerMember = settings["controllerbinds"].FindMember("overdrive");
		overdriveControllerMember->value.SetInt(controllerOverdrive);
		rapidjson::Value::MemberIterator overdriveControllerDirMember = settings["controllerbinds"].FindMember("overdrive_direction");
		overdriveControllerDirMember->value.SetInt(controllerOverdriveAxisDirection);
		rapidjson::Value::MemberIterator controllerTypeMember = settings["controllerbinds"].FindMember("type");
		controllerTypeMember->value.SetInt(controllerType);

		char writeBuffer[8192];
		FILE* fp = fopen(settingsFile.string().c_str(), "wb");
		rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
		rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
		settings.Accept(writer);
		fclose(fp);
	}
};
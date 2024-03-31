#pragma once
#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
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
	}
public:
	rapidjson::Document settings;
	std::vector<int> defaultKeybinds4K{ KEY_D,KEY_F,KEY_J,KEY_K };
	std::vector<int> defaultKeybinds5K{ KEY_D,KEY_F,KEY_J,KEY_K,KEY_L };
	std::vector<int> defaultKeybinds4KAlt{ -1,-1,-1,-1 };
	std::vector<int> defaultKeybinds5KAlt{ -1,-1,-1,-1,-1 };
	int defaultKeybindOverdrive = 32;
	int defaultKeybindOverdriveAlt = -1;
	std::vector<int> keybinds4K = defaultKeybinds4K;
	std::vector<int> keybinds5K = defaultKeybinds5K;
	std::vector<int> keybinds4KAlt = defaultKeybinds4KAlt;
	std::vector<int> keybinds5KAlt = defaultKeybinds5KAlt;
	int keybindOverdrive = defaultKeybindOverdrive;
	int keybindOverdriveAlt = defaultKeybindOverdriveAlt;
	std::vector<int> prev4k = keybinds4K;
	std::vector<int> prev5k = keybinds5K;
	std::vector<int> prev4kAlt = keybinds4KAlt;
	std::vector<int> prev5kAlt = keybinds5KAlt;
	int prevOverdrive = keybindOverdrive;
	int prevOverdriveAlt = keybindOverdriveAlt;
	std::vector<float> defaultTrackSpeedOptions = { 0.5f,0.75f,1.0f,1.25f,1.5f,1.75f,2.0f };
	std::vector<float> trackSpeedOptions = defaultTrackSpeedOptions;
	int trackSpeed = 4;
	int prevTrackSpeed = trackSpeed;
	int avOffsetMS = 0;
	int prevAvOffsetMS = avOffsetMS;
	int inputOffsetMS = 0;
	int prevInputOffsetMS = inputOffsetMS;
	bool changing4k = false;
	bool changingAlt = false;
    bool missHighwayDefault = false;
    bool prevMissHighwayColor = missHighwayDefault;

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
		rapidjson::Value avOffset(avOffsetMS);
		settings.AddMember("avOffset", avOffset, allocator);
		rapidjson::Value inputOffset(inputOffsetMS);
		settings.AddMember("inputOffset", inputOffset, allocator);
		rapidjson::Value trackSpeedVal(4);
		settings.AddMember("trackSpeed", trackSpeedVal, allocator);
		rapidjson::Value arrayTrackSpeedOptions(rapidjson::kArrayType);
		for (float& speed : defaultTrackSpeedOptions)
			arrayTrackSpeedOptions.PushBack(rapidjson::Value().SetFloat(speed), allocator);
		settings.AddMember("trackSpeedOptions", arrayTrackSpeedOptions, allocator);
        settings.AddMember("missHighwayColor", missHighwayDefault, allocator);
        rapidjson::Value missHighwayColor;
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
		bool avError = false;
		bool inputError = false;
		bool trackSpeedOptionsError = false;
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
							prev4k = keybinds4K;
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
							prev5k = keybinds5K;
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
							prev4kAlt = keybinds4KAlt;
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
							prev5kAlt = keybinds5KAlt;
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
                if (settings.HasMember("missHighwayColor") && settings["missHighwayColor"].IsBool()) {
                    MissHighwayColor = settings["missHighwayColor"].GetBool();
                } else {
                    MissHighwayError = true;
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
			settings.AddMember("keybinds", rapidjson::Value(rapidjson::kObjectType), allocator);
			settings["keybinds"].AddMember("4k", array4K, allocator);
			settings["keybinds"].AddMember("5k", array5K, allocator);

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
			rapidjson::Value overdriveKey(KEY_SPACE);
			settings["keybinds"].AddMember("overdrive", overdriveKey, allocator);
		}
		if (keybindsOverdriveAltError){
			if (settings["keybinds"].HasMember("overdriveAlt"))
				settings["keybinds"].EraseMember("overdriveAlt");
			rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
			rapidjson::Value overdriveKeyAlt(defaultKeybindOverdriveAlt);
			settings["keybinds"].AddMember("overdriveAlt", overdriveKeyAlt, allocator);
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
        if (MissHighwayError) {
            if (settings.HasMember("missHighwayColor"))
                settings.EraseMember("missHighwayColor");
            rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
            rapidjson::Value missHighwayColor;
            missHighwayColor.SetBool(false);
            settings.AddMember("missHighwayColor", missHighwayColor, allocator);
        }
		if (MissHighwayError||keybindsError || keybinds4KError || keybinds5KError || keybinds4KAltError || keybinds5KAltError|| keybindsOverdriveError || keybindsOverdriveAltError || avError || inputError|| trackSpeedError || trackSpeedOptionsError) {
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
					prev4k = keybinds4K;
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
					prev5k = keybinds5K;
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
		rapidjson::Value::MemberIterator trackSpeedMember = settings.FindMember("trackSpeed");
		trackSpeedMember->value.SetInt(trackSpeed);
		rapidjson::Value::MemberIterator avOffsetMember = settings.FindMember("avOffset");
		avOffsetMember->value.SetInt(avOffsetMS);
		rapidjson::Value::MemberIterator inputOffsetMember = settings.FindMember("inputOffset");
		inputOffsetMember->value.SetInt(inputOffsetMS);
        rapidjson::Value::MemberIterator missHighwayColorMember = settings.FindMember("missHighwayColor");
        missHighwayColorMember->value.SetBool(missHighwayDefault);
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
		char writeBuffer[8192];
		FILE* fp = fopen(settingsFile.string().c_str(), "wb");
		rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
		rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
		settings.Accept(writer);
		fclose(fp);
	}
};
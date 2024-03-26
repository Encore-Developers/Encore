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
public:
	rapidjson::Document settings;
	std::vector<int> defaultKeybinds4K{ KEY_D,KEY_F,KEY_J,KEY_K };
	std::vector<int> defaultKeybinds5K{ KEY_D,KEY_F,KEY_J,KEY_K,KEY_L };
	std::vector<int> keybinds4K = defaultKeybinds4K;
	std::vector<int> keybinds5K = defaultKeybinds5K;
	std::vector<int> prev4k = keybinds4K;
	std::vector<int> prev5k = keybinds5K;
	std::vector<float> defaultTrackSpeedOptions = { 0.5f,0.75f,1.0f,1.25f,1.5f,1.75f,2.0f };
	std::vector<float> trackSpeedOptions = defaultTrackSpeedOptions;
	int trackSpeed = 4;
	int prevTrackSpeed = trackSpeed;
	bool changing4k = false;
	void writeDefaultSettings(std::filesystem::path settingsFile) {
		rapidjson::Document::AllocatorType& allocator = settings.GetAllocator();
		settings.SetObject();
		rapidjson::Value array4K(rapidjson::kArrayType);
		for (int& key : defaultKeybinds4K)
			array4K.PushBack(rapidjson::Value().SetInt(key), allocator);
		rapidjson::Value array5K(rapidjson::kArrayType);
		for (int& key : defaultKeybinds5K)
			array5K.PushBack(rapidjson::Value().SetInt(key), allocator);
		settings.AddMember("keybinds", rapidjson::Value(rapidjson::kObjectType), allocator);
		settings["keybinds"].AddMember("4k", array4K, allocator);
		settings["keybinds"].AddMember("5k", array5K, allocator);
		rapidjson::Value avOffset;
		avOffset.SetInt(0);
		settings.AddMember("avOffset", avOffset, allocator);
		rapidjson::Value inputOffset;
		inputOffset.SetInt(0);
		settings.AddMember("inputOffset", inputOffset, allocator);
		rapidjson::Value trackSpeedVal;
		trackSpeedVal.SetInt(4);
		settings.AddMember("trackSpeed", trackSpeedVal, allocator);
		rapidjson::Value arrayTrackSpeedOptions(rapidjson::kArrayType);
		for (float& speed : defaultTrackSpeedOptions)
			arrayTrackSpeedOptions.PushBack(rapidjson::Value().SetFloat(speed), allocator);
		settings.AddMember("trackSpeedOptions", arrayTrackSpeedOptions, allocator);
		

		saveSettings(settingsFile);
	}
	void loadSettings(std::filesystem::path settingsFile) {
		bool keybindsError = false;
		bool keybinds4KError = false;
		bool keybinds5KError = false;
		bool avError = false;
		bool inputError = false;
		bool trackSpeedOptionsError = false;
		bool trackSpeedError = false;
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
				}
				else {
					keybindsError = true;
				}

				if (settings.HasMember("avOffset") && settings["avOffset"].IsInt()) {
					const rapidjson::Value& offset = settings["avOffset"];
					VideoOffset = -(float)(offset.GetInt() / 1000);
				}
				else {
					avError = true;
				}
				if (settings.HasMember("inputOffset") && settings["inputOffset"].IsInt()) {
					const rapidjson::Value& offset = settings["inputOffset"];
					InputOffset = (float)(offset.GetInt() / 1000);
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
		if (keybindsError || keybinds4KError || keybinds5KError || avError || inputError|| trackSpeedError || trackSpeedOptionsError) {
			saveSettings(settingsFile);
		}
	}
	const void saveSettings(std::filesystem::path settingsFile) {
		char writeBuffer[8192];
		FILE* fp = fopen(settingsFile.string().c_str(), "wb");
		rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
		rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
		settings.Accept(writer);
		fclose(fp);
	}
};
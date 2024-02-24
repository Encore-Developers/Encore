#include "rapidjson/document.h"
#include "raylib.h"
#include "chart.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <filesystem>

enum PartIcon {
	Drum,
	Bass,
	Guitar,
	Vocals,
	Keyboard,
	None
};

std::unordered_map<std::string, PartIcon> stringToEnum = {
	{"Drum",PartIcon::Drum},
	{"Bass",PartIcon::Bass},
	{"Guitar",PartIcon::Guitar},
	{"Vocals",PartIcon::Vocals},
	{"Keyboard",PartIcon::Keyboard},
	{"None",PartIcon::None},
	{"",PartIcon::None}
};

// Function to convert string to enum
static PartIcon iconFromString(const std::string& str) {
	auto it = stringToEnum.find(str);
	if (it != stringToEnum.end()) {
		return it->second;
	}
	else {
		// Handle error: String doesn't match any enum value
		throw std::runtime_error("Invalid enum string");
	}
}

struct SongPart {
	int diff = -1;
	bool hasPart = false;
};
struct Song {
	std::string title;
	std::string artist;
	Texture albumArt;
	int length = 0;
	std::vector<PartIcon> partIcons{ PartIcon::None,PartIcon::None,PartIcon::None,PartIcon::None };
	std::vector<SongPart*> parts{ new SongPart,new SongPart,new SongPart,new SongPart ,new SongPart ,new SongPart ,new SongPart };
	bool is10Channel = false;
	std::vector<std::string> stemsPath{ "","","","","" };
	std::string audio10CPath = "";
	std::filesystem::path midiPath = "";
	void loadSong(std::filesystem::path jsonPath) {
		std::ifstream ifs(jsonPath);
		if (!ifs.is_open()) {
			std::cerr << "Failed to open JSON file." << std::endl;
		}

		std::string jsonString((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

		rapidjson::Document document;
		document.Parse(jsonString.c_str());
		if (document.IsObject()) {
			if (document.HasMember("title") && document["title"].IsString()) {
				title = document["title"].GetString();
			}
			if (document.HasMember("artist") && document["artist"].IsString()) {
				artist = document["artist"].GetString();
			}
			if (document.HasMember("length") && document["length"].IsInt()) {
				length = document["length"].GetInt();
			}
			if (document.HasMember("sid") && document["sid"].IsString()) {
				partIcons[0] = iconFromString(document["sid"].GetString());
			}
			if (document.HasMember("sib") && document["sib"].IsString()) {
				partIcons[1] = iconFromString(document["sib"].GetString());
			}
			if (document.HasMember("sig") && document["sig"].IsString()) {
				partIcons[2] = iconFromString(document["sig"].GetString());
			}
			if (document.HasMember("siv") && document["siv"].IsString()) {
				partIcons[3] = iconFromString(document["siv"].GetString());
			}
			if (document.HasMember("diff") && document["diff"].IsObject()) {
				for (rapidjson::Value::ConstMemberIterator itr = document["diff"].MemberBegin(); itr != document["diff"].MemberEnd(); ++itr) {
					if (itr->name.GetString() == "ds") {
						parts[0]->diff = itr->value.GetInt();
						parts[0]->hasPart = true;
					}
					else if (itr->name.GetString() == "ba") {
						parts[1]->diff = itr->value.GetInt();
						parts[1]->hasPart = true;
					}
					else if (itr->name.GetString() == "gr") {
						parts[2]->diff = itr->value.GetInt();
						parts[2]->hasPart = true;
					}
					else if (itr->name.GetString() == "vl") {
						parts[3]->diff = itr->value.GetInt();
						parts[3]->hasPart = true;
					}
					else if (itr->name.GetString() == "pd") {
						parts[4]->diff = itr->value.GetInt();
						parts[4]->hasPart = true;
					}
					else if (itr->name.GetString() == "pb") {
						parts[5]->diff = itr->value.GetInt();
						parts[5]->hasPart = true;
					}
					else if (itr->name.GetString() == "pg") {
						parts[6]->diff = itr->value.GetInt();
						parts[6]->hasPart = true;
					}
				}
			}
			if (document.HasMember("midi") && document["midi"].IsString()) {
				midiPath = jsonPath.parent_path() / document["midi"].GetString();
			}
			if (document.HasMember("stems") && document["stems"].IsObject()) {
				std::cout << "stems" << std::endl;
				for (rapidjson::Value::ConstMemberIterator itr = document["stems"].MemberBegin(); itr != document["stems"].MemberEnd(); ++itr) {
					std::string stem = std::string(itr->name.GetString());
					if (stem == "drums")
						stemsPath[0] = (jsonPath.parent_path() / itr->value.GetString()).string();
					else if (stem == "bass")
						stemsPath[1] = (jsonPath.parent_path() / itr->value.GetString()).string();
					else if (stem == "lead")
						stemsPath[2] = (jsonPath.parent_path() / itr->value.GetString()).string();
					else if (stem == "vocals")
						stemsPath[3] = (jsonPath.parent_path() / itr->value.GetString()).string();
					else if (stem == "backing")
						stemsPath[4] = (jsonPath.parent_path() / itr->value.GetString()).string();
					else if (stem == "10channel") {
						is10Channel = true;
						audio10CPath = (jsonPath.parent_path() / itr->value.GetString()).string();
					}
				}					
			}
			if (document.HasMember("art") && document["art"].IsString()) {
				std::string artPath = (jsonPath.parent_path() / document["art"].GetString()).string();
				Image albumImage = LoadImage(artPath.c_str());
				if (albumImage.height > 512) {
					ImageResize(&albumImage, 512, 512);
				}
				albumArt = LoadTextureFromImage(albumImage);
				UnloadImage(albumImage);
			}
		}
	}

};
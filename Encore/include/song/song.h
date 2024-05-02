#pragma once
#include "rapidjson/document.h"
#include "raylib.h"
#include "chart.h"
#include "midifile/MidiFile.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <filesystem>
#include <cmath>

enum PartIcon {
	IconDrum,
	IconBass,
	IconGuitar,
	IconVocals,
	IconKeyboard,
	IconNone
};




enum SongParts {
	PartDrums,
	PartBass,
	PartGuitar,
	PartVocals,
	PlasticDrums,
	PlasticBass,
	PlasticGuitar,
	Invalid
};




enum Difficulty {
	Easy,
	Medium,
	Hard,
	Expert
};


struct SongPart 
{
	int diff = -1;
	bool hasPart = false;
	std::vector<Chart> charts;
};

struct TimeSig {
	double time;
	int numer;
	int denom;
};
struct BPM {
	double time;
	double bpm;
};

class Song 
{
public:
    std::unordered_map<std::string, PartIcon> stringToEnum = {

            {"Drum",PartIcon::IconDrum},
            {"Bass",PartIcon::IconBass},
            {"Guitar",PartIcon::IconGuitar},
            {"Vocals",PartIcon::IconVocals},
            {"Keyboard",PartIcon::IconKeyboard},
            {"None",PartIcon::IconNone},
            {"",PartIcon::IconNone}

    };

    PartIcon iconFromString(const std::string& str)
    {
        auto it = stringToEnum.find(str);
        if (it != stringToEnum.end())
        {
            return it->second;
        }
        else
        {
            throw std::runtime_error("Invalid enum string");
        }
    }

    std::unordered_map<std::string, SongParts> midiNameToEnum = {
            {"PART DRUMS",SongParts::PartDrums},
            {"PART BASS",SongParts::PartBass},
            {"PART GUITAR",SongParts::PartGuitar},
            {"PART VOCALS",SongParts::PartVocals},
            {"PLASTIC DRUMS",SongParts::PlasticDrums},
            {"PLASTIC BASS",SongParts::PlasticBass},
            {"PLASTIC GUITAR",SongParts::PlasticGuitar}
    };

    SongParts partFromString(const std::string& str)
    {
        auto it = midiNameToEnum.find(str);
        if (it != midiNameToEnum.end())
        {
            return it->second;
        }
        else
        {
            return SongParts::Invalid;
        }
    }

	bool midiParsed=false;
	std::string title;
	float titleXOffset = 0;
	float titleTextWidth = 0;
	double titleScrollTime = 0.0;
	std::string artist;
	float artistXOffset = 0;
	float artistTextWidth = 0;
	double artistScrollTime = 0.0;
    Texture albumArtBlur;
	Texture albumArt;

	int length = 0;
	
	std::vector<BPM> bpms;
	std::vector<TimeSig> timesigs;

	double music_start=0.0;
	double end=0.0;
	std::vector<PartIcon> partIcons{ PartIcon::IconNone,PartIcon::IconNone,PartIcon::IconNone,PartIcon::IconNone };
	//Parts order will always be Drums, Bass, Guitar, Vocals, Plastic Drums, Plastic Bass, Plastic Guitar
	std::vector<SongPart*> parts{ new SongPart,new SongPart,new SongPart,new SongPart ,new SongPart ,new SongPart ,new SongPart };

	std::vector<std::pair<double, bool>> beatLines; //double time, bool downbeat

	std::vector<std::pair<std::string, int>> stemsPath{ {"",0},{"",1},{"",2},{"",3},{"",4} };

	std::filesystem::path midiPath = "";

	void LoadSong(std::filesystem::path jsonPath) 
	{
		std::ifstream ifs(jsonPath);

		if (!ifs.is_open()) {
			std::cerr << "Failed to open JSON file." << std::endl;
		}

		std::string jsonString((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		
		rapidjson::Document document;
		document.Parse(jsonString.c_str());

		if (document.IsObject()) 
		{
			if (document.HasMember("title") && document["title"].IsString()) 
			{
				title = document["title"].GetString();
			}

			if (document.HasMember("artist") && document["artist"].IsString()) 
			{
				artist = document["artist"].GetString();
			}

			if (document.HasMember("length") && document["length"].IsInt()) 
			{
				length = document["length"].GetInt();
			}

			if (document.HasMember("sid") && document["sid"].IsString()) 
			{
				partIcons[0] = iconFromString(document["sid"].GetString());
			}

			if (document.HasMember("sib") && document["sib"].IsString()) 
			{
				partIcons[1] = iconFromString(document["sib"].GetString());
			}

			if (document.HasMember("sig") && document["sig"].IsString()) 
			{
				partIcons[2] = iconFromString(document["sig"].GetString());
			}

			if (document.HasMember("siv") && document["siv"].IsString()) 
			{
				partIcons[3] = iconFromString(document["siv"].GetString());
			}

			if (document.HasMember("diff") && document["diff"].IsObject()) 
			{
				for (rapidjson::Value::ConstMemberIterator itr = document["diff"].MemberBegin(); itr != document["diff"].MemberEnd(); ++itr) 
				{
					std::string part = std::string(itr->name.GetString());
					if (part == "ds")
					{
						parts[0]->diff = itr->value.GetInt();
					}

					else if (part == "ba")
					{
						parts[1]->diff = itr->value.GetInt();
					}

					else if (part == "gr")
					{
						parts[2]->diff = itr->value.GetInt();
					}

					else if (part == "vl")
					{
						parts[3]->diff = itr->value.GetInt();
					}

					else if (part == "pd")
					{
						parts[4]->diff = itr->value.GetInt();
					}

					else if (part == "pb")
					{
						parts[5]->diff = itr->value.GetInt();
					}

					else if (part == "pg")
					{
						parts[6]->diff = itr->value.GetInt();
					}
				}
			}

			if (document.HasMember("midi") && document["midi"].IsString()) 
			{
				midiPath = jsonPath.parent_path() / document["midi"].GetString();
			}

			if (document.HasMember("stems") && document["stems"].IsObject()) 
			{
				for (rapidjson::Value::ConstMemberIterator itr = document["stems"].MemberBegin(); itr != document["stems"].MemberEnd(); ++itr) 
				{
					std::string stem = std::string(itr->name.GetString());

					if (std::filesystem::exists(jsonPath.parent_path() / itr->value.GetString())) 
					{
						if (stem == "drums")
							stemsPath[0].first = (jsonPath.parent_path() / itr->value.GetString()).string();

						else if (stem == "bass")
							stemsPath[1].first = (jsonPath.parent_path() / itr->value.GetString()).string();

						else if (stem == "lead")
							stemsPath[2].first = (jsonPath.parent_path() / itr->value.GetString()).string();

						else if (stem == "vocals")
							stemsPath[3].first = (jsonPath.parent_path() / itr->value.GetString()).string();

						else if (stem == "backing")
							stemsPath[4].first = (jsonPath.parent_path() / itr->value.GetString()).string();
					}
				}					
			}

			if (document.HasMember("art") && document["art"].IsString()) 
			{
				std::string artPath = (jsonPath.parent_path() / document["art"].GetString()).string();
				Image albumImage = LoadImage(artPath.c_str());

				if (albumImage.height > 512) {
                    ImageResize(&albumImage, 512, 512);
                }
				albumArt = LoadTextureFromImage(albumImage);
                SetTextureFilter(albumArt, TEXTURE_FILTER_ANISOTROPIC_16X);
                ImageBlurGaussian(&albumImage, 10);
                albumArtBlur = LoadTextureFromImage(albumImage);
				UnloadImage(albumImage);
			}
		}
		ifs.close();
	}
	void parseBeatLines(smf::MidiFile& midiFile, int trkidx, smf::MidiEventList events) {
		for (int i = 0; i < events.getSize(); i++) {
			if (events[i].isNoteOn()) {
				beatLines.push_back({ midiFile.getTimeInSeconds(trkidx, i) , (int)events[i][1] == 12 });
			}
		}
	}
	void getTiming(smf::MidiFile& midiFile, int trkidx, smf::MidiEventList events) {
		for (int i = 0; i < events.getSize(); i++) {
			if (events[i].isTempo()) {
				bpms.push_back({ midiFile.getTimeInSeconds(trkidx, i) , events[i].getTempoBPM() });
				std::cout << "BPM @" << midiFile.getTimeInSeconds(trkidx, i) << ": " << events[i].getTempoBPM() << std::endl;
			}
			else if (events[i].isMeta() && events[i][1] == 0x58) {
				int numer = (int)events[i][3];
				int denom = pow(2,(int)events[i][4]);
				timesigs.push_back({ midiFile.getTimeInSeconds(trkidx, i), numer,denom });
				std::cout << "TIMESIG @" << midiFile.getTimeInSeconds(trkidx, i) << ": " << numer <<"/"<<denom<< std::endl;
			}
		}
		if (timesigs.size() == 0) {
			timesigs.push_back({ 4,4 }); //midi always assumed to be 4/4 if time sig event isn't found
		}
	}

	void getStartEnd(smf::MidiFile& midiFile, int trkidx, smf::MidiEventList events) {
		for (int i = 0; i < events.getSize(); i++) {
			if (events[i].isMeta() && (int)events[i][1] == 1) {
				double time = midiFile.getTimeInSeconds(trkidx, i);
				std::string evt_string = "";
				for (int k = 3; k < events[i].getSize(); k++) {
					evt_string += events[i][k];
				}
				std::cout << evt_string << std::endl;
				if (evt_string == "[music_start]")
					music_start = time;
				else if (evt_string == "[end]")
					end = time;
			}
		}
	}
};
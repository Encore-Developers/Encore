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
	std::string title = "";
	float titleXOffset = 0;
	float titleTextWidth = 0;
	double titleScrollTime = 0.0;
	std::string artist = "";
	float artistXOffset = 0;
	float artistTextWidth = 0;
	double artistScrollTime = 0.0;
    Texture albumArtBlur;
	Texture albumArt;
	std::string album = "";
	int length = 0;
	
	std::vector<BPM> bpms{};
	std::vector<TimeSig> timesigs{};

	double music_start=0.0;
	double end=0.0;
	std::vector<PartIcon> partIcons{ PartIcon::IconNone,PartIcon::IconNone,PartIcon::IconNone,PartIcon::IconNone };
	//Parts order will always be Drums, Bass, Guitar, Vocals, Plastic Drums, Plastic Bass, Plastic Guitar
	std::vector<SongPart*> parts{ new SongPart,new SongPart,new SongPart,new SongPart ,new SongPart ,new SongPart ,new SongPart };

	std::vector<std::pair<double, bool>> beatLines; //double time, bool downbeat

	std::vector<std::pair<std::string, int>> stemsPath{};

	std::filesystem::path midiPath = "";

    std::string songDir = "";
    std::string albumArtPath = "";
    std::string songInfoPath = "";
	int releaseYear = 0;
	std::string loadingPhrase = "";
	std::vector<std::string> charters{};



	void LoadSong(std::filesystem::path jsonPath) 
	{
		std::ifstream ifs(jsonPath);

		if (!ifs.is_open()) {
			std::cerr << "Failed to open JSON file." << std::endl;
		}

		std::string jsonString((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		
		rapidjson::Document document;
		document.Parse(jsonString.c_str());
        songInfoPath = jsonPath.string();
        songDir = jsonPath.parent_path().string();
		if (document.IsObject()) 
		{
			for (auto& item : document.GetObject()) {
				if (item.name == "title" && item.value.IsString())
					title = item.value.GetString();
				if (item.name == "artist" && item.value.IsString())
					artist = item.value.GetString();
				if (item.name == "album" && item.value.IsString())
					album = item.value.GetString();
				if (item.name == "length" && item.value.IsInt())
					length = item.value.GetInt();
				if (item.name == "release_year" && item.value.IsInt())
					releaseYear = item.value.GetInt();
				if (item.name == "loading_phrase" && item.value.IsString())
					loadingPhrase = item.value.GetString();
				if ((item.name=="sid" || item.name=="icon_drums") && item.value.IsString())
					partIcons[0] = iconFromString(item.value.GetString());
				if ((item.name == "sib" || item.name == "icon_bass") && item.value.IsString())
					partIcons[1] = iconFromString(item.value.GetString());
				if ((item.name == "sig" || item.name == "icon_guitar") && item.value.IsString())
					partIcons[2] = iconFromString(item.value.GetString());
				if ((item.name == "siv" || item.name == "icon_vocals") && item.value.IsString())
					partIcons[3] = iconFromString(item.value.GetString());
				if (item.name == "midi" && item.value.IsString())
					midiPath = jsonPath.parent_path() / item.value.GetString();
				if (item.name == "art" && item.value.IsString()) {
					albumArtPath = (jsonPath.parent_path() / item.value.GetString()).string();
				}
				if (item.name=="diff" && item.value.IsObject())
				{
					for (auto& diff:item.value.GetObject())
					{
						std::string part = std::string(diff.name.GetString());
						if (diff.value.IsInt()) {
							if (part == "ds" || part == "drums")
							{
								parts[0]->diff = diff.value.GetInt();
							}
							else if (part == "ba" || part == "bass")
							{
								parts[1]->diff = diff.value.GetInt();
							}
							else if (part == "gr" || part == "guitar")
							{
								parts[2]->diff = diff.value.GetInt();
							}
							else if (part == "vl" || part == "vocals")
							{
								parts[3]->diff = diff.value.GetInt();
							}
							else if (part == "pd" || part == "plastic_drums")
							{
								parts[4]->diff = diff.value.GetInt();
							}
							else if (part == "pb" || part == "plastic_bass")
							{
								parts[5]->diff = diff.value.GetInt();
							}
							else if (part == "pg" || part == "plastic_guitar")
							{
								parts[6]->diff = diff.value.GetInt();
							}
						}
					}
				}
				if (item.name=="stems" && item.value.IsObject())
				{
					for (auto &path : item.value.GetObject())
					{
						std::string stem = std::string(path.name.GetString());
						if (path.value.IsString()) {
							if (std::filesystem::exists(jsonPath.parent_path() / path.value.GetString()))
							{
								if (stem == "drums")
									stemsPath.push_back({ (jsonPath.parent_path() / path.value.GetString()).string(),0 });

								else if (stem == "bass")
									stemsPath.push_back({ (jsonPath.parent_path() / path.value.GetString()).string(),1 });

								else if (stem == "lead")
									stemsPath.push_back({ (jsonPath.parent_path() / path.value.GetString()).string(),2 });

								else if (stem == "vocals")
									stemsPath.push_back({ (jsonPath.parent_path() / path.value.GetString()).string(),3 });

								else if (stem == "backing")
									stemsPath.push_back({ (jsonPath.parent_path() / path.value.GetString()).string(),4 });
							}
						}
						else if (path.value.IsArray()) {
							for (auto& path : path.value.GetArray()) {
								if (std::filesystem::exists(jsonPath.parent_path() / path.GetString()))
								{
									if (stem == "drums")
										stemsPath.push_back({ (jsonPath.parent_path() / path.GetString()).string(),0 });

									else if (stem == "bass")
										stemsPath.push_back({ (jsonPath.parent_path() / path.GetString()).string(),1 });

									else if (stem == "lead")
										stemsPath.push_back({ (jsonPath.parent_path() / path.GetString()).string(),2 });

									else if (stem == "vocals")
										stemsPath.push_back({ (jsonPath.parent_path() / path.GetString()).string(),3 });

									else if (stem == "backing")
										stemsPath.push_back({ (jsonPath.parent_path() / path.GetString()).string(),4 });
								}
							}
						}
					}
				}
				if (item.name == "charters" && item.value.IsObject()) {
					for(auto& charter:item.value.GetObject()){
						if (charter.value.IsString()) {
							charters.push_back(charter.value.GetString());
						}
					}
				}
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
				std::cout << evt_string << events[i].tick << std::endl;
				if (evt_string == "[music_start]")
					music_start = time;
				else if (evt_string == "[end]")
					end = time;
			}
		}
	}

    void LoadAlbumArt(std::string artpath) {
        Image albumImage = LoadImage(artpath.c_str());
        if (albumImage.height > 512) {
            ImageResize(&albumImage, 512, 512);
        }
        albumArt = LoadTextureFromImage(albumImage);
        GenTextureMipmaps(&albumArt);
        SetTextureFilter(albumArt, TEXTURE_FILTER_TRILINEAR);

        ImageBlurGaussian(&albumImage, 10);
        albumArtBlur = LoadTextureFromImage(albumImage);
        GenTextureMipmaps(&albumArtBlur);
        SetTextureFilter(albumArtBlur, TEXTURE_FILTER_TRILINEAR);
        UnloadImage(albumImage);
    };
};
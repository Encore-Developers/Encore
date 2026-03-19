#pragma once

#ifndef ENCORE_SONG_H
#define ENCORE_SONG_H

#include "raylib.h"
#include "chart.h"
#include "midifile/MidiFile.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <filesystem>
#include <cmath>
#include <string>
#include <atomic>
#include "picosha2.h"
#include "inih/INIReader.h"
#include "util/enclog.h"

#include <array>
#include <nlohmann/json.hpp>

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
    PartKeys,
    PartVocals,
    PlasticDrums,
    PlasticBass,
    PlasticGuitar,
    PlasticKeys,
    PlasticVocals,
    PitchedVocals,
    Invalid,
    BeatLines,
    Events
};

enum Difficulty {
    Easy,
    Medium,
    Hard,
    Expert
};

struct SongPart {
    int diff = -1;
    int TrackInt = -1;
    bool Valid = false;
    std::array<bool, 4> ValidDiffs{ false, false, false, false };
};

struct Beat {
    double Time;
    bool Major = false;
    bool Clapped = false;
    int Tick;
};

static std::atomic_int LoadingState = -1;

inline std::array<std::string, 4> diffList = { "Easy", "Medium", "Hard", "Expert" };
inline std::vector<std::string> songPartsList{
    "Drums", "Bass", "Lead", "Keys",
    "Vocals", "Classic Drums", "Classic Bass", "Classic Lead",
    "Classic Keys", "Classic Vocals", "Classic Vocals",
};

inline std::unordered_map<std::string, PartIcon> stringToEnum = {

    { "Drum", PartIcon::IconDrum },
    { "Bass", PartIcon::IconBass },
    { "Guitar", PartIcon::IconGuitar },
    { "Vocals", PartIcon::IconVocals },
    { "Keyboard", PartIcon::IconKeyboard },
    { "None", PartIcon::IconNone },
    { "", PartIcon::IconNone }

};

inline PartIcon iconFromString(const std::string &str) {
    auto it = stringToEnum.find(str);
    if (it != stringToEnum.end()) {
        return it->second;
    } else {
        throw std::runtime_error("Invalid enum string");
    }
}

inline std::unordered_map<std::string, SongParts> midiNameToEnum = {
    { "PART DRUMS", SongParts::PartDrums },
    { "PART BASS", SongParts::PartBass },
    { "PART GUITAR", SongParts::PartGuitar },
    { "PART VOCALS", SongParts::PartVocals },
    { "PART KEYS", SongParts::PartKeys },
    { "PLASTIC DRUMS", SongParts::PlasticDrums },
    { "PLASTIC BASS", SongParts::PlasticBass },
    { "PLASTIC GUITAR", SongParts::PlasticGuitar },
    { "PLASTIC VOCALS", SongParts::PlasticVocals },
    { "PLASTIC KEYS", SongParts::PlasticKeys },
    { "PITCHED VOCALS", SongParts::Invalid },
    { "BEAT", SongParts::BeatLines },
    { "EVENTS", SongParts::Events }
};

inline std::unordered_map<std::string, SongParts> midiNameToEnumINI = {
    { "PAD DRUMS", SongParts::PartDrums }, { "PAD BASS", SongParts::PartBass },
    { "PAD GUITAR", SongParts::PartGuitar }, { "PAD VOCALS", SongParts::PartVocals },
    { "PAD KEYS", SongParts::PartKeys }, { "PART DRUMS", SongParts::PlasticDrums },
    { "PART BASS", SongParts::PlasticBass }, { "PART GUITAR", SongParts::PlasticGuitar },
    { "PART VOCALS", SongParts::Invalid }, { "PART KEYS", SongParts::PlasticKeys },
    { "PLASTIC VOCALS", SongParts::Invalid }, { "BEAT", SongParts::BeatLines },
    { "EVENTS", SongParts::Events }
};

inline std::unordered_map<std::string, SongParts> stemEnumToPart = {
    { "drums", SongParts::PartDrums },
    { "bass", SongParts::PartBass },
    { "lead", SongParts::PartGuitar },
    { "vocals", SongParts::PartVocals },
    { "backing", SongParts::Invalid },
    { "keys", SongParts::PartKeys }
};

inline std::vector<int> PlasticToPadEnumConverter = { PartDrums, PartBass, PartGuitar,
                                                      PartVocals, PartKeys, PartDrums,
                                                      PartBass, PartGuitar, PartVocals,
                                                      PartKeys, PartVocals, Invalid };

inline SongParts partFromString(const std::string &str) {
    auto it = midiNameToEnum.find(str);
    if (it != midiNameToEnum.end()) {
        return it->second;
    } else {
        return SongParts::Invalid;
    }
}

inline SongParts partFromStringINI(const std::string &str) {
    auto it = midiNameToEnumINI.find(str);
    if (it != midiNameToEnumINI.end()) {
        return it->second;
    } else {
        return SongParts::Invalid;
    }
}

class Song {
public:
    bool midiParsed = false;
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
    std::string source = "custom";
    std::string album = "";
    int length = 0;
    int songListPos = 0;
    int BeatTrackID = 0;

    bool AlbumArtLoaded = false;
    double music_start = 0.0;
    double end = 0.0;
    std::vector<PartIcon> partIcons{
        PartIcon::IconNone, PartIcon::IconNone, PartIcon::IconNone, PartIcon::IconNone
    };
    // Parts order will always be Drums, Bass, Guitar, Vocals, Plastic Drums, Plastic
    // Bass, Plastic Guitar
    std::vector<SongPart *> parts{ new SongPart, new SongPart, new SongPart,
                                   new SongPart, new SongPart, new SongPart,
                                   new SongPart, new SongPart, new SongPart,
                                   new SongPart, new SongPart };

    std::vector<Beat> beatLines; // double time, bool downbeat

    std::vector<std::pair<std::string, int>> stemsPath{};

    std::filesystem::path midiPath = "";

    std::string songDir = "";
    std::string albumArtPath = "";
    std::filesystem::path songInfoPath = "";
    std::string releaseYear = "";
    std::string loadingPhrase = "";
    std::vector<std::string> charters{};
    std::string jsonHash = "";
    int hopoThreshold = -1;
    bool ini = false;
    smf::MidiFile midiFile;
    void LoadAudioINI(std::filesystem::path songPath);
    float previewStartTime = 0.0f;

    SongParts GetSongPart(smf::MidiEventList track) {
        for (int events = 0; events < track.getSize(); events++) {
            std::string trackName;
            if (!track[events].isMeta())
                continue;
            if ((int)track[events][1] == 3) {
                for (int k = 3; k < track[events].getSize(); k++) {
                    trackName += track[events][k];
                }
                if (ini)
                    return partFromStringINI(trackName);
                return partFromString(trackName);
            }
        }
        return Invalid;
    }

    std::vector<std::vector<int> > pDiffRangeNotes = {
        { 60, 64 }, { 72, 76 }, { 84, 88 }, { 96, 100 }
    };

    void IsPartValid(smf::MidiEventList track, SongParts songPart, int trackNumber) {
        if (songPart == Invalid || songPart == PitchedVocals || songPart == BeatLines) {
            return;
        }
        for (int diff = 0; diff < 4; diff++) {
            bool StopSearching = false;

            for (int i = 0; i < track.getSize(); i++) {
                if (track[i].isNoteOn() && !track[i].isMeta()
                    && track[i][1] >= pDiffRangeNotes[diff][0]
                    && track[i][1] <= pDiffRangeNotes[diff][1] && !StopSearching) {
                    parts[songPart]->ValidDiffs.at(diff) = true;
                    parts[songPart]->TrackInt = trackNumber;
                    parts[songPart]->Valid = true;
                    StopSearching = true;
                    }
                if (StopSearching)
                    break;
            }
        }
    }


    void LoadAudioJSON(const nlohmann::json &jsonPath) {
        if (!stemsPath.empty())
            stemsPath.clear();

        for (auto & [key, val] : jsonPath.at("stems").items()) {
            stemsPath.emplace_back(
                (songInfoPath.parent_path() / val).string(),
                midiNameToEnum[key]);
        }
    }

    void LoadInfoINI(std::filesystem::path iniPath);
    void PullInfoFromINI(INIReader &ini);

    void LoadInfoJSON(const nlohmann::json &infoData) {

        if (!charters.empty())
            charters.clear();

        title = infoData.value<std::string>("title", "Unknown song");
        artist = infoData.value<std::string>("artist", "Unknown artist");
        album = infoData.value<std::string>("album", "Unknown album");
        source = infoData.value<std::string>("source", "Unknown source");
        length = infoData.value<int>("length", 0);
        releaseYear = infoData.value("release_year", "");
        loadingPhrase = infoData.value<std::string>("loading_phrase", "");
        midiPath =
            (std::filesystem::path(songDir) /
                infoData.value<std::string>("midi", "")).string();

        try {
            std::string charter = infoData.value<std::string>("charters", "Unknown chart author");
            charters.push_back(charter);
        } catch (const std::exception &e) {
            charters.push_back("Unknown chart author");
        }
    }

    void LoadSongIni(std::filesystem::path songPath);

    using json = nlohmann::json;

    void LoadSongJSON(std::filesystem::path jsonPath) {
        std::ifstream infoFile(jsonPath);
        json infoData = json::parse(infoFile);
        infoFile.close();

        LoadInfoJSON(infoData);
        LoadAudioJSON(infoData);
    }

    void parseBeatLines(smf::MidiFile &midiFile, int trkidx) {
        int MaxTick = midiFile[trkidx].last().tick;
        for (int i = 0; i < MaxTick; i += 240) {
            beatLines.push_back({ midiFile.getTimeInSeconds(i), false, false, i });
        }

        /*
        for (int i = 0; i < midiFile[trkidx].getSize(); i++) {
            if (midiFile[trkidx][i].isNoteOn()) {
                beatLines.push_back(
                    { midiFile.getTimeInSeconds(trkidx, i),
                      (int)midiFile[trkidx][i][1] == 12,
                      false,
                      midiFile[trkidx][i].tick }
                );
            }
        }
        */
    }

    /*
    void getTiming(smf::MidiFile &midiFile, int trkidx, smf::MidiEventList events) {
        for (int i = 0; i < events.getSize(); i++) {
            if (events[i].isTempo()) {
                bpms.push_back(
                    { midiFile.getTimeInSeconds(trkidx, i),
                      events[i].getTempoBPM(),
                      events[i].tick }
                );
                // std::cout << "BPM @" << midiFile.getTimeInSeconds(trkidx, i) << ": "
                //           << events[i].getTempoBPM() << std::endl;
            } else if (events[i].isMeta() && events[i][1] == 0x58) {
                int numer = (int)events[i][3];
                int denom = pow(2, (int)events[i][4]);
                timesigs.push_back({ midiFile.getTimeInSeconds(trkidx, i), numer, denom
    });
                // std::cout << "TIMESIG @" << midiFile.getTimeInSeconds(trkidx, i) << ":
                // "
                //           << numer << "/" << denom << std::endl;
            }
        }
        if (timesigs.size() == 0) {
            timesigs.push_back({ 4, 4 }); // midi always assumed to be 4/4 if time sig
                                          // event isn't found
        }
    }
*/
    int endTick = 0;

    void getStartEnd(smf::MidiFile &midiFile, int trkidx, smf::MidiEventList events) {
        for (int i = 0; i < events.getSize(); i++) {
            if (events[i].isMeta() && (int)events[i][1] == 1) {
                double time = midiFile.getTimeInSeconds(trkidx, i);
                std::string evt_string = "";
                for (int k = 3; k < events[i].getSize(); k++) {
                    evt_string += events[i][k];
                }

                if (evt_string == "[music_start]") {
                    music_start = time;
                    Encore::EncoreLog(
                        LOG_DEBUG,
                        TextFormat("SONG: Song start: %5.4f", time)
                    );
                }
                if (evt_string == "[end]") {
                    end = time;
                    endTick = events[i].tick;
                    Encore::EncoreLog(
                        LOG_DEBUG,
                        TextFormat("SONG: Song end: %5.4f", time)
                    );
                }
            }
        }
    }

    // Coda BRE {};
    void getCodas(smf::MidiFile &midiFile) {
        int codaCount = 0;
        for (int track = 0; track < midiFile.getTrackCount(); track++) {
            std::string trackName;
            for (int events = 0; events < midiFile[track].getSize(); events++) {
                if (midiFile[track][events].isMeta()) {
                    if ((int)midiFile[track][events][1] == 3) {
                        for (int k = 3; k < midiFile[track][events].getSize(); k++) {
                            trackName += midiFile[track][events][k];
                        }
                        SongParts songPart;
                        if (ini) {
                            songPart = partFromStringINI(trackName);
                        } else
                            songPart = partFromString(trackName);
                        if (songPart > PlasticDrums && songPart <= PlasticGuitar) {
                            int codaNote = 120; // i dont wanna bother with checking all
                            // five lanes
                            for (int i = 0; i < midiFile[track].getSize(); i++) {
                                if (midiFile[track][i].isNoteOn()
                                    && !midiFile[track][i].isMeta()
                                    && (int)midiFile[track][i][1] == codaNote) {
                                    // if (BRE.StartSec == 0.0) {
                                    //     BRE.StartSec = midiFile.getTimeInSeconds(
                                    //         midiFile[track][i].tick
                                    //     );
                                    //     BRE.StartTick = midiFile[track][i].tick;
                                    //    Encore::EncoreLog(LOG_DEBUG, "BRE start found");
                                    //}
                                }
                                if (midiFile[track][i].isNoteOff()
                                    && !midiFile[track][i].isMeta()
                                    && (int)midiFile[track][i][1] == codaNote) {
                                    // if (BRE.EndSec == 0.0) {
                                    //     BRE.EndSec = midiFile.getTimeInSeconds(
                                    //         midiFile[track][i].tick
                                    //     );
                                    //    BRE.EndTick = midiFile[track][i].tick;
                                    //    Encore::EncoreLog(LOG_DEBUG, "BRE end found");
                                    //    codaCount++;
                                    //}
                                }
                            }
                        }
                    }
                }
            }
        }
        // if (BRE.EndSec != 0.0 && BRE.StartSec != 0.0) {
        //     Encore::EncoreLog(LOG_DEBUG, "BRE valid");
        //     BRE.exists = true;
        // }
    }

    void LoadAlbumArt() {
        Image albumImage = LoadImage(albumArtPath.c_str());
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



#endif // ENCORE_SONG_H
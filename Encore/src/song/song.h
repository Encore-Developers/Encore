#pragma once

#ifndef ENCORE_SONG_H
#define ENCORE_SONG_H

#include "picosha2.h"
#include "raylib.h"
#include "midifile/MidiFile.h"
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <string>
#include <atomic>
#include "inih/INIReader.h"
#include "tracy/Tracy.hpp"
#include "util/enclog.h"

#include <array>
#include <nlohmann/json.hpp>

#include "audio.h"


enum PartIcon {
    IconDrum,
    IconBass,
    IconGuitar,
    IconVocals,
    IconKeyboard,
    IconNone
};

enum SongParts : int {
    PartDrums = 0,
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
    bool AutoToPad = false;
};

struct Beat {
    double Time;
    bool Major = false;
    bool Clapped = false;
    int Tick;
};

static std::atomic_int LoadingState = -1;

inline std::array<std::string, 4> diffList = { "diff.easy", "diff.medium", "diff.hard", "diff.expert" };
inline std::vector<std::string> songPartsList{
    "parts.drums", "parts.bass", "parts.lead", "parts.keys",
    "parts.vocals", "parts.classicDrums", "parts.classicBass", "parts.classicLead",
    "parts.classicKeys", "parts.classicVocals", "parts.classicVocals",
};


inline std::unordered_map<std::string, SongParts> midiNameToEnumINI = {
    { "PAD DRUMS", SongParts::PartDrums }, { "PAD BASS", SongParts::PartBass },
    { "PAD GUITAR", SongParts::PartGuitar }, { "PAD VOCALS", SongParts::PartVocals },
    { "PAD KEYS", SongParts::PartKeys }, { "PART DRUMS", SongParts::PlasticDrums },
    { "PART BASS", SongParts::PlasticBass }, { "PART GUITAR", SongParts::PlasticGuitar },
    { "PART VOCALS", SongParts::PitchedVocals }, { "PART KEYS", SongParts::PlasticKeys },
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

inline SongParts partFromStringINI(const std::string &str) {
    auto it = midiNameToEnumINI.find(str);
    if (it != midiNameToEnumINI.end()) {
        return it->second;
    } else {
        return SongParts::Invalid;
    }
}

// todo: split drums into getting individual stems for proper drum mappings
inline std::map<SongParts, Encore::AudioManager::Stems> InstrumentToStemEnum {
        { PartDrums, AUDIOSTEM(Drums1) },
        { PartDrums, AUDIOSTEM(Drums2) },
        { PartDrums, AUDIOSTEM(Drums3) },
        { PartDrums, AUDIOSTEM(Drums4) },
        { PartBass, AUDIOSTEM(Bass) },
        { PartGuitar, AUDIOSTEM(Guitar) },
        { PartVocals, AUDIOSTEM(Vocals) },
        { PartKeys, AUDIOSTEM(Keys) },
};

inline Encore::AudioManager::Stems GetStemFromInstrument(SongParts part) {
    int fuck = part > PartVocals ? part - PlasticDrums : part;
    // I AM A PROGRAMMER! I AM A PROGRAMMER!
    return InstrumentToStemEnum.at(SongParts(fuck));
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
    std::string source = "custom";
    std::string album = "";
    int length = 0;
    int songListPos = 0;
    int BeatTrackID = 0;
    double music_start = 0.0;
    double end = 0.0;
    unsigned char chartHash[picosha2::k_digest_size];
    std::vector<PartIcon> partIcons{
        PartIcon::IconNone, PartIcon::IconNone, PartIcon::IconNone, PartIcon::IconNone
    };
    // Parts order will always be Drums, Bass, Guitar, Vocals, Plastic Drums, Plastic
    // Bass, Plastic Guitar
    std::vector<SongPart> parts { SongPart(), SongPart(), SongPart(),
                                   SongPart(), SongPart(), SongPart(),
                                   SongPart(), SongPart(), SongPart(),
                                   SongPart(), SongPart() };

    std::vector<Beat> beatLines; // double time, bool downbeat

    std::filesystem::path midiPath = "";

    std::filesystem::path songDir = "";
    std::filesystem::path albumArtPath = "";
    std::filesystem::path songInfoPath = "";
    std::string releaseYear = "";
    std::string loadingPhrase = "";
    std::vector<std::string> charters{};
    std::string jsonHash = "";
    int hopoThreshold = -1;
    smf::MidiFile midiFile;
    std::vector<std::pair<std::filesystem::path, Encore::AudioManager::Stems>> LoadAudioINI();
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
                return partFromStringINI(trackName);
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
            for (int i = 0; i < track.getSize(); i++) {
                if (track[i].isNoteOn() && !track[i].isMeta()
                    && track[i][1] >= pDiffRangeNotes[diff][0]
                    && track[i][1] <= pDiffRangeNotes[diff][1]) {
                    parts[songPart].ValidDiffs.at(diff) = true;
                    parts[songPart].TrackInt = trackNumber;
                    parts[songPart].Valid = true;
                    break;
                    }
            }
        }
    }


    void LoadInfoINI(std::filesystem::path iniPath);
    void PullInfoFromINI(INIReader &ini);

    void LoadSongIni(const std::filesystem::path& songPath);

    using json = nlohmann::json;

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
        if (endTick == 0) endTick = midiFile.getFileDurationInTicks();
        if (end == 0) end = midiFile.getFileDurationInSeconds();
    }

    // Coda BRE {};
    void getCodas(smf::MidiFile &midiFile) {
        // int codaCount = 0;
        for (int track = 0; track < midiFile.getTrackCount(); track++) {
            std::string trackName;
            for (int events = 0; events < midiFile[track].getSize(); events++) {
                if (midiFile[track][events].isMeta()) {
                    if ((int)midiFile[track][events][1] == 3) {
                        for (int k = 3; k < midiFile[track][events].getSize(); k++) {
                            trackName += midiFile[track][events][k];
                        }
                        SongParts songPart;
                        songPart = partFromStringINI(trackName);
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
                                    //    BRE.TickLength = midiFile[track][i].tick;
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

    void LoadAlbumArt();
};



#endif // ENCORE_SONG_H
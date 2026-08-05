#pragma once
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

#include "assets.h"
#include "audio.h"


enum PartIcon {
    IconDrum,
    IconBass,
    IconGuitar,
    IconVocals,
    IconKeyboard,
    IconNone
};

enum SongPart : int {
    PartDrums = 0,
    PartBass,
    PartGuitar,
    PartKeys,
    PartVocals,
    PlasticDrums,
    PlasticBass,
    PlasticGuitar,
    PlasticKeys,
    PitchedVocals,
    PlasticVocals,
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


inline std::unordered_map<std::string, SongPart> midiNameToEnumINI = {
    { "PAD DRUMS", PartDrums }, { "PAD BASS", PartBass },
    { "PAD GUITAR", PartGuitar }, { "PAD VOCALS", PartVocals },
    { "PAD KEYS", PartKeys }, { "PART DRUMS", PlasticDrums },
    { "PART BASS", PlasticBass }, { "PART GUITAR", PlasticGuitar },
    { "PART VOCALS", PitchedVocals }, { "PART KEYS", PlasticKeys },
    { "PLASTIC VOCALS", Invalid }, { "BEAT", BeatLines },
    { "EVENTS", Events }
};

inline std::vector<int> PlasticToPadEnumConverter = { PartDrums, PartBass, PartGuitar,
                                                      PartVocals, PartKeys, PartDrums,
                                                      PartBass, PartGuitar, PartVocals,
                                                      PartKeys, PartVocals, Invalid };

inline SongPart partFromStringINI(const std::string &str) {
    auto it = midiNameToEnumINI.find(str);
    if (it != midiNameToEnumINI.end()) {
        return it->second;
    } else {
        return SongPart::Invalid;
    }
}

// todo: split drums into getting individual stems for proper drum mappings
inline std::map<SongPart, Encore::AudioManager::Stems> InstrumentToStemEnum {
        { PartDrums, AUDIOSTEM(Drums1) },
        { PartDrums, AUDIOSTEM(Drums2) },
        { PartDrums, AUDIOSTEM(Drums3) },
        { PartDrums, AUDIOSTEM(Drums4) },
        { PartBass, AUDIOSTEM(Bass) },
        { PartGuitar, AUDIOSTEM(Guitar) },
        { PartVocals, AUDIOSTEM(Vocals) },
        { PartKeys, AUDIOSTEM(Keys) },
};

inline Encore::AudioManager::Stems GetStemFromInstrument(SongPart part) {
    int fuck = part > PartVocals ? part - PlasticDrums : part;
    // I AM A PROGRAMMER! I AM A PROGRAMMER!
    return InstrumentToStemEnum.at(SongPart(fuck));
}

constexpr size_t songHashSize = picosha2::k_digest_size;

struct SongHash {
    unsigned char hash[songHashSize];

    bool operator==(const SongHash rhs) const {
        return std::memcmp(hash, rhs.hash, songHashSize) == 0;
    }
};


// bring on the hash collisions
template<>
struct std::hash<SongHash> {
    std::size_t operator()(const SongHash& hash) const noexcept {
        // oough this is gonna hurt but WHATEVER
        char data[songHashSize+1];
        data[songHashSize] = 0;
        memcpy(data, hash.hash, songHashSize);
        return std::hash<std::string_view>()(std::string_view(data));
    }
};


struct SortName {
    std::string name = "";
    std::string sortName = "";


    SortName() {}
    SortName(const std::string &name) : name(name) {
        UpdateSortName();
    }
    SortName(const char* name) : name(name) {
        UpdateSortName();
    }

    operator std::string&() {
        return name;
    }

    SortName& operator =(const std::string& newValue) {
        name = newValue;
        UpdateSortName();
        return *this;
    }

    const char* c_str() const {
        return name.c_str();
    }

    std::string operator +(const char* add) const {
        return name + add;
    }

    std::string operator +(const std::string& add) const {
        return name + add;
    }

    static std::string lower(std::string string) {
        std::transform(string.begin(),
                       string.end(),
                       string.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return string;
    }

    void UpdateSortName() {
        ZoneScoped;

        std::string use = lower(name);

        if (use.starts_with("a ")) {
            sortName = use.substr(2);
        }
        else if (use.starts_with("an ")) {
            sortName = use.substr(3);
        }
        else if (use.starts_with("the ")) {
            sortName = use.substr(4);
        } else {
            sortName = use;
        }

        sortName = use;
    }
};

inline std::string operator +(const std::string& lhs, const SortName &rhs) {
    return lhs + rhs.name;
}

class Song {
public:
    Song() {
        albumArtPath = TheAssets.getDirectory() / "ui" / "missing.png";
    }
    bool midiParsed = false;
    SortName title = "";
    float titleXOffset = 0;
    float titleTextWidth = 0;
    double titleScrollTime = 0.0;
    SortName artist = "";
    float artistXOffset = 0;
    float artistTextWidth = 0;
    double artistScrollTime = 0.0;
    std::string source = "custom";
    SortName album = "";
    int length = 0;
    int songListPos = 0;
    int BeatTrackID = 0;
    std::string playlist = "";
    double music_start = 0.0;
    double end = 0.0;
    SongHash hash;
    std::vector<PartIcon> partIcons{
        PartIcon::IconNone, PartIcon::IconNone, PartIcon::IconNone, PartIcon::IconNone
    };
    // Parts order will always be Drums, Bass, Guitar, Vocals, Plastic Drums, Plastic
    // Bass, Plastic Guitar
    std::array<int, PlasticVocals> Difficulties {-1};

    std::filesystem::path midiPath = "";
    std::filesystem::path songDir = "";
    std::filesystem::path albumArtPath;
    std::filesystem::path songInfoPath = "";
    std::string releaseYear = "";
    std::string loadingPhrase = "";
    std::vector<std::string> charters{};
    std::string jsonHash = "";
    int hopoThreshold = -1;
    std::vector<std::pair<std::filesystem::path, Encore::AudioManager::Stems>> LoadAudioINI();
    /// Returns an empty string if not found
    std::filesystem::path GetVideoPath();
    float previewStartTime = 0.0f;
    double videoStartTime = 0.0;

    std::string GetPlaylistPath() const {
        return midiPath.parent_path().parent_path().filename().string();
    }

    void LoadInfoINI(std::filesystem::path iniPath);
    void PullInfoFromINI(INIReader &ini);

    void LoadSongIni(const std::filesystem::path& songPath);

    using json = nlohmann::json;

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
                    Encore::Log::Debug("SONG: Song start: {:5.4f}", time);
                }
                if (evt_string == "[end]") {
                    end = time;
                    endTick = events[i].tick;

                    Encore::Log::Debug("SONG: Song end: {:5.4f}", time);
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
                        SongPart songPart;
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

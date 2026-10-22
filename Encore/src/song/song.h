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
    bool operator==(const SortName &that) const {
        return this->sortName == that.sortName;
    }
    bool operator<=(const SortName &that) const {
        return this->sortName <= that.sortName;
    }

    bool operator>=(const SortName &that) const {
        return this->sortName >= that.sortName;
    }

    bool operator>(const SortName &that) const {
        return this->sortName > that.sortName;
    }
    bool operator<(const SortName &that) const {
        return this->sortName < that.sortName;
    }

    bool operator!=(const SortName &that) const {
        return this->sortName != that.sortName;
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

class MTVProvider {
public:
    virtual ~MTVProvider() = default;
    virtual const char* GetTitle() {return "";};
    virtual const char* GetArtist() {return "";};
    virtual const char* GetYear() {return "";};
    virtual const char* GetAlbum() {return "";};
    virtual const char* GetSource() {return "";};
    virtual const char* GetCharter() {return "";};

    virtual std::pair<double, double> GetMTVFadeInRange() {return {0, 0};}
    virtual std::pair<double, double> GetMTVFadeOutRange() {return {0, 0};}
};

class SubSong : public MTVProvider {
public:
    std::string title;
    std::string artist;
    std::string album;
    std::string year;
    std::string source;
    std::string charter;

    // Timing info, blank until the song's chart is loaded
    double startTime = INFINITY;
    double enterTime = INFINITY;
    double exitTime = INFINITY;

    double mtvFadeInTime;
    double mtvFadeOutTime;

    const char *GetTitle() override { return title.c_str(); }
    const char *GetArtist() override { return artist.c_str(); }
    const char *GetAlbum() override { return album.c_str(); }
    const char *GetYear() override { return year.c_str(); }
    const char *GetSource() override { return source.c_str(); }
    const char *GetCharter() override { return charter.c_str(); }

    std::pair<double, double> GetMTVFadeInRange() override {return {enterTime, enterTime+mtvFadeInTime};}
    std::pair<double, double> GetMTVFadeOutRange() override {return {exitTime, exitTime+mtvFadeOutTime};}
};

enum class ChartFormat : uint8_t {
    Midi,
    DotChart
};

class Song : public MTVProvider {
public:
    Song() {
        albumArtPath = TheAssets.getDirectory() / "ui" / "missing.png";
    }
    SortName title = "";
    SortName artist = "";
    std::string source = "custom";
    SortName album = "";
    int length = 0;
    int songListPos = 0;
    int BeatTrackID = 0;
    std::string playlist = "";
    double music_start = 0.0;
    double end = 0.0;
    int trackListing = -1;
    int playlistListing = -1;
    SongHash hash;
    std::vector<PartIcon> partIcons{
        PartIcon::IconNone, PartIcon::IconNone, PartIcon::IconNone, PartIcon::IconNone
    };
    // Parts order will always be Drums, Bass, Guitar, Vocals, Plastic Drums, Plastic
    // Bass, Plastic Guitar
    std::array<int, PlasticVocals> Difficulties {-1};

    ChartFormat chartFormat = ChartFormat::Midi;
    std::filesystem::path chartPath = "";
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
    std::vector<SubSong> subSongs;

    std::shared_ptr<std::istream> GetChartData();

    const char *GetTitle() override { return title.c_str(); }
    const char *GetArtist() override { return artist.c_str(); }
    const char *GetAlbum() override { return artist.c_str(); }
    const char *GetYear() override { return releaseYear.c_str(); }
    const char *GetSource() override { return source.c_str(); }
    const char *GetCharter() override { return charters[0].c_str(); }

    std::pair<double, double> GetMTVFadeInRange() override;
    std::pair<double, double> GetMTVFadeOutRange() override;

    std::string GetPlaylistPath() const {
        return chartPath.parent_path().parent_path().filename().string();
    }

    void LoadInfoINI(std::filesystem::path iniPath);
    void PullInfoFromINI(INIReader &ini);

    void LoadSongIni(const std::filesystem::path& songPath);

    using json = nlohmann::json;

    void LoadAlbumArt();
};

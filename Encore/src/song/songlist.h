#pragma once
#include <filesystem>
#include <vector>

#include "song/song.h"

// Version is formatted as YY_MM_DD_RR, where:
// - YY: Current year (2 digits, 4 digits impedes on 32-bit integer limit)
// - MM: Current month
// - DD: Current day
// - RR: Number of times the cache was revised that day, starting from 1
#define SONG_CACHE_VERSION 24083101
#define SONG_CACHE_HEADER 0x52434E45 // "ENCR"

struct ListMenuEntry {
    bool isHeader;
    int songListID;
    std::string headerChar;
    bool hiddenEntry;
    ListMenuEntry(bool _isHeader, int _songListID, std::string _headerChar, bool _hiddenEntry) {
        isHeader = _isHeader;
        songListID = _songListID;
        headerChar = _headerChar;
        hiddenEntry = _hiddenEntry;
    }
};

enum class SortType : int {
    EnumStart = 0,
    Title = 0,
    Artist,
    Source,
    Length,
    EnumEnd
};

// Enums don't have built-in operator++, and we also need wrapping,
// so we can combine both into one function
inline SortType NextSortType(SortType current) {
    int underlying = static_cast<int>(current);
    current = static_cast<SortType>(++underlying);

    if (current >= SortType::EnumEnd) {
        current = SortType::EnumStart;
    }

    return current;
}

inline std::atomic<int> CurrentChartNumber = -1;
inline std::atomic<int> ListLoadingState = -1;
inline std::atomic<int> MaxChartsToLoad = -1;

enum SongListLoadingStates {
    FINDING_CACHE,
    LOADING_CACHE,
    SCANNING_EXTRAS
};

class SongList {


    static bool sortArtist(const Song &a, const Song &b);
    static bool sortTitle(const Song &a, const Song &b);
    static bool sortSource(const Song &a, const Song &b);
    static bool sortAlbum(const Song &a, const Song &b);
    static bool sortLen(const Song &a, const Song &b);

public:
    SongList();
    ~SongList();

    std::vector<ListMenuEntry> listMenuEntries;
    std::vector<Song> songs;
    int songCount = 0;
    int directoryCount = 0;
    int badSongCount = 0;
    Song *curSong = nullptr;

    void Clear();

    // for when you dont have a song selected
    void sortList(SortType sortType);

    // for when you have a song selected and want to keep that position in song list
    void sortList(SortType sortType, int &selectedSong);

    void WriteCache();

    void ScanSongs(const std::vector<std::filesystem::path> &songsFolder);

    std::vector<ListMenuEntry>
    GenerateSongEntriesWithHeaders(const std::vector<Song> &songs, SortType sortType);

    void LoadCache(const std::vector<std::filesystem::path> &songsFolder);
};

extern SongList TheSongList;

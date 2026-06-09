#pragma once
#include <filesystem>
#include <vector>
#include <atomic>
#include "debug/EncoreDebug.h"

#include "song.h"
#include "menus/util/Jukebox.h"

#include <deque>
#include <queue>

// Version is formatted as YY_MM_DD_RR, where:
// - YY: Current year (2 digits, 4 digits impedes on 32-bit integer limit)
// - MM: Current month
// - DD: Current day
// - RR: Number of times the cache was revised that day, starting from 1
#define SONG_CACHE_VERSION 26052204
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

struct LSection {
    size_t firstListID;
    size_t lastListID;
};

enum class SortType : int {
    EnumStart = 0,
    Title = 0,
    Artist,
    Source,
    Length,
    Year,
    Playlist,
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

inline std::atomic_int CurrentChartNumber = 0;
inline std::atomic_int ListLoadingState = 0;
inline std::atomic_int FolderCount = 0;
inline std::atomic_int SongCount = 0;
inline std::atomic_int BadSongCount = 0;
inline std::atomic_int SongsHashed = 0;
inline std::atomic_bool ScanningSongs = false;
inline std::atomic_int MaxChartsToLoad = 0;
inline std::vector<std::string> sortTypes { "title", "artist", "source", "length", "year", "playlist" };

enum SongListLoadingStates {
    FINDING_CACHE,
    LOADING_CACHE,
    SCANNING_EXTRAS
};

class SongList {


    static bool sortArtist(Song *a, Song *b);
    static bool sortTitle(Song *a, Song *b);
    static bool sortPlaylist(Song *a, Song *b);
    static bool sortSource(Song *a, Song *b);
    static bool sortAlbum(Song *a, Song *b);
    static bool sortLen(Song *a, Song *b);
    static bool sortYear(Song *a, Song *b);

    Song *curSong = nullptr;

    friend class SongSelectMenu;
    friend class MainMenu;
    friend class Encore::Jukebox;
    friend void EncoreDebug::DrawSongList();
    friend void EncoreDebug::MenuBar();
public:
    SongList();
    ~SongList();

    int CurrentSong = 0;
    std::vector<ListMenuEntry> listMenuEntries;
    std::vector<LSection> sectionEntries;
    std::deque<Song> songs;
    std::vector<Song*> sortedSongs;
    std::deque<Song*> playlist;
    bool PlaylistMode = false;
    // only for decorative purposes (i.e. menus/MTV overlay)
    int PlaylistSize = 0;
    int PlaylistIndex = 0;

    int songCount = 0;
    int directoryCount = 0;
    int badSongCount = 0;

    std::filesystem::path cachePath();
    std::filesystem::path badSongsPath();

    void Clear();

    // for when you dont have a song selected
    void sortList(SortType sortType);

    // for when you have a song selected and want to keep that position in song list
    void sortList(SortType sortType, size_t &selectedSong);

    void WriteCache();

    void ScanSongs(const std::vector<std::filesystem::path> &songsFolder);
    void ScanFolder(const std::filesystem::path &folder, std::wofstream &badSongs);

    void GenerateSongEntriesWithHeaders(SortType sortType);

    void LoadCache(const std::vector<std::filesystem::path> &songsFolder);
};

extern SongList TheSongList;
extern std::filesystem::path prefsPath;
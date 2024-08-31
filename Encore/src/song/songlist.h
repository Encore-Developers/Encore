#pragma once
#include <algorithm>
#include <filesystem>
#include <set>
#include <thread>
#include <vector>

#include "picosha2.h"
#include "raylib.h"

#include "song/song.h"

// Version is formatted as YY_MM_DD_RR, where:
// - YY: Current year (2 digits, 4 digits impedes on 32-bit integer limit)
// - MM: Current month
// - DD: Current day
// - RR: Number of times the cache was revised that day, starting from 1
#define SONG_CACHE_VERSION 24'08'31'01
#define SONG_CACHE_HEADER 'ENCR'

enum class SortType : int {
    EnumStart = 0,

    Title = 0,
    Artist,
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

class SongList
{
    SongList() {}
public:
    struct ListMenuEntry {
        bool isHeader;
        int songListID;
        std::string headerChar;
        bool hiddenEntry;
    };

    static SongList& getInstance() {
        static SongList instance; // This is the single instance
        return instance;
    }

    std::vector<ListMenuEntry> listMenuEntries;
    std::vector<Song> songs;
    int songCount = 0;
    int directoryCount  = 0;
    int badSongCount = 0;

private:
    static bool sortArtist(const Song& a, const Song& b) {
        std::string aLower = TextToLower(a.artist.c_str());
        std::string bLower = TextToLower(b.artist.c_str());
        return aLower < bLower;
    }

    static bool sortTitle(const Song& a, const Song& b) {
        std::string aLower = TextToLower(a.title.c_str());
        std::string bLower = TextToLower(b.title.c_str());
        return aLower < bLower;
    }

    static bool sortLen(const Song& a, const Song& b) {
        return a.length < b.length;
    }

public:
    void sortList(SortType sortType) {
        switch (sortType) {
            case SortType::Title:
                std::sort(songs.begin(), songs.end(), sortTitle);
                break;
            case SortType::Artist:
                std::sort(songs.begin(), songs.end(), sortArtist);
                break;
            case SortType::Length:
                std::sort(songs.begin(), songs.end(), sortLen);
                break;
        }
        listMenuEntries = GenerateSongEntriesWithHeaders(songs, sortType);
    }

    void sortList(SortType sortType, int& selectedSong) {
        Song& curSong = songs[selectedSong];
        selectedSong = 0;
        switch (sortType) {
        case SortType::Title:
            std::sort(songs.begin(), songs.end(), sortTitle);
            break;
        case SortType::Artist:
            std::sort(songs.begin(), songs.end(), sortArtist);
            break;
        case SortType::Length:
            std::sort(songs.begin(), songs.end(), sortLen);
            break;
        }
        for (int i = 0; i < songs.size(); i++) {
            if (songs[i].artist == curSong.artist && songs[i].title == curSong.title) {
                selectedSong = i;
                break;
            }
        }
        listMenuEntries = GenerateSongEntriesWithHeaders(songs, sortType);
    }

private:
    static std::string ReadString(std::ifstream& stream) {
        size_t length;
        stream >> length;

        std::string str(length, '\0');
        stream.read(str.data(), str.length());

        return str;
    }

    static void WriteString(std::ofstream& stream, const std::string& str) {
        stream << (size_t)str.length();
        stream.write(str.data(), str.length());
    }

public:
    void WriteCache() {
        std::filesystem::remove("songCache.encr");

        std::ofstream SongCache("songCache.encr", std::ios::binary);

        // Casts used to explicitly indicate type
        SongCache << (uint32_t)SONG_CACHE_HEADER;
        SongCache << (uint32_t)SONG_CACHE_VERSION;
        SongCache << (size_t)songs.size();

        for (const auto& song : songs) {
            WriteString(SongCache, song.songDir);
            WriteString(SongCache, song.albumArtPath);
            WriteString(SongCache, song.songInfoPath);
            WriteString(SongCache, song.jsonHash);

            WriteString(SongCache, song.title);
            WriteString(SongCache, song.artist);

            SongCache << song.length;

            TraceLog(LOG_INFO, TextFormat("%s - %s", song.title.c_str(), song.artist.c_str()));
            TraceLog(LOG_INFO, TextFormat("Directory - %s", song.songDir.c_str()));
            TraceLog(LOG_INFO, TextFormat("Album Art Path - %s", song.albumArtPath.c_str()));
            TraceLog(LOG_INFO, TextFormat("Song Info Path - %s", song.songInfoPath.c_str()));
            TraceLog(LOG_INFO, std::to_string(song.length).c_str());
        }

        SongCache.close();
    }

    static void ScanSongs(const std::vector<std::filesystem::path>& songsFolder)
    {
        SongList list;

        for (const auto &folder : songsFolder) {
            if (!std::filesystem::is_directory(folder)) {
                continue;
            }

            for (const auto &entry: std::filesystem::directory_iterator(folder)) {
                if (!std::filesystem::is_directory(entry)) {
                    continue;
                }

                list.directoryCount++;
                if (std::filesystem::exists(entry.path() / "info.json")) {
                    Song song;
                    song.LoadSong(entry.path() / "info.json");
                    list.songs.push_back(std::move(song));
                    list.songCount++;
                }
                else if (std::filesystem::exists(entry.path() / "song.ini")) {
                    Song song;

                    song.songInfoPath = (entry.path() / "song.ini").string();
                    song.songDir = entry.path().string();
                    song.LoadSongIni(entry.path());
                    song.ini = true;
                    list.songs.push_back(std::move(song));
                    list.songCount++;
                }
            }
        }

        TraceLog(LOG_INFO, "Rewriting song cache");
        list.WriteCache();
    }

    std::vector<ListMenuEntry> GenerateSongEntriesWithHeaders(const std::vector<Song>& songs, SortType sortType) {
        std::vector<ListMenuEntry> songEntries;
        std::string currentHeader = "";

        for (int i = 0; i < songs.size(); i++) {
            const Song& song = songs[i];
            switch (sortType) {
                case SortType::Title: {
                    if (toupper(song.title[0]) != currentHeader[0]) {
                        currentHeader = toupper(song.title[0]);
                        songEntries.emplace_back(true, 0, currentHeader, false);
                    }
                    break;
                }
                case SortType::Artist: {
                    if (song.artist != currentHeader) {
                        currentHeader = song.artist;
                        songEntries.emplace_back(true, 0, currentHeader, false);
                    }
                    break;
                }
                case SortType::Length: {
                    if (std::to_string(song.length) != currentHeader) {
                        currentHeader = std::to_string(song.length);
                        songEntries.emplace_back(true, 0, currentHeader, false);
                    }
                    break;
                }
            }
            songEntries.emplace_back(false, i, "", false);
        }

        return songEntries;
    }

    void LoadCache(const std::vector<std::filesystem::path>& songsFolder) {
        std::ifstream SongCacheIn("songCache.encr", std::ios::binary);
        if (!SongCacheIn) {
            TraceLog(LOG_WARNING, "Failed to load song cache!");
            return;
        }

        listMenuEntries.clear();
        songs.clear();
        songCount = 0;
        directoryCount = 0;
        badSongCount = 0;

        // Read header
        uint32_t header;
        SongCacheIn >> header;
        if (header != SONG_CACHE_HEADER) {
            TraceLog(LOG_WARNING, "Invalid song cache format, rescanning");
            SongCacheIn.close();
            ScanSongs(songsFolder);
            LoadCache(songsFolder);
            return;
        }

        uint32_t version;
        SongCacheIn >> version;
        if (version != SONG_CACHE_VERSION) {
            TraceLog(LOG_WARNING, TextFormat("Cache version %01i, but current version is %01i",version, SONG_CACHE_VERSION));
            SongCacheIn.close();
            ScanSongs(songsFolder);
            LoadCache(songsFolder);
            return;
        }

        size_t cachedSongCount;
        SongCacheIn >> cachedSongCount;

        TraceLog(LOG_INFO, "Loading song cache");
        std::set<std::string> loadedSongs;  // To track loaded songs and avoid duplicates
        for (int i = 0; i < cachedSongCount; i++) {
            Song song;

            // Read cache values
            song.songDir = ReadString(SongCacheIn);
            song.albumArtPath = ReadString(SongCacheIn);
            song.songInfoPath = ReadString(SongCacheIn);
            song.jsonHash = ReadString(SongCacheIn);

            song.title = ReadString(SongCacheIn);
            song.artist = ReadString(SongCacheIn);

            SongCacheIn >> song.length;

            TraceLog(LOG_INFO, TextFormat("Directory - %s", song.songDir.c_str()));

            if (!std::filesystem::exists(song.songDir)) {
                continue;
            }

            // Set other info properties
            if (std::filesystem::path(song.songInfoPath).filename() == "song.ini") {
                song.ini = true;
            }

            std::ifstream jsonFile(song.songInfoPath);
            std::string jsonHashNew = "";
            if (jsonFile) {
                auto itBegin = std::istreambuf_iterator<char>(jsonFile);
                auto itEnd = std::istreambuf_iterator<char>();
                std::string jsonString(itBegin, itEnd);
                jsonFile.close();

                jsonHashNew = picosha2::hash256_hex_string(jsonString);
            }

            if (song.jsonHash != jsonHashNew) {
                continue;
            }

            songs.push_back(std::move(song));
            loadedSongs.insert(song.songDir);
        }

        SongCacheIn.close();
        size_t loadedSongCount = songs.size();

        // Load additional songs from directories if needed
        for (const auto& folder : songsFolder) {
            if (!std::filesystem::is_directory(folder)) {
                continue;
            }

            for (const auto& entry : std::filesystem::directory_iterator(folder)) {
                if (!std::filesystem::is_directory(entry)) {
                    continue;
                }

                if (std::filesystem::exists(entry.path() / "info.json")) {
                    if (loadedSongs.find(entry.path().string()) == loadedSongs.end()) {
                        Song song;
                        song.LoadSong(entry.path() / "info.json");
                        songs.push_back(std::move(song));
                        songCount++;
                    }
                    else {
                        badSongCount++;
                    }
                }

                if (std::filesystem::exists(entry.path() / "song.ini")) {
                    if (loadedSongs.find(entry.path().string()) == loadedSongs.end()) {
                        Song song;
                        song.songInfoPath = (entry.path() / "song.ini").string();
                        song.songDir = entry.path().string();
                        song.LoadSongIni(entry.path());
                        song.ini = true;

                        songs.push_back(std::move(song));
                        songCount++;
                    } else {
                        badSongCount++;
                    }
                }
            }
        }

        if (cachedSongCount != loadedSongCount || songs.size() != loadedSongCount) {
            TraceLog(LOG_INFO, "Updating song cache");
            WriteCache();
        }

        // ScanSongs(songsFolder);
        sortList(SortType::Title);
    }
};


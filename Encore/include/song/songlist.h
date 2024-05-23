#pragma once
#include "song.h"
#include <vector>
#include <filesystem>
#include <thread>
#include <algorithm>
#include <set>
#include "picosha2.h"
class SongList
{
    SongList() {}
public:

    static SongList& getInstance() {
        static SongList instance; // This is the single instance
        return instance;
    }

    static bool sortArtist(const Song& a, const Song& b) {
        return a.artist < b.artist;
    }
    static bool sortTitle(const Song& a, const Song& b) {
        return a.title < b.title;
    }
    static bool sortLen(const Song& a, const Song& b) {
        return a.length < b.length;
    }
    std::vector<Song> songs;
    int songCount = 0;
    int directoryCount  = 0;
    int badSongCount = 0;
    //0 - title
    //1 - artist
    //2 - length
    void sortList(int sortType) {
        switch (sortType) {
            case (0):
                std::sort(songs.begin(), songs.end(), sortTitle);
                break;
            case (1):
                std::sort(songs.begin(), songs.end(), sortArtist);
                break;
            case (2):
                std::sort(songs.begin(), songs.end(), sortLen);
                break;
        }
    }
    void sortList(int sortType,int& selectedSong) {
        Song curSong = songs[selectedSong];
        selectedSong = 0;
        switch (sortType) {
        case (0):
            std::sort(songs.begin(), songs.end(), sortTitle);
            break;
        case (1):
            std::sort(songs.begin(), songs.end(), sortArtist);
            break;
        case (2):
            std::sort(songs.begin(), songs.end(), sortLen);
            break;
        }
        for (int i = 0; i < songs.size();i++) {
            if (songs[i].artist == curSong.artist && songs[i].title == curSong.title) {
                selectedSong = i;
            }
        }
    }

    void WriteCache(std::vector<Song> songs) {
        std::filesystem::remove("songCache.encr");

        std::ofstream SongCache("songCache.encr", std::ios::binary);

        const char header[7] = "ENCORE";
        SongCache.write(header, 6);
        uint16_t version = CACHE_VERSION;
        SongCache.write(reinterpret_cast<const char*>(&version), 2);

        size_t SongCount = songs.size();
        SongCache.write(reinterpret_cast<const char*>(&SongCount), sizeof(SongCount));


        for (const auto& song : songs) {
            TraceLog(LOG_INFO, TextFormat("%s - %s", song.title.c_str(), song.artist.c_str()));
            size_t nameLen = song.title.size();
            size_t songDirectoryLen = song.songDir.size();
            size_t albumArtPathLen = song.albumArtPath.size();
            size_t jsonPathLen = song.songInfoPath.size();
            size_t artistLen = song.artist.size();
            size_t lengthLen = std::to_string(song.length).size();

            SongCache.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
            SongCache.write(song.title.c_str(), nameLen);

            SongCache.write(reinterpret_cast<const char*>(&songDirectoryLen), sizeof(songDirectoryLen));
            SongCache.write(song.songDir.c_str(), songDirectoryLen);
            TraceLog(LOG_INFO, TextFormat("Directory - %s", song.songDir.c_str()));
            SongCache.write(reinterpret_cast<const char*>(&albumArtPathLen), sizeof(albumArtPathLen));
            SongCache.write(song.albumArtPath.c_str(), albumArtPathLen);
            TraceLog(LOG_INFO, TextFormat("Album Art Path - %s", song.albumArtPath.c_str()));

            SongCache.write(reinterpret_cast<const char*>(&artistLen), sizeof(artistLen));
            SongCache.write(song.artist.c_str(), artistLen);

            SongCache.write(reinterpret_cast<const char*>(&jsonPathLen), sizeof(jsonPathLen));
            SongCache.write(song.songInfoPath.c_str(), jsonPathLen);
            TraceLog(LOG_INFO, TextFormat("Song Info Path - %s", song.songInfoPath.c_str()));
            SongCache.write(song.jsonHash.c_str(), 64);

            SongCache.write(reinterpret_cast<const char*>(&lengthLen), sizeof(lengthLen));
            SongCache.write(std::to_string(song.length).c_str(), lengthLen);
            TraceLog(LOG_INFO, std::to_string(song.length).c_str());
        }

        SongCache.close();
    }


    void ScanSongs(const std::vector<std::filesystem::path>& songsFolder)
    {
        SongList list;
        for (const auto & i : songsFolder) {
            for (const auto &entry: std::filesystem::directory_iterator(i)) {
                if (std::filesystem::is_directory(entry)) {
                    list.directoryCount++;
                    if (std::filesystem::exists(entry.path() / "info.json")) {
                        Song song;
                        song.LoadSong(entry.path() / "info.json");
                        list.songs.push_back(song);
                        list.songCount++;
                    } else {
                        list.badSongCount++;
                    }
                }
            }
        }
        TraceLog(LOG_INFO, "Rewriting song cache");
        WriteCache(list.songs);
    }

    SongList LoadCache(const std::vector<std::filesystem::path>& songsFolder) {
        SongList list;
        std::ifstream SongCacheIn("songCache.encr", std::ios::binary);
        if (!SongCacheIn) {
            TraceLog(LOG_WARNING, "Failed to load song cache!");
            return list;
        }

        // Read header
        char header[6];
        SongCacheIn.read(header, 6);
        if (std::string(header, 6) != "ENCORE") {
            TraceLog(LOG_WARNING, "Invalid song cache format, rescanning");
            SongCacheIn.close();
            ScanSongs(songsFolder);
            return LoadCache(songsFolder);
        }

        uint16_t version;
        SongCacheIn.read(reinterpret_cast<char*>(&version), 2);
        if (version != CACHE_VERSION) {
            TraceLog(LOG_WARNING, TextFormat("Cache version %01i, but current version is %01i",version, CACHE_VERSION));
            SongCacheIn.close();
            ScanSongs(songsFolder);
            return LoadCache(songsFolder);
        }

        size_t size;
        SongCacheIn.read(reinterpret_cast<char*>(&size), sizeof(size));
        list.songs.resize(size);
        TraceLog(LOG_INFO, "Loading song cache");

        std::set<std::string> loadedSongs;  // To track loaded songs and avoid duplicates
        int idx = 0;
        for (auto& ___ : list.songs) {
            size_t nameLen = 0, albumArtPathLen = 0, directoryLen = 0, jsonPathLen = 0, artistLen = 0, lengthLen = 0;
            std::string LengthString;
            Song& song = list.songs[idx];
            SongCacheIn.read(reinterpret_cast<char*>(&nameLen), 8);
            song.title.resize(nameLen);
            SongCacheIn.read(&song.title[0], nameLen);

            SongCacheIn.read(reinterpret_cast<char*>(&directoryLen), 8);
            song.songDir.resize(directoryLen);
            SongCacheIn.read(&song.songDir[0], directoryLen);
            

            TraceLog(LOG_INFO, TextFormat("Directory - %s", song.songDir.c_str()));

            SongCacheIn.read(reinterpret_cast<char*>(&albumArtPathLen), 8);
            song.albumArtPath.resize(albumArtPathLen);
            SongCacheIn.read(&song.albumArtPath[0], albumArtPathLen);

            SongCacheIn.read(reinterpret_cast<char*>(&artistLen), 8);
            song.artist.resize(artistLen);
            SongCacheIn.read(&song.artist[0], artistLen);

            SongCacheIn.read(reinterpret_cast<char*>(&jsonPathLen), 8);
            song.songInfoPath.resize(jsonPathLen);
            SongCacheIn.read(&song.songInfoPath[0], jsonPathLen);
            song.jsonHash.resize(64);
            SongCacheIn.read(&song.jsonHash[0], 64);
            std::ifstream jsonFile(song.songInfoPath);
            std::string jsonString((std::istreambuf_iterator<char>(jsonFile)), std::istreambuf_iterator<char>());
            jsonFile.close();
            std::string jsonHashNew = picosha2::hash256_hex_string(jsonString);
            
            SongCacheIn.read(reinterpret_cast<char*>(&lengthLen), 8);
            LengthString.resize(lengthLen);
            SongCacheIn.read(&LengthString[0], lengthLen);

            song.length = std::stoi(LengthString);
            if (!std::filesystem::exists(song.songDir))
                continue;
            if (song.jsonHash != jsonHashNew)
                continue;
            loadedSongs.insert(song.songDir);
            idx++;
        }
        list.songs.resize(idx);
        SongCacheIn.close();
        int loadedFromCache = list.songs.size();
        // Load additional songs from directories if needed
        for (const auto& folder : songsFolder) {
            for (const auto& entry : std::filesystem::directory_iterator(folder)) {
                if (std::filesystem::is_directory(entry)) {
                    if (std::filesystem::exists(entry.path() / "info.json")) {
                        if (loadedSongs.find(entry.path().string()) == loadedSongs.end()) {
                            Song song;
                            song.LoadSong(entry.path() / "info.json");
                            list.songs.push_back(song);
                            list.songCount++;
                        }
                        else {
                            list.badSongCount++;
                        }
                    }
                }
            }
        }
        if (size!=loadedFromCache || list.songs.size() > loadedFromCache) {
            TraceLog(LOG_INFO, "Updating song cache");
            WriteCache(list.songs);
        }
        list.sortList(0);
        return list;
    }
};


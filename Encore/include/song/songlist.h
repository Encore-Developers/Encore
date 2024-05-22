#pragma once
#include "song.h"
#include <vector>
#include <filesystem>
#include <thread>
#include <algorithm>
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
        TraceLog(LOG_INFO, "Reloading song cache");
        std::filesystem::remove("songCache.bin");

        std::ofstream SongCache("songCache.bin", std::ios::binary);
        size_t SongCount = list.songs.size();
        SongCache.write(reinterpret_cast<const char*>(&SongCount),  sizeof(SongCount));


        for (const auto& song : list.songs) {
            TraceLog(LOG_INFO, TextFormat("%s - %s", song.title.c_str(), song.artist.c_str()));
            size_t nameLen = song.title.size();
            size_t songDirectoryLen = song.songDir.size();
            size_t albumArtPathLen = song.albumArtPath.size();
            size_t jsonPathLen = song.songInfoPath.size();
            size_t artistLen = song.artist.size();
            size_t lengthLen =  std::to_string(song.length).size();

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

            SongCache.write(reinterpret_cast<const char*>(&lengthLen), sizeof(lengthLen));
            SongCache.write(std::to_string(song.length).c_str(), lengthLen);
            TraceLog(LOG_INFO, std::to_string(song.length).c_str());
        }

        SongCache.close();
    }

    SongList LoadCache() {
        SongList list;
        std::ifstream SongCacheIn("songCache.bin", std::ios::binary);
        if (!SongCacheIn) {
            TraceLog(LOG_ERROR, "Failed to load song cache!");
            return list;
        }


        size_t size;
        SongCacheIn.read(reinterpret_cast<char*>(&size), sizeof(size));
        list.songs.resize(size);
        TraceLog(LOG_INFO, "Loading song cache");
        int idx=0;
        for (auto& ___ : list.songs) {
            size_t nameLen, albumArtPathLen, directoryLen, jsonPathLen, artistLen, lengthLen;
            std::string LengthString;
            Song& song = list.songs[idx];

            SongCacheIn.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
            song.title.resize(nameLen);
            SongCacheIn.read(&song.title[0], nameLen);

            SongCacheIn.read(reinterpret_cast<char*>(&directoryLen), sizeof(directoryLen));
            song.songDir.resize(directoryLen);
            SongCacheIn.read(&song.songDir[0], directoryLen);
            if (!std::filesystem::exists(song.songDir)) {
                continue;
            }
            TraceLog(LOG_INFO, TextFormat("Directory - %s", song.songDir.c_str()));

            SongCacheIn.read(reinterpret_cast<char*>(&albumArtPathLen), sizeof(albumArtPathLen));
            song.albumArtPath.resize(albumArtPathLen);
            SongCacheIn.read(&song.albumArtPath[0], albumArtPathLen);
            TraceLog(LOG_INFO, TextFormat("Album Art Path - %s", song.albumArtPath.c_str()));

            SongCacheIn.read(reinterpret_cast<char*>(&artistLen), sizeof(artistLen));
            song.artist.resize(artistLen);
            SongCacheIn.read(&song.artist[0], artistLen);

            SongCacheIn.read(reinterpret_cast<char*>(&jsonPathLen), sizeof(jsonPathLen));
            song.songInfoPath.resize(jsonPathLen);
            SongCacheIn.read(&song.songInfoPath[0], jsonPathLen);
            TraceLog(LOG_INFO, TextFormat("Song Info Path - %s", song.songInfoPath.c_str()));

            SongCacheIn.read(reinterpret_cast<char*>(&lengthLen), sizeof(lengthLen));
            LengthString.resize(lengthLen);
            SongCacheIn.read(&LengthString[0], lengthLen);
            TraceLog(LOG_INFO, TextFormat("Song Length - %s", LengthString.c_str()));

            TraceLog(LOG_INFO, TextFormat("%s - %s", song.title.c_str(), song.artist.c_str()));

            song.length = std::stoi(LengthString);
            idx++;
        }
        list.songs.resize(idx);
        SongCacheIn.close();
        list.sortList(0);
        return list;
    };
};


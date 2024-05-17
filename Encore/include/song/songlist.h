#pragma once
#include "song.h"
#include <vector>
#include <filesystem>
#include <thread>
#include <algorithm>
class SongList
{
public:
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

        std::filesystem::remove("songCache.bin");

        std::ofstream SongCache("songCache.bin", std::ios::binary);
        size_t SongCount = list.songs.size();
        SongCache.write(reinterpret_cast<const char*>(&SongCount), sizeof(SongCount));

        for (const auto& song : list.songs) {
            size_t nameLen = song.title.size();
            size_t songDirectoryLen = song.songDir.size();
            size_t albumArtPathLen = song.albumArtPath.size();
            size_t jsonPathLen = song.songInfoPath.size();

            SongCache.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
            SongCache.write(song.title.c_str(), nameLen);

            SongCache.write(reinterpret_cast<const char*>(&songDirectoryLen), sizeof(songDirectoryLen));
            SongCache.write(song.songDir.c_str(), songDirectoryLen);

            SongCache.write(reinterpret_cast<const char*>(&albumArtPathLen), sizeof(albumArtPathLen));
            SongCache.write(song.albumArtPath.c_str(), albumArtPathLen);

            SongCache.write(reinterpret_cast<const char*>(&jsonPathLen), sizeof(jsonPathLen));
            SongCache.write(song.songInfoPath.c_str(), jsonPathLen);
        }

        SongCache.close();
    }

    SongList LoadCache() {
        SongList list;
        std::ifstream SongCache("songCache.bin", std::ios::binary);
        if (!SongCache) {
            TraceLog(LOG_ERROR, "Failed to load song cache!");
            return list;
        }

        size_t size;
        SongCache.read(reinterpret_cast<char*>(&size), sizeof(size));
        list.songs.resize(size);

        for (auto& song : list.songs) {
            size_t nameLen, albumArtPathLen, directoryLen, jsonPathLen;

            SongCache.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
            song.title.resize(nameLen);
            SongCache.read(&song.title[0], nameLen);

            SongCache.read(reinterpret_cast<char*>(&albumArtPathLen), sizeof(albumArtPathLen));
            song.albumArtPath.resize(albumArtPathLen);
            SongCache.read(&song.albumArtPath[0], albumArtPathLen);

            SongCache.read(reinterpret_cast<char*>(&directoryLen), sizeof(directoryLen));
            song.songDir.resize(directoryLen);
            SongCache.read(&song.songDir[0], directoryLen);

            SongCache.read(reinterpret_cast<char*>(&jsonPathLen), sizeof(jsonPathLen));
            song.songInfoPath.resize(jsonPathLen);
            SongCache.read(&song.songInfoPath[0], jsonPathLen);

            song.LoadSong(song.songInfoPath);
        }

        SongCache.close();
        list.sortList(0);
        return list;
    };
};


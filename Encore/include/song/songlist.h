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
    SongList LoadSongs(const std::vector<std::filesystem::path>& songsFolder)
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
        list.sortList(0);
        return list;
    }
};


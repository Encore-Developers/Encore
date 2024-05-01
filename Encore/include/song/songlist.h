#pragma once
#include "song.h"
#include <vector>
#include <filesystem>
#include <thread>

class SongList
{
public:
    std::vector<Song> songs;
    int songCount = 0;
    int directoryCount  = 0;
    int badSongCount = 0;
};

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
    return list;
}
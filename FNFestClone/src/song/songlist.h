#pragma once
#include "song.h"
#include <vector>
#include <filesystem>

class SongList
{
public:
    std::vector<Song> songs;
};

SongList LoadSongs(std::filesystem::path songsFolder) 
{
    SongList list;

    for (const auto& entry : std::filesystem::directory_iterator(songsFolder)) 
    {
        if (std::filesystem::is_directory(entry)) 
        {
            if (std::filesystem::exists(entry.path() / "info.json"))
            {
                Song song;
                song.LoadSong(entry.path() / "info.json");
                list.songs.push_back(song);
            }
        }
    }
    return list;
}
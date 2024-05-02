#pragma once
#include "song.h"
#include <vector>
#include <filesystem>
#include <thread>

#ifndef SONGLIST_H
#define SONGLIST_H

class SongList
{
public:
    std::vector<Song> songs;
    int songCount;
    int directoryCount;
    int badSongCount;
};

SongList LoadSongs(const std::vector<std::filesystem::path>& songsFolder);

#endif
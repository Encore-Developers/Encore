#pragma once

#include "songlist.h"

SongList LoadSongs(const std::vector<std::filesystem::path>& songsFolder)
{
    SongList list;

    // for every song folder
    for (const auto & i : songsFolder) {

        // for every song in the specified song folder
        for (const auto &entry: std::filesystem::directory_iterator(i)) {
            // are we sure its a folder?
            if (std::filesystem::is_directory(entry)) {
                list.directoryCount++;

                // find the info.json and add the song to the list
                if (std::filesystem::exists(entry.path() / "info.json")) {
                    Song song;
                    song.LoadSong(entry.path() / "info.json");
                    list.songs.emplace_back(song);
                    list.songCount++;
                } else {
                    list.badSongCount++;
                }
            }
        }
    }
    return list;
}
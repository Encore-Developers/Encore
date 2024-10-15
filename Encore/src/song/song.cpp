//
// Created by marie on 13/08/2024.
//

#include "song.h"


#include "inih/INIReader.h"
#include <map>

std::map<std::string, int> IniStems = {
    { "song", 4 },    { "guitar", 2 },  { "bass", 1 },     { "rhythm", 1 },
    { "keys", 8 },    { "vocals", 3 },  { "vocals_1", 3 }, { "vocals_2", 3 },
    { "drums", 0 },   { "drums_1", 0 }, { "drums_2", 0 },  { "drums_3", 0 },
    { "drums_4", 0 }, { "crowd", 4 }
};

void Song::LoadInfoINI(std::filesystem::path iniPath) {
    for (const auto &entry : std::filesystem::directory_iterator(iniPath.parent_path())) {
        if (entry.is_regular_file()) {
            std::string base_filename = entry.path().string().substr(
                entry.path().string().find_last_of("/\\") + 1
            );
            std::string::size_type const p(base_filename.find_last_of('.'));
            std::string file_without_extension = base_filename.substr(0, p);

            if (file_without_extension == "album") {
                albumArtPath = entry.path().string();
            }
            if (base_filename == "notes.mid") {
                midiPath = entry.path();
            }
        }
    }

    INIReader ini(iniPath.string());

    title = ini.GetString("song", "name", "Unknown Song");

    artist = ini.GetString("song", "artist", "Unknown Artist");

    // genre = ini.GetValue("song", "genre");
    charters.push_back(ini.GetString("song", "charter", "Unkown Charter"));

    album = ini.GetString("song", "album", "Unknown Album");

    releaseYear = ini.GetInteger("song", "year", 0);

    hopoThreshold = ini.GetInteger("song", "hopo_threshold", 170);
    length = ini.GetInteger("song", "song_length", 0);

    parts[PlasticGuitar]->diff = ini.GetInteger("song", "diff_guitar", -1);

    parts[PlasticBass]->diff = ini.GetInteger("song", "diff_bass", -1);

    parts[PlasticDrums]->diff = ini.GetInteger("song", "diff_drums", -1);

    parts[PlasticKeys]->diff = ini.GetInteger("song", "diff_keys", -1);

    parts[PitchedVocals]->diff = ini.GetInteger("song", "diff_vocals", -1);

    parts[PartGuitar]->diff = ini.GetInteger("song", "diff_guitar_pad", -1);

    parts[PartBass]->diff = ini.GetInteger("song", "diff_bass_pad", -1);

    parts[PartKeys]->diff = ini.GetInteger("song", "diff_keys_pad", -1);

    parts[PartDrums]->diff = ini.GetInteger("song", "diff_drums_pad", -1);

    parts[PartVocals]->diff = ini.GetInteger("song", "diff_vocals_pad", -1);
}

void Song::LoadSongIni(std::filesystem::path songPath) {
    for (const auto &entry : std::filesystem::directory_iterator(songPath)) {
        if (entry.is_regular_file()) {
            std::string base_filename = entry.path().string().substr(
                entry.path().string().find_last_of("/\\") + 1
            );
            std::string::size_type const p(base_filename.find_last_of('.'));
            std::string file_without_extension = base_filename.substr(0, p);

            if (file_without_extension == "album") {
                albumArtPath = entry.path().string();
            }
            if (base_filename == "notes.mid") {
                midiPath = entry.path();
            }
        }
    }
    LoadInfoINI(songInfoPath);
    LoadAudioINI(songPath);
}

void Song::LoadAudioINI(std::filesystem::path songPath) {
    for (const auto &entry : std::filesystem::directory_iterator(songPath)) {
        if (entry.is_regular_file()) {
            std::string base_filename = entry.path().string().substr(
                entry.path().string().find_last_of("/\\") + 1
            );
            std::string::size_type const p(base_filename.find_last_of('.'));
            std::string file_without_extension = base_filename.substr(0, p);
            for (auto filename : IniStems) {
                if (file_without_extension == filename.first
                    && base_filename != "song.ini") {
                    stemsPath.push_back({ entry.path().string(), filename.second });
                }
            }
        }
    }
}

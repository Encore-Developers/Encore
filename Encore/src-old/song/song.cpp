//
// Created by marie on 13/08/2024.
//

#include "song.h"


#include "inih/INIReader.h"
#include <map>

std::map<std::string, int> IniStems = {
    { "song", Invalid },        { "guitar", PartGuitar },   { "bass", PartBass },
    { "rhythm", PartBass },     { "keys", PartKeys },       { "vocals", PartVocals },
    { "vocals_1", PartVocals }, { "vocals_2", PartVocals }, { "drums", PartDrums },
    { "drums_1", PartDrums },   { "drums_2", PartDrums },   { "drums_3", PartDrums },
    { "drums_4", PartDrums },   { "crowd", Invalid }
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
    PullInfoFromINI(ini);

}

void Song::PullInfoFromINI(INIReader &ini) {
    title = ini.GetString("song", "name", "Unknown Song");

    artist = ini.GetString("song", "artist", "Unknown Artist");

    // genre = ini.GetValue("song", "genre");
    charters.push_back(ini.GetString("song", "charter", "Unknown Charter"));

    album = ini.GetString("song", "album", "Unknown Album");

    source = ini.GetString("song", "icon", "custom");

    if (source == "custom") {
        source = ini.GetString("song", "game_origin", "custom");
    }

    releaseYear = ini.GetString("song", "year", "Unknown Year");

    hopoThreshold = ini.GetInteger("song", "hopo_frequency", 170);

    length = ini.GetInteger("song", "song_length", 0);

    previewStartTime = ini.GetInteger("song", "preview_start_time", 0);

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

void Song::LoadSongIni(const std::filesystem::path& songPath) {
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

void Song::LoadAudioINI(const std::filesystem::path& songPath) {
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

SongParts GetSongPart(Song* song, smf::MidiEventList track) {
    for (int events = 0; events < track.getSize(); events++) {
        std::string trackName;
        if (!track[events].isMeta())
            continue;
        if ((int)track[events][1] == 3) {
            for (int k = 3; k < track[events].getSize(); k++) {
                trackName += track[events][k];
            }
            return partFromStringINI(trackName);
        }
    }
    return Invalid;
}

std::vector<std::vector<int> > pDiffRangeNotes = {
    { 60, 64 }, { 72, 76 }, { 84, 88 }, { 96, 100 }
};

void IsPartValid(Song* song, smf::MidiEventList track, SongParts songPart, int trackNumber) {
    if (songPart == Invalid || songPart == PitchedVocals || songPart == BeatLines) {
        return;
    }
    for (int diff = 0; diff < 4; diff++) {
        bool StopSearching = false;

        for (int i = 0; i < track.getSize(); i++) {
            if (track[i].isNoteOn() && !track[i].isMeta()
                && track[i][1] >= pDiffRangeNotes[diff][0]
                && track[i][1] <= pDiffRangeNotes[diff][1] && !StopSearching) {
                song->parts[songPart]->ValidDiffs.at(diff) = true;
                song->parts[songPart]->TrackInt = trackNumber;
                song->parts[songPart]->Valid = true;
                StopSearching = true;
                }
            if (StopSearching)
                break;
        }
    }
}

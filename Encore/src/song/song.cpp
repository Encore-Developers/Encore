//
// Created by marie on 13/08/2024.
//

#include "song.h"

#include "ArtLoader.h"
#include "inih/INIReader.h"
#include <map>

#include "audio.h"

using namespace Encore;

std::map<std::string, AudioManager::Stems> IniStems = {
    { "song",       AudioManager::Stems::Background },
    { "guitar",     AudioManager::Stems::Guitar },
    { "bass",       AudioManager::Stems::Bass },
    { "rhythm",     AudioManager::Stems::Bass },
    { "keys",       AudioManager::Stems::Keys },
    { "vocals",     AudioManager::Stems::Vocals },
    { "vocals_1",   AudioManager::Stems::BackingVocals },
    { "vocals_2",   AudioManager::Stems::Vocals },
    { "drums",      AudioManager::Stems::Drums1 },
    { "drums_1",    AudioManager::Stems::Drums1 },
    { "drums_2",    AudioManager::Stems::Drums2 },
    { "drums_3",    AudioManager::Stems::Drums3 },
    { "drums_4",    AudioManager::Stems::Drums4 },
    { "crowd",      AudioManager::Stems::Crowd }
};

void Song::LoadInfoINI(std::filesystem::path iniPath) {
    for (const auto &entry : std::filesystem::directory_iterator(iniPath.parent_path())) {
        if (entry.is_regular_file()) {
            if (entry.path().stem() == "album") {
                albumArtPath = entry.path();
            }
            if (entry.path().filename() == "notes.mid") {
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

    releaseYear = ini.GetString("song", "year", "Unknown Year");

    hopoThreshold = ini.GetInteger("song", "hopo_frequency", 170);

    length = ini.GetInteger("song", "song_length", 0);

    previewStartTime = ini.GetInteger("song", "preview_start_time", 0);

    parts[PlasticGuitar].diff = ini.GetInteger("song", "diff_guitar", -1);

    parts[PlasticBass].diff = ini.GetInteger("song", "diff_bass", -1);

    parts[PlasticDrums].diff = ini.GetInteger("song", "diff_drums", -1);

    parts[PlasticKeys].diff = ini.GetInteger("song", "diff_keys", -1);

    parts[PitchedVocals].diff = ini.GetInteger("song", "diff_vocals", -1);

    parts[PartGuitar].diff = ini.GetInteger("song", "diff_guitar_pad", -1);

    parts[PartBass].diff = ini.GetInteger("song", "diff_bass_pad", -1);

    parts[PartKeys].diff = ini.GetInteger("song", "diff_keys_pad", -1);

    parts[PartDrums].diff = ini.GetInteger("song", "diff_drums_pad", -1);

    parts[PartVocals].diff = ini.GetInteger("song", "diff_vocals_pad", -1);
}

void Song::LoadSongIni(const std::filesystem::path& songPath) {
    for (const auto &entry : std::filesystem::directory_iterator(songPath)) {
        if (entry.is_regular_file()) {
            if (entry.path().stem() == "album") {
                albumArtPath = entry.path();
            }
            if (entry.path().filename() == "notes.mid") {
                midiPath = entry.path();
            }
        }
    }
    LoadInfoINI(songInfoPath);
}

void Song::LoadAlbumArt() {
    TheArtLoader.LoadAlbumArt(this);
}

std::vector<std::pair<std::filesystem::path, Encore::AudioManager::Stems>> Song::LoadAudioINI() {
    ZoneScoped
    std::vector<std::pair<std::filesystem::path, Encore::AudioManager::Stems>> stemsPath;
    for (const auto &entry : std::filesystem::directory_iterator(songDir)) {
        if (entry.is_regular_file()) {
            for (auto filename : IniStems) {
                if (entry.path().stem() == filename.first
                    && entry.path().filename() != "song.ini") {
                    stemsPath.push_back({ entry.path(), filename.second });
                }
            }
        }
    }
    return stemsPath;
}

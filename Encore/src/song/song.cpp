//
// Created by marie on 13/08/2024.
//

#include "song.h"

#include "ArtLoader.h"
#include "inih/INIReader.h"
#include <map>

#include "audio.h"

using namespace Encore;

std::vector<std::string> IniStems = {
    "song",     "guitar", "bass",    "rhythm",  "keys",    "vocals",  "vocals_1",
    "vocals_2", "drums",  "drums_1", "drums_2", "drums_3", "drums_4", "crowd",
};

std::pair<double, double> Song::GetMTVFadeInRange() {
    return {0, 0};
}
std::pair<double, double> Song::GetMTVFadeOutRange() {
    if (!subSongs.empty()) {
        return {0, subSongs[0].startTime};
    }
    return {INFINITY, INFINITY};
}
void Song::LoadInfoINI(std::filesystem::path iniPath) {
    for (const auto &entry : std::filesystem::directory_iterator(iniPath.parent_path())) {
        if (entry.is_regular_file()) {
            if (entry.path().stem() == "album") {
                albumArtPath = entry.path();
            }
            if (entry.path().filename() == "notes.mid") {
                chartPath = entry.path();
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

    loadingPhrase = ini.GetString("song", "loading_phrase", "");


    trackListing = ini.GetInteger("song", "album_track", -1);
    if (trackListing == -1)
        trackListing = ini.GetInteger("song", "track", -1);

    playlistListing = ini.GetInteger("song", "playlist_track", -1);

    length = ini.GetInteger("song", "song_length", 0);

    previewStartTime = ini.GetInteger("song", "preview_start_time", 0);

    videoStartTime = ini.GetInteger("song", "video_start_time", 0) / 1000.0f;

    for (auto& part : RhythmEngine::SongPart::LoadedParts | std::views::values) {
        if (ini.HasValue("song", part.iniDifficultyKey)) {
            Difficulties[&part] = ini.GetInteger("song", part.iniDifficultyKey, -1);
        }
    }

    subSongs.clear();
    int subsongCount = ini.GetInteger("song", "subsongs", 0);
    for (int i = 0; i < subsongCount; i++) {
        std::string sectionName = std::format("subsong_{}", i+1);
        if (!ini.HasSection(sectionName)) {
            Encore::Log::Error("Error loading subsongs for {}: subsongs property is {} but section [subsong_{}] is not defined;", title.c_str(), subsongCount, i+1);
            break;
        }
        SubSong subsong;

        // All of the subsong properties are optional and fall back to the main song's values when missing
        subsong.title = ini.GetString(sectionName, "name", title.name);
        subsong.artist = ini.GetString(sectionName, "artist", artist.name);
        subsong.album = ini.GetString(sectionName, "album", album.name);
        subsong.source = ini.GetString(sectionName, "icon", source);
        subsong.charter = ini.GetString(sectionName, "charter", charters[0]);
        subsong.year = ini.GetString(sectionName, "year", releaseYear);

        subsong.mtvFadeInTime = ini.GetReal(sectionName, "mtv_fade_in_time", 1);
        subsong.mtvFadeOutTime = ini.GetReal(sectionName, "mtv_fade_out_time", 1);

        subSongs.push_back(subsong);
    }
}

void Song::LoadSongIni(const std::filesystem::path& songPath) {
    for (const auto &entry : std::filesystem::directory_iterator(songPath)) {
        if (entry.is_regular_file()) {
            if (entry.path().stem() == "album") {
                albumArtPath = entry.path();
            }
            if (entry.path().filename() == "notes.mid") {
                chartPath = entry.path();
                chartFormat = ChartFormat::Midi;
            }
            if (entry.path().filename() == "notes.chart") {
                chartPath = entry.path();
                chartFormat = ChartFormat::DotChart;
            }
        }
    }
    LoadInfoINI(songInfoPath);
}

void Song::LoadAlbumArt() {
    TheArtLoader.LoadAlbumArt(this);
}

std::vector<std::pair<std::filesystem::path, std::string>> Song::LoadAudioINI() {
    ZoneScoped
    std::vector<std::pair<std::filesystem::path, std::string>> stemsPath;
    for (const auto &entry : std::filesystem::directory_iterator(songDir)) {
        if (entry.is_regular_file()) {
            for (const auto& filename : IniStems) {
                if (entry.path().stem() == filename
                    && entry.path().filename() != "song.ini") {
                    stemsPath.emplace_back( entry.path(), filename);
                }
            }
        }
    }
    return stemsPath;
}
std::filesystem::path Song::GetVideoPath() {
    for (const auto& entry : std::filesystem::directory_iterator(songDir)) {
        if (entry.is_regular_file()) {
            if (entry.path().stem() == "video") {
                return entry.path();
            }
        }
    }
    return "";
}
std::shared_ptr<std::istream> Song::GetChartData() {
    switch (chartFormat) {
    case ChartFormat::Midi:
        return std::make_shared<std::ifstream>(chartPath, std::ios::binary);
    case ChartFormat::DotChart:
        return std::make_shared<std::ifstream>(chartPath);
    }
    return nullptr;
}

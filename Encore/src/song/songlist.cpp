#include "songlist.h"
#include "util/enclog.h"

#include <set>

#include <algorithm>
#include <nlohmann/json.hpp>
#include <fstream>
#include "util/binary.h"

using json = nlohmann::json;
// sorting
void SongList::Clear() {
    listMenuEntries.clear();
    songs.clear();
    songCount = 0;
    directoryCount = 0;
    badSongCount = 0;
}

bool startsWith(const std::string &str, const std::string &prefix) {
    return str.substr(0, prefix.size()) == prefix;
}

std::string removeArticle(const std::string &str) {
    std::string result = str;

    if (startsWith(result, "a ")) {
        return str.substr(2);
    } else if (startsWith(result, "an ")) {
        return str.substr(3);
    } else if (startsWith(result, "the ")) {
        return str.substr(4);
    }
    return str;
}

bool SongList::sortArtist(const Song &a, const Song &b) {
    std::string aLower = TextToLower(a.artist.c_str());
    std::string bLower = TextToLower(b.artist.c_str());
    std::string aaa = removeArticle(aLower);
    std::string bbb = removeArticle(bLower);
    return aaa < bbb;
}

bool SongList::sortTitle(const Song &a, const Song &b) {
    std::string aLower = TextToLower(a.title.c_str());
    std::string bLower = TextToLower(b.title.c_str());
    std::string aaa = removeArticle(aLower);
    std::string bbb = removeArticle(bLower);
    return aaa < bbb;
}

bool SongList::sortSource(const Song &a, const Song &b) {
    std::string aLower = TextToLower(a.source.c_str());
    std::string bLower = TextToLower(b.source.c_str());
    return aLower < bLower;
}

bool SongList::sortAlbum(const Song &a, const Song &b) {
    std::string aLower = TextToLower(a.album.c_str());
    std::string bLower = TextToLower(b.album.c_str());
    std::string aaa = removeArticle(aLower);
    std::string bbb = removeArticle(bLower);
    return aaa < bbb;
}

bool SongList::sortLen(const Song &a, const Song &b) {
    return a.length < b.length;
}

bool SongList::sortYear(const Song &a, const Song &b) {
    return a.releaseYear < b.releaseYear;
}

SongList::SongList() {}
SongList::~SongList() {}

void SongList::sortList(SortType sortType) {
    switch (sortType) {
    case SortType::Title:
        std::sort(songs.begin(), songs.end(), sortTitle);
        break;
    case SortType::Artist:
        std::sort(songs.begin(), songs.end(), sortAlbum);
        std::sort(songs.begin(), songs.end(), sortArtist);
        break;
    case SortType::Source:
        std::sort(songs.begin(), songs.end(), sortTitle);
        std::sort(songs.begin(), songs.end(), sortSource);
        break;
    case SortType::Length:
        std::sort(songs.begin(), songs.end(), sortLen);
        break;
    case SortType::Year:
        std::sort(songs.begin(), songs.end(), sortTitle);
        std::sort(songs.begin(), songs.end(), sortYear);
        break;
    default:;
    }
    listMenuEntries = GenerateSongEntriesWithHeaders(songs, sortType);
}

void SongList::sortList(SortType sortType, int &selectedSong) {
    Song curSong;
    bool hasCurrentSong = selectedSong >= 0 && selectedSong < songs.size();
    if (hasCurrentSong) {
        curSong = songs[selectedSong];
    }
    selectedSong = 0;
    switch (sortType) {
    case SortType::Title:
        std::sort(songs.begin(), songs.end(), sortTitle);
        break;
    case SortType::Artist:
        std::sort(songs.begin(), songs.end(), sortAlbum);
        std::sort(songs.begin(), songs.end(), sortArtist);
        break;
    case SortType::Source:
        std::sort(songs.begin(), songs.end(), sortTitle);
        std::sort(songs.begin(), songs.end(), sortSource);
        break;
    case SortType::Length:
        std::sort(songs.begin(), songs.end(), sortLen);
        break;
    case SortType::Year:
        std::sort(songs.begin(), songs.end(), sortTitle);
        std::sort(songs.begin(), songs.end(), sortYear);
    break;
    default:;
    }
    if (hasCurrentSong) {
        for (size_t i = 0; i < songs.size(); i++) {
            if (songs[i].artist == curSong.artist && songs[i].title == curSong.title) {
                selectedSong = i;
                break;
            }
        }
    }
    listMenuEntries = GenerateSongEntriesWithHeaders(songs, sortType);
}

void SongList::WriteCache() {
    std::filesystem::remove("songCache.encr");

    // Native-endian order used for best performance, since the cache is not a portable
    // file
    encore::bin_ofstream_native SongCache("songCache.encr", std::ios::binary);

    // Casts used to explicitly indicate type
    SongCache << (uint32_t)SONG_CACHE_HEADER;
    SongCache << (uint32_t)SONG_CACHE_VERSION;
    SongCache << (size_t)songs.size();

    for (const auto &song : songs) {
        SongCache << song.songDir;
        SongCache << song.albumArtPath;
        SongCache << song.songInfoPath.string();
        SongCache << song.jsonHash;

        SongCache << song.title;
        SongCache << song.artist;

        SongCache << song.length;
        SongCache << song.releaseYear;

        Encore::EncoreLog(LOG_INFO, TextFormat("CACHE: Song found:     %s - %s", song.title.c_str(), song.artist.c_str()));
        Encore::EncoreLog(LOG_INFO, TextFormat("CACHE: Directory:      %s", song.songDir.c_str()));
        Encore::EncoreLog(LOG_INFO, TextFormat("CACHE: Album Art Path: %s", song.albumArtPath.c_str()));
        Encore::EncoreLog(LOG_INFO, TextFormat("CACHE: Song Info Path: %s", song.songInfoPath.c_str()));
        Encore::EncoreLog(LOG_INFO, TextFormat("CACHE: Song length:    %01i", song.length));
    }

    SongCache.close();
}


void SongList::ScanFolder(const std::filesystem::path &folder) {
    if (!std::filesystem::is_directory(folder)) {
        return;
    }

    directoryCount++;
    Song song;
    std::filesystem::path infoPath = folder / "info.json";
    if (std::filesystem::exists(infoPath)) {
        song.songInfoPath = (folder / "info.json");
        song.songDir = folder.string();
        if (std::filesystem::exists(folder / "cover.png")) {
            song.albumArtPath = (folder / "cover.png").string();
        }
        if (std::filesystem::exists(folder / "cover.jpg")) {
            song.albumArtPath = (folder / "cover.jpg").string();
        }
        if (std::filesystem::exists(folder / "cover.jpeg")) {
            song.albumArtPath = (folder / "cover.jpeg").string();
        }
        song.LoadSongJSON(song.songInfoPath);
    } else if (std::filesystem::exists(folder / "song.ini")) {
        song.songInfoPath = (folder / "song.ini").string();
        song.songDir = folder.string();
        song.LoadSongIni(folder);
        song.ini = true;
    } else {
        // If this folder doesn't have song.ini or song.json, this must be a organizational folder; continue scanning.
        for (const auto &entry : std::filesystem::directory_iterator(folder)) {
            ScanFolder(entry.path());
        };
    }

    songs.push_back(std::move(song));
    songCount++;
}

void SongList::ScanSongs(const std::vector<std::filesystem::path> &songsFolder) {
    Clear();

    for (const auto &folder : songsFolder) {
        if (!is_directory(folder)) {
            continue;
        }
        
        ScanFolder(folder);
    }

    Encore::EncoreLog(LOG_INFO, "CACHE: Rewriting song cache");
    WriteCache();
}

std::string GetLengthHeader(int length) {
    if (length < 60) return "< 1:00";
    if (length < 120) return "1:00-2:00";
    if (length < 180) return "2:00-3:00";
    if (length < 240) return "3:00-4:00";
    if (length < 300) return "4:00-5:00";
    return "5:00+";
}

std::vector<ListMenuEntry> SongList::GenerateSongEntriesWithHeaders(
    const std::vector<Song> &songs, SortType sortType
) {
    std::vector<ListMenuEntry> songEntries;
    std::string currentHeader = "";
    int pos = 0;
    for (size_t i = 0; i < songs.size(); i++) {
        const Song &song = songs[i];
        std::string header;
        switch (sortType) {
        case SortType::Title: {
            std::string title = removeArticle(TextToLower(song.title.c_str()));
            header = title.empty() ? "#" : std::string(1, toupper(title[0]));
            break;
        }
        case SortType::Artist: {
            std::string artist = removeArticle(song.artist);
            header = artist.empty() ? "#" : artist;
            break;
        }
        case SortType::Source: {
            std::string source = removeArticle(song.source);
            header = source.empty() ? "Unknown" : source;
            break;
        }
        case SortType::Length: {
            header = GetLengthHeader(song.length);
            break;
        }
        case SortType::Year: {
            header = song.releaseYear.empty() ? "Unknown Year" : song.releaseYear;
            break;
        }
        default:
            header = "#";
            break;
        }
        if (header != currentHeader) {
            currentHeader = header;
            songEntries.emplace_back(true, 0, currentHeader, false);
            pos++;
        }
        songEntries.emplace_back(false, i, "", false);
        pos++;
        this->songs[i].songListPos = pos;
    }

    return songEntries;
}

void SongList::LoadCache(const std::vector<std::filesystem::path> &songsFolder) {
    encore::bin_ifstream_native SongCacheIn("songCache.encr", std::ios::binary);
    if (!SongCacheIn) {
        Encore::EncoreLog(LOG_WARNING, "CACHE: Failed to load song cache!");
        SongCacheIn.close();
        ScanSongs(songsFolder);
        return;
    }

    // Read header
    uint32_t header;
    SongCacheIn >> header;
    if (header != SONG_CACHE_HEADER) {
        Encore::EncoreLog(LOG_WARNING, "CACHE: Invalid song cache format, rescanning");
        SongCacheIn.close();
        ScanSongs(songsFolder);
        return;
    }

    uint32_t version;
    SongCacheIn >> version;
    if (version != SONG_CACHE_VERSION) {
        Encore::EncoreLog(
            LOG_WARNING,
            TextFormat(
                "CACHE: Cache version %01i, but current version is %01i",
                version,
                SONG_CACHE_VERSION
            )
        );
        SongCacheIn.close();
        ScanSongs(songsFolder);
        return;
    }

    size_t cachedSongCount;
    SongCacheIn >> cachedSongCount;
    ListLoadingState = LOADING_CACHE;
    // Load cached songs
    Clear();
    Encore::EncoreLog(LOG_INFO, "CACHE: Loading song cache");
    std::set<std::string> loadedSongs; // To track loaded songs and avoid duplicates
    MaxChartsToLoad = cachedSongCount;
    for (int i = 0; i < cachedSongCount; i++) {
        CurrentChartNumber = i;
        Song song;

        // Read cache values
        SongCacheIn >> song.songDir;
        SongCacheIn >> song.albumArtPath;

        std::string infopath;
        SongCacheIn >> infopath;
        song.songInfoPath = infopath;

        SongCacheIn >> song.jsonHash;

        SongCacheIn >> song.title;
        SongCacheIn >> song.artist;

        SongCacheIn >> song.length;
        SongCacheIn >> song.releaseYear;

        Encore::EncoreLog(LOG_INFO, TextFormat("CACHE: Directory - %s", song.songDir.c_str()));

        if (!std::filesystem::exists(song.songDir)) {
            continue;
        }

        // Set other info properties
        if (std::filesystem::path(song.songInfoPath).filename() == "song.ini") {
            song.ini = true;
        }

        std::ifstream jsonFile(song.songInfoPath);
        std::string jsonHashNew = "";
        if (jsonFile) {
            auto itBegin = std::istreambuf_iterator<char>(jsonFile);
            auto itEnd = std::istreambuf_iterator<char>();
            std::string jsonString(itBegin, itEnd);
            jsonFile.close();

            jsonHashNew = picosha2::hash256_hex_string(jsonString);
        }

        if (song.jsonHash != jsonHashNew) {
            continue;
        }
        loadedSongs.insert(song.songDir);
        songs.push_back(std::move(song));

    }

    SongCacheIn.close();
    size_t loadedSongCount = songs.size();


    if (cachedSongCount != loadedSongCount || songs.size() != loadedSongCount) {
        Encore::EncoreLog(LOG_INFO, "CACHE: Updating song cache");
        WriteCache();
    }

    // ScanSongs(songsFolder);
    sortList(SortType::Title);
}
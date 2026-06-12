#include "songlist.h"

#include "picosha2.h"
#include "SDL3/SDL_filesystem.h"
#include "util/enclog.h"

#include <set>

#include <algorithm>
#include <codecvt>
#include <nlohmann/json.hpp>
#include <fstream>
#include <regex>
#include <sstream>
#include "util/binary.h"
#include "util/threadpool.h"

using json = nlohmann::json;

std::filesystem::path SongList::cachePath() {
    return prefsPath / std::filesystem::path("songCache.encr");
}

std::filesystem::path SongList::badSongsPath() {
    return prefsPath / std::filesystem::path("bad-songs.txt");
}

// sorting
void SongList::Clear() {
    listMenuEntries.clear();
    songs.clear();
    sortedSongs.clear();
    songHashIndex.clear();
    songCount = 0;
    directoryCount = 0;
    badSongCount = 0;
}
void SongList::PopulateHashIndex() {
    songHashIndex.clear();
    for (auto& song : songs) {
        songHashIndex.emplace(song.hash, &song);
    }
}

std::string lower(std::string string) {
    std::transform(string.begin(),
                   string.end(),
                   string.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return string;
}

bool startsWith(const std::string &str, const std::string &prefix) {
    return str.substr(0, prefix.size()) == prefix;
}

std::string removeArtistJunk(const std::string &str) {
    ZoneScoped;
    static const std::regex artistRegex("(?:(the|a|an|and||\\W) +)|([\\W])");
    return std::regex_replace(str, artistRegex, " ");
}

std::string removeArticle(const std::string &str) {
    ZoneScoped;

    if (str.starts_with("a ")) {
        return str.substr(2);
    }
    if (str.starts_with("an ")) {
        return str.substr(3);
    }
    if (str.starts_with("the ")) {
        return str.substr(4);
    }
    return str;
}

bool SongList::sortArtist(Song *a, Song *b) {
    ZoneScoped
    return removeArticle(lower(a->artist)) < removeArticle(lower(b->artist));
}

bool SongList::sortTitle(Song *a, Song *b) {
    ZoneScoped
    return removeArticle(lower(a->title)) < removeArticle(lower(b->title));
}

bool SongList::sortPlaylist(Song *a, Song *b) {
    ZoneScoped
    // i just made some BUUUUULLLLLLLLLSHIIIIIIIIIIT.
    // this gets the end iter of the path,
    // then goes back THREE path segments to get playlist name
    // then sorts like usual, like how sortTitle works.
    // i tried to bring this to its own function but for some reason it didnt work
    // and it works like this (sorta) and im so fucking scared #tbh
    // they made mental hospitals for the people who could concieve such a thing
    // /Path/To/SongFolder/SongName/notes.mid/ (end)
    // ..............^-------|--------|---------|
    // return *------a->midiPath.end() < *------b->midiPath.end();
    return a->GetPlaylistPath() < b->GetPlaylistPath();
}

bool SongList::sortSource(Song *a, Song *b) {
    ZoneScoped;
    std::string aLower = TextToLower(a->source.c_str());
    std::string bLower = TextToLower(b->source.c_str());
    return aLower < bLower;
}

bool SongList::sortAlbum(Song *a, Song *b) {
    ZoneScoped;
    std::string aLower = TextToLower(a->album.c_str());
    std::string bLower = TextToLower(b->album.c_str());
    std::string aaa = removeArticle(TextToLower(a->album.c_str()));
    std::string bbb = removeArticle(TextToLower(b->album.c_str()));
    return aaa < bbb;
}

bool SongList::sortLen(Song *a, Song *b) {
    return a->length < b->length;
}

bool SongList::sortYear(Song *a, Song *b) {
    return a->releaseYear < b->releaseYear;
}

SongList::SongList() {
}

SongList::~SongList() {
}

void SongList::sortList(SortType sortType) {
    ZoneScoped
    sectionEntries.clear();
    sortedSongs.clear();
    for (auto &song : songs) {
        sortedSongs.push_back(&song);
    }
    switch (sortType) {
    case SortType::Title:
        std::sort(sortedSongs.begin(), sortedSongs.end(), sortTitle);
        break;
    case SortType::Artist:
        // std::sort(sortedSongs.begin(), sortedSongs.end(), sortAlbum);
        std::sort(sortedSongs.begin(), sortedSongs.end(), sortArtist);
        break;
    case SortType::Source:
        // std::sort(sortedSongs.begin(), sortedSongs.end(), sortTitle);
        std::sort(sortedSongs.begin(), sortedSongs.end(), sortSource);
        break;
    case SortType::Length:
        std::sort(sortedSongs.begin(), sortedSongs.end(), sortLen);
        break;
    case SortType::Year:
        // std::sort(sortedSongs.begin(), sortedSongs.end(), sortTitle);
        std::sort(sortedSongs.begin(), sortedSongs.end(), sortYear);
        break;
    case SortType::Playlist:
        // std::sort(sortedSongs.begin(), sortedSongs.end(), sortTitle);
        std::sort(sortedSongs.begin(), sortedSongs.end(), sortPlaylist);
        break;
    default: ;
    }
    GenerateSongEntriesWithHeaders(sortType);
}

void SongList::WriteCache() {
    ZoneScoped
    std::filesystem::remove(cachePath());

    // Native-endian order used for best performance, since the cache is not a portable
    // file
    encore::bin_ofstream_native SongCache(cachePath(), std::ios::binary);

    // Casts used to explicitly indicate type
    SongCache << (uint32_t)SONG_CACHE_HEADER;
    SongCache << (uint32_t)SONG_CACHE_VERSION;
    SongCache << (size_t)songs.size();

    for (const auto &song : songs) {
        SongCache << song.songDir.string();
        SongCache << song.albumArtPath.string();
        SongCache << song.songInfoPath.string();
        SongCache << song.midiPath.string();
        SongCache << song.hash;
        std::ifstream songInfo(song.songInfoPath);
        std::ostringstream sstr;
        sstr << songInfo.rdbuf();
        SongCache << sstr.str();

        //Encore::EncoreLog(LOG_INFO, TextFormat("CACHE: Song found:     %s - %s", song.title.c_str(), song.artist.c_str()));
        //Encore::EncoreLog(LOG_INFO, TextFormat("CACHE: Directory:      %s", song.songDir.c_str()));
        //Encore::EncoreLog(LOG_INFO, TextFormat("CACHE: Album Art Path: %s", song.albumArtPath.c_str()));
        //Encore::EncoreLog(LOG_INFO, TextFormat("CACHE: Song Info Path: %s", song.songInfoPath.c_str()));
        //Encore::EncoreLog(LOG_INFO, TextFormat("CACHE: Song length:    %01i", song.length));
    }

    SongCache.close();
}

ThreadPool *scanPool;

void SongList::ScanFolder(const std::filesystem::path &folder, std::wofstream &badSongs) {
    ZoneScoped
    if (!std::filesystem::is_directory(folder)) {
        return;
    }

    directoryCount++;

    auto infoPath = folder / "song.ini";
    ++FolderCount;

    if (std::filesystem::exists(infoPath)) {
        ++SongCount;
        try {
            if (std::filesystem::exists(folder / "notes.mid")) {
                Song song;
                song.songInfoPath = infoPath;
                song.songDir = folder;
                song.LoadSongIni(folder);
                auto placedSong = &songs.emplace_back(std::move(song));
                scanPool->SubmitTask([folder, placedSong]() {
                    ZoneScopedN("Hash Song")
                    std::ifstream hashStream(folder / "notes.mid", std::ios::binary);
                    unsigned char hash[picosha2::k_digest_size] = { 0 };
                    picosha2::hash256(hashStream, hash, hash + picosha2::k_digest_size);
                    memcpy(&placedSong->hash, hash, picosha2::k_digest_size);
                    ++SongsHashed;
                });
            } else {
                badSongs << "Song does not have notes.mid" << std::endl;
                badSongs << folder << std::endl << std::endl;
                ++BadSongCount;
            }
        } catch (const std::exception &e) {
            // I give up. If your song includes any fucked up characters and can't,
            // for some reason, be scanned, we're just putting you on the bad songs list.
            // God. I fucking hate C actually. Both C and C++. Both ends.
            // How do people manage with this bullshit?
            ++BadSongCount;
            badSongs << e.what() << std::endl;
            badSongs << folder << std::endl << std::endl;
        }
    } else {
        // If this folder doesn't have song.ini, this must be a organizational folder; continue scanning.
        for (const auto &entry : std::filesystem::directory_iterator(folder)) {
            ScanFolder(entry.path(), badSongs);
        };
    }

    songCount++;
}

void SongList::ScanSongs(const std::vector<std::filesystem::path> &songsFolder) {
    ZoneScoped
    ScanningSongs = true;
    Clear();
    std::vector<std::filesystem::path> folders{ "./Songs" };
    for (const auto &folder : songsFolder) {
        folders.push_back(folder);
    }
    scanPool = new ThreadPool(std::thread::hardware_concurrency() - 1);

    std::wofstream badSongs(badSongsPath(),
                            std::ios::out | std::ios::trunc | std::ios::binary);

    badSongs.imbue(std::locale(badSongs.getloc(),
                               new std::codecvt_utf16<
                                   wchar_t, 0x10FFFF, std::little_endian>));
    badSongs << "Please open in Notepad++ or any editor that detects UTF16" << std::endl
        << std::endl;

    for (const auto &folder : folders) {
        if (!is_directory(folder)) {
            continue;
        }

        ScanFolder(folder, badSongs);
    }

    delete scanPool;

    PopulateHashIndex();
    Encore::EncoreLog(LOG_INFO, "CACHE: Rewriting song cache");
    WriteCache();
    sortList(SortType::Title);
    curSong = nullptr;
    badSongs.close();
    ScanningSongs = false;
}

std::string GetLengthHeader(int length) {
    if (length < 60000)
        return "< 1:00";
    if (length < 120000)
        return "1:00-2:00";
    if (length < 180000)
        return "2:00-3:00";
    if (length < 240000)
        return "3:00-4:00";
    if (length < 300000)
        return "4:00-5:00";
    return "5:00+";
}

void SongList::GenerateSongEntriesWithHeaders(SortType sortType) {
    listMenuEntries.clear();
    std::string currentHeader = "";
    int pos = 0;
    for (size_t i = 0; i < sortedSongs.size(); i++) {
        Song *song = sortedSongs[i];
        std::string header;
        switch (sortType) {
        case SortType::Title: {
            std::string title = removeArticle(TextToLower(song->title.c_str()));
            header = title.empty() ? "#" : std::string(1, toupper(title[0]));
            break;
        }
        case SortType::Artist: {
            std::string artist = removeArtistJunk(TextToLower(song->artist.c_str()));
            header = artist.empty() ? "#" : artist;
            break;
        }
        case SortType::Source: {
            std::string source = song->source;
            header = source.empty() ? "Unknown" : source;
            break;
        }
        case SortType::Length: {
            header = GetLengthHeader(song->length);
            break;
        }
        case SortType::Year: {
            header = song->releaseYear.empty() ? "Unknown Year" : song->releaseYear;
            break;
        }
        case SortType::Playlist: {
            // What.
            // /Path/To/SongFolder/SongName/notes.mid/ (end)
            // ..............^-------|--------|---------|
            header = song->GetPlaylistPath(); // (------song->midiPath.end())->string();
            break;
        }
        default:
            header = "#";
            break;
        }
        if (lower(header) != lower(currentHeader)) {
            currentHeader = header;
            if (!sectionEntries.empty()) {
                sectionEntries.back().lastListID = listMenuEntries.size() - 1;
            }
            sectionEntries.emplace_back(listMenuEntries.size());
            if (sortType == SortType::Artist) {
                listMenuEntries.emplace_back(true, 0, song->artist, false);
            } else {
                listMenuEntries.emplace_back(true, 0, currentHeader, false);
            }
            pos++;
        }
        listMenuEntries.emplace_back(false, i, "", false);
        song->songListPos = listMenuEntries.size();
        pos++;
    }
    if (!listMenuEntries.empty()) {
        sectionEntries.back().lastListID = listMenuEntries.size();
    }
    listMenuEntries.shrink_to_fit();
}

void SongList::LoadCache(const std::vector<std::filesystem::path> &songsFolder) {
    ZoneScoped;
    Encore::EncoreLog(LOG_INFO,
                      TextFormat("CACHE: Loading cache from %s",
                                 cachePath().generic_u8string().c_str()));
    encore::bin_ifstream_native SongCacheIn(cachePath(), std::ios::binary);
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
    std::set<std::filesystem::path> loadedSongs;
    // To track loaded songs and avoid duplicates
    //songs.reserve(cachedSongCount);
    MaxChartsToLoad = cachedSongCount;
    for (size_t i = 0; i < cachedSongCount; i++) {
        ZoneScopedN("Song")
        CurrentChartNumber = i + 1;
        Song song;

        // Read cache values
        std::string dir;
        SongCacheIn >> dir;
        song.songDir = dir;

        std::string art;
        SongCacheIn >> art;
        song.albumArtPath = art;

        std::string infopath;
        SongCacheIn >> infopath;
        song.songInfoPath = infopath;

        std::string midiPath;
        SongCacheIn >> midiPath;
        song.midiPath = midiPath;
        
        SongCacheIn >> song.hash;

        {
            ZoneScopedN("INI Parse")
            std::string iniData;
            SongCacheIn >> iniData;
            INIReader reader(iniData.c_str(), iniData.length());
            song.PullInfoFromINI(reader);
        }

        // Encore::EncoreLog(LOG_INFO, TextFormat("CACHE: Directory - %s", song.songDir.c_str()));

        {
            ZoneScopedN("Vector insert");
            if (!std::filesystem::exists(song.songDir)) {
                continue;
            }
            loadedSongs.insert(song.songDir);
            this->songs.emplace_back(song);
        }
    }

    SongCacheIn.close();
    // size_t loadedSongCount = songs.size();

    //if (cachedSongCount != loadedSongCount || songs.size() != loadedSongCount) {
    //    Encore::EncoreLog(LOG_INFO, "CACHE: Updating song cache");
    //    WriteCache();
    //}

    // ScanSongs(songsFolder);
    PopulateHashIndex();
    sortList(SortType::Title);
}
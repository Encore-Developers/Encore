#include "songlist.h"

#include <set>

#include "util/binary.h"

// sorting
void SongList::Clear() {
	listMenuEntries.clear();
	songs.clear();
	songCount = 0;
	directoryCount = 0;
	badSongCount = 0;
}

bool SongList::sortArtist(const Song& a, const Song& b) {
	std::string aLower = TextToLower(a.artist.c_str());
	std::string bLower = TextToLower(b.artist.c_str());
	return aLower < bLower;
}

bool SongList::sortTitle(const Song& a, const Song& b) {
	std::string aLower = TextToLower(a.title.c_str());
	std::string bLower = TextToLower(b.title.c_str());
	return aLower < bLower;
}

bool SongList::sortLen(const Song& a, const Song& b) {
	return a.length < b.length;
}

void SongList::sortList(SortType sortType) {
	switch (sortType) {
		case SortType::Title:
			std::sort(songs.begin(), songs.end(), sortTitle);
		break;
		case SortType::Artist:
			std::sort(songs.begin(), songs.end(), sortArtist);
		break;
		case SortType::Length:
			std::sort(songs.begin(), songs.end(), sortLen);
		break;
	}
	listMenuEntries = GenerateSongEntriesWithHeaders(songs, sortType);
}

void SongList::sortList(SortType sortType, int& selectedSong) {
	Song& curSong = songs[selectedSong];
	selectedSong = 0;
	switch (sortType) {
		case SortType::Title:
			std::sort(songs.begin(), songs.end(), sortTitle);
		break;
		case SortType::Artist:
			std::sort(songs.begin(), songs.end(), sortArtist);
		break;
		case SortType::Length:
			std::sort(songs.begin(), songs.end(), sortLen);
		break;
	}
	for (int i = 0; i < songs.size(); i++) {
		if (songs[i].artist == curSong.artist && songs[i].title == curSong.title) {
			selectedSong = i;
			break;
		}
	}
	listMenuEntries = GenerateSongEntriesWithHeaders(songs, sortType);
}

void SongList::WriteCache() {
	std::filesystem::remove("songCache.encr");

	// Native-endian order used for best performance, since the cache is not a portable file
	encore::bin_ofstream_native SongCache("songCache.encr", std::ios::binary);

	// Casts used to explicitly indicate type
	SongCache << (uint32_t)SONG_CACHE_HEADER;
	SongCache << (uint32_t)SONG_CACHE_VERSION;
	SongCache << (size_t)songs.size();

	for (const auto& song : songs) {
		SongCache << song.songDir;
		SongCache << song.albumArtPath;
		SongCache << song.songInfoPath;
		SongCache << song.jsonHash;

		SongCache << song.title;
		SongCache << song.artist;

		SongCache << song.length;

		TraceLog(LOG_INFO, TextFormat("%s - %s", song.title.c_str(), song.artist.c_str()));
		TraceLog(LOG_INFO, TextFormat("Directory - %s", song.songDir.c_str()));
		TraceLog(LOG_INFO, TextFormat("Album Art Path - %s", song.albumArtPath.c_str()));
		TraceLog(LOG_INFO, TextFormat("Song Info Path - %s", song.songInfoPath.c_str()));
		TraceLog(LOG_INFO, std::to_string(song.length).c_str());
	}

	SongCache.close();
}

void SongList::ScanSongs(const std::vector<std::filesystem::path>& songsFolder)
{
	Clear();

	for (const auto &folder : songsFolder) {
		if (!std::filesystem::is_directory(folder)) {
			continue;
		}

		for (const auto &entry: std::filesystem::directory_iterator(folder)) {
			if (!std::filesystem::is_directory(entry)) {
				continue;
			}

			directoryCount++;
			if (std::filesystem::exists(entry.path() / "info.json")) {
				Song song;
				song.LoadSong(entry.path() / "info.json");
				songs.push_back(std::move(song));
				songCount++;
			}
			else if (std::filesystem::exists(entry.path() / "song.ini")) {
				Song song;

				song.songInfoPath = (entry.path() / "song.ini").string();
				song.songDir = entry.path().string();
				song.LoadSongIni(entry.path());
				song.ini = true;
				songs.push_back(std::move(song));
				songCount++;
			}
		}
	}

	TraceLog(LOG_INFO, "Rewriting song cache");
	WriteCache();
}

std::vector<ListMenuEntry> SongList::GenerateSongEntriesWithHeaders(const std::vector<Song>& songs, SortType sortType) {
	std::vector<ListMenuEntry> songEntries;
	std::string currentHeader = "";
	int pos = 0;
	for (int i = 0; i < songs.size(); i++) {
		const Song& song = songs[i];
		switch (sortType) {
			case SortType::Title: {
				if (toupper(song.title[0]) != currentHeader[0]) {
					currentHeader = toupper(song.title[0]);
					songEntries.emplace_back(true, 0, currentHeader, false);
					pos++;
				}
				break;
			}
			case SortType::Artist: {
				if (song.artist != currentHeader) {
					currentHeader = song.artist;
					songEntries.emplace_back(true, 0, currentHeader, false);
					pos++;
				}
				break;
			}
			case SortType::Length: {
				if (std::to_string(song.length) != currentHeader) {
					currentHeader = std::to_string(song.length);
					songEntries.emplace_back(true, 0, currentHeader, false);
					pos++;
				}
				break;
			}
		}
		songEntries.emplace_back(false, i, "", false);
		pos++;
		this->songs[i].songListPos = pos;
	}

	return songEntries;
}

void SongList::LoadCache(const std::vector<std::filesystem::path>& songsFolder) {
        encore::bin_ifstream_native SongCacheIn("songCache.encr", std::ios::binary);
        if (!SongCacheIn) {
            TraceLog(LOG_WARNING, "Failed to load song cache!");
            SongCacheIn.close();
            ScanSongs(songsFolder);
            return;
        }

        // Read header
        uint32_t header;
        SongCacheIn >> header;
        if (header != SONG_CACHE_HEADER) {
            TraceLog(LOG_WARNING, "Invalid song cache format, rescanning");
            SongCacheIn.close();
            ScanSongs(songsFolder);
            return;
        }

        uint32_t version;
        SongCacheIn >> version;
        if (version != SONG_CACHE_VERSION) {
            TraceLog(LOG_WARNING, TextFormat("Cache version %01i, but current version is %01i",version, SONG_CACHE_VERSION));
            SongCacheIn.close();
            ScanSongs(songsFolder);
            return;
        }

        size_t cachedSongCount;
        SongCacheIn >> cachedSongCount;

        // Load cached songs
        Clear();
        TraceLog(LOG_INFO, "Loading song cache");
        std::set<std::string> loadedSongs;  // To track loaded songs and avoid duplicates
        for (int i = 0; i < cachedSongCount; i++) {
            Song song;

            // Read cache values
            SongCacheIn >> song.songDir;
            SongCacheIn >> song.albumArtPath;
            SongCacheIn >> song.songInfoPath;
            SongCacheIn >> song.jsonHash;

            SongCacheIn >> song.title;
            SongCacheIn >> song.artist;

            SongCacheIn >> song.length;

            TraceLog(LOG_INFO, TextFormat("Directory - %s", song.songDir.c_str()));

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

            songs.push_back(std::move(song));
            loadedSongs.insert(song.songDir);
        }

        SongCacheIn.close();
        size_t loadedSongCount = songs.size();

        // Load additional songs from directories if needed
        for (const auto& folder : songsFolder) {
            if (!std::filesystem::is_directory(folder)) {
                continue;
            }

            for (const auto& entry : std::filesystem::directory_iterator(folder)) {
                if (!std::filesystem::is_directory(entry)) {
                    continue;
                }

                if (std::filesystem::exists(entry.path() / "info.json")) {
                    if (loadedSongs.find(entry.path().string()) == loadedSongs.end()) {
                        Song song;
                        song.LoadSong(entry.path() / "info.json");
                        songs.push_back(std::move(song));
                        songCount++;
                    }
                    else {
                        badSongCount++;
                    }
                }

                if (std::filesystem::exists(entry.path() / "song.ini")) {
                    if (loadedSongs.find(entry.path().string()) == loadedSongs.end()) {
                        Song song;
                        song.songInfoPath = (entry.path() / "song.ini").string();
                        song.songDir = entry.path().string();
                        song.LoadSongIni(entry.path());
                        song.ini = true;

                        songs.push_back(std::move(song));
                        songCount++;
                    } else {
                        badSongCount++;
                    }
                }
            }
        }

        if (cachedSongCount != loadedSongCount || songs.size() != loadedSongCount) {
            TraceLog(LOG_INFO, "Updating song cache");
            WriteCache();
        }

        // ScanSongs(songsFolder);
        sortList(SortType::Title);
    }


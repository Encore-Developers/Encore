#include "cacheload.h"
#include "songlist.h"

std::atomic_bool CacheLoad::finished;
std::atomic_bool CacheLoad::started;
std::thread CacheLoad::cacheLoadThread;

void CacheLoad::StartLoad() {
    if (started) return;
    started = true;
    cacheLoadThread = std::thread([]() {
        TheSongList.LoadCache(TheGameSettings.SongPaths);
        finished = true;
    });
    cacheLoadThread.detach();
}
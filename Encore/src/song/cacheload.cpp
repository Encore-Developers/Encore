#include "cacheload.h"
#include "songlist.h"
#include "tracy/TracyC.h"

std::atomic_bool CacheLoad::finished;
std::atomic_bool CacheLoad::started;
std::thread CacheLoad::cacheLoadThread;

void CacheLoad::StartLoad() {
    if (started) return;
    started = true;
    cacheLoadThread = std::thread([]() {
        TracyCSetThreadName("Song Cache Loader")
        TheSongList.LoadCache(TheGameSettings.SongPaths);
        finished = true;
    });
    cacheLoadThread.detach();
}
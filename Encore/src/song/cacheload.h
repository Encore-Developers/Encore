#pragma once
#ifndef CACHELOAD_H
#define CACHELOAD_H
#include <atomic>
#include <thread>
#include "settings/settings.h"

namespace CacheLoad {
    extern std::atomic_bool finished;
    extern std::atomic_bool started;
    extern std::thread cacheLoadThread;

    void StartLoad();

}
#endif
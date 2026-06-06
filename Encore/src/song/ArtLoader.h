#pragma once
#include "song.h"
#include "raylib.h"
#include "util/ownedtexture.h"

#include <queue>
#include <mutex>
#include <semaphore>

class Song;

struct ArtRequest {
    Song* song;
};

struct LoadResult {
    Image loadedImage;
    bool blur;
    Image otherImage;
};

class ArtLoader {
    void ThreadRun();

    void SendResult(LoadResult result);
public:
    bool keepAlive = true;
    std::counting_semaphore<1000000> semaphore;

    std::queue<ArtRequest> requests;
    std::mutex requestsMutex;

    std::queue<LoadResult> results;
    std::mutex resultsMutex;


    Song* currentSongArt;
    std::shared_ptr<OwnedTexture> loadedArt;
    std::shared_ptr<OwnedTexture> loadedArtBlur;

    ArtLoader();

    void LoadAlbumArt(Song* song);
    void Poll();

    ~ArtLoader();
};

extern ArtLoader TheArtLoader;
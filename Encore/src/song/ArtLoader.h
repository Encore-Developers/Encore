#pragma once
#include "song.h"
#include "raylib.h"

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
    std::counting_semaphore<100> semaphore;

    std::queue<ArtRequest> requests;
    std::mutex requestsMutex;

    std::queue<LoadResult> results;
    std::mutex resultsMutex;


    Song* currentSongArt;
    Texture2D loadedArt;
    Texture2D loadedArtBlur;

    ArtLoader();

    void LoadAlbumArt(Song* song);
    void Poll();

    ~ArtLoader();
};

extern ArtLoader TheArtLoader;
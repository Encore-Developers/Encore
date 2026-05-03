#include "ArtLoader.h"
#include <thread>

#include "tracy/TracyC.h"

ArtLoader TheArtLoader;

void ArtLoader::ThreadRun() {
    TracyCSetThreadName("Album Art Loader");
    while (keepAlive) {
        semaphore.acquire();
        if (!keepAlive) {
            semaphore.release();
            return;
        }
        {
            ZoneScopedN("Lock Requests")
            requestsMutex.lock();
            if (requests.empty()) {
                requestsMutex.unlock();
                continue;
            }
        }
        ZoneScopedN("Load Album Art")
        auto request = requests.front();
        requests.pop();
        requestsMutex.unlock();
        auto image = LoadImage(request.song->albumArtPath.c_str());
        SendResult({image, false, {0}});
        auto blurred = ImageCopy(image);
        ImageResize(&blurred, 256, 256);
        ImageBlurGaussian(&blurred, 10);
        SendResult({blurred, true, image});
    }
}
void ArtLoader::SendResult(LoadResult result) {
    resultsMutex.lock();
    results.push(result);
    resultsMutex.unlock();
}
ArtLoader::ArtLoader() : semaphore(0) {
    loadedArt.id = 0;
    loadedArtBlur.id = 0;
    keepAlive = true;

    std::thread thread = std::thread([this]() { this->ThreadRun(); });
    thread.detach();
}
void ArtLoader::LoadAlbumArt(Song *song) {
    if (currentSongArt == song) {
        return;
    }
    currentSongArt = song;
    requestsMutex.lock();
    while (!requests.empty()) {
        requests.pop();
    }
    requests.push({song});
    requestsMutex.unlock();
    semaphore.release();
}
void ArtLoader::Poll() {
    ZoneScopedN("Poll Art Loader")
    if (resultsMutex.try_lock()) {
        if (results.empty()) {
            resultsMutex.unlock();
            return;
        }
        ZoneScopedN("Upload Album Art")
        auto result = results.front();
        results.pop();
        resultsMutex.unlock();

        if (!result.blur) {
            if (loadedArt.id != 0) {
                UnloadTexture(loadedArt);
            }
            loadedArt = LoadTextureFromImage(result.loadedImage);
        } else {
            if (loadedArtBlur.id != 0) {
                UnloadTexture(loadedArtBlur);
            }
            loadedArtBlur = LoadTextureFromImage(result.loadedImage);
            UnloadImage(result.loadedImage);
            UnloadImage(result.otherImage);
        }

    }
}
ArtLoader::~ArtLoader() {
    keepAlive = false;
    semaphore.release();
    semaphore.acquire(); // Wait for the thread to die
}
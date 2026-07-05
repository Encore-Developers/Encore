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

        // Delusional. All this just to support Windows.
        std::ifstream file(request.song->albumArtPath, std::ios::binary | std::ios::ate);
        int fileSize = file.tellg();
        int realFileSize = fileSize;
        file.seekg(0, std::ios::beg);

        char* fileBuffer = (char *)malloc(fileSize);
        file.read(fileBuffer, realFileSize);
        file.close();
        auto image = LoadImageFromMemory(reinterpret_cast<const char *>(request.song->albumArtPath.extension().generic_u8string().c_str()), (const unsigned char*)fileBuffer, fileSize);
        // LoadImage(request.song->albumArtPath.string().c_str());
        SendResult({image, false, {0}});
        auto blurred = ImageCopy(image);
        ImageResize(&blurred, 512, 512);
        ImageBlurGaussian(&blurred, 10);
        SendResult({blurred, true, image});
        if (fileBuffer != nullptr) {
            free(fileBuffer);
            fileBuffer = nullptr;
        }
    }
}
void ArtLoader::SendResult(LoadResult result) {
    resultsMutex.lock();
    results.push(result);
    resultsMutex.unlock();
}
ArtLoader::ArtLoader() : semaphore(0) {

    keepAlive = true;

    std::thread thread = std::thread([this]() { this->ThreadRun(); });
    thread.detach();
}
void ArtLoader::LoadAlbumArt(Song *song) {
    assert(song);
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
            loadedArt = std::make_shared<OwnedTexture>(LoadTextureFromImage(result.loadedImage));
            SetTextureFilter(*loadedArt, TEXTURE_FILTER_BILINEAR);
        } else {
            loadedArtBlur = std::make_shared<OwnedTexture>(LoadTextureFromImage(result.loadedImage));
            SetTextureFilter(*loadedArtBlur, TEXTURE_FILTER_BILINEAR);
        }

    }
}
ArtLoader::~ArtLoader() {
    keepAlive = false;
    semaphore.release();
    semaphore.acquire(); // Wait for the thread to die
}
//
// Created by marie on 02/05/2024.
//

#include "assets.h"

#include "graphicsState.h"
#include "stb_image.h"

#include <filesystem>
#include "SDL3/SDL_log.h"
#include "tracy/Tracy.hpp"
#include "tracy/TracyC.h"

#include <fstream>
#include <iostream>

class Assets;
class Asset;

Assets TheAssets; // This is the single instance

const char *AssetStateName(AssetState state) {
    switch (state) {
    case UNLOADED:
        return "UNLOADED";
    case LOADING:
        return "LOADING";
    case PREFINALIZED:
        return "PREFINALIZED";
    case LOADED:
        return "LOADED";
    }
    return "INVALID";
}
void Asset::AddToFinalizeQueue() {
    TheAssets.finalizeQueueMutex.lock();
    TheAssets.finalizeQueue.push_back(this);
    TheAssets.finalizeQueueMutex.unlock();
    state = PREFINALIZED;
}

Assets &Assets::getInstance() {
    return TheAssets;
}

void Asset::CheckForFetch() {
    if (state != LOADED) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Asset %s was fetched before it was loaded. This is no longer supported.", id.c_str());
        abort();
    }
}

void Asset::SetAssetParent(Asset *newParent) {
    parent = newParent;
    DelistAsset();
}

void Asset::DelistAsset() {
    for (auto iter = TheAssets.assets.begin(); iter != TheAssets.assets.end(); ++iter) {
        auto asset = *iter;
        if (asset == this) {
            TheAssets.assets.erase(iter);
            break;
        }
    }
}

Asset::~Asset() {
    DelistAsset();
}

Asset::Asset(const std::string &id) {
    this->id = id;
    TheAssets.assets.push_back(this);
}

void Asset::StartLoad() {
    ZoneScoped;
    if (state == UNLOADED) {
        state = LOADING;
        loadingThread = std::thread([this]() { this->Load(); });
        loadingThread.detach();
        //Encore::EncoreLog(LOG_INFO, TextFormat("Loading asset %s...", id.c_str()));
    }
}

void Asset::LoadImmediate() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Loading asset %s immediately...", id.c_str());
    StartLoad();
    while (state == LOADING) {
    }
}

void FileAsset::LoadFile() {
    std::ifstream file(GetPath(), std::ios::binary | std::ios::ate);
    fileSize = file.tellg();
    int realFileSize = fileSize;
    if (addNullTerminator)
        fileSize++;
    file.seekg(0, std::ios::beg);

    fileBuffer = (char *)malloc(fileSize);
    file.read(fileBuffer, realFileSize);
    file.close();
    if (addNullTerminator)
        fileBuffer[fileSize - 1] = '\0';
}

void FileAsset::FreeFileBuffer() {
    if (fileBuffer != nullptr) {
        free(fileBuffer);
        fileBuffer = nullptr;
    }
}

const std::filesystem::path FileAsset::GetBaseDirectory() {
    return TheAssets.getDirectory();
}

void FileAsset::Load() {
    LoadFile();
    state = LOADED;
}

void FileAsset::Unload() {
    FreeFileBuffer();
    state = UNLOADED;
}

size_t FileAsset::GetFileSize() {
    CheckForFetch();
    return fileSize;
}

char *FileAsset::FetchRaw() {
    CheckForFetch();
    return fileBuffer;
}

void TextureAsset::Load() {
    ZoneScoped
    LoadFile();
    int channelsInFile = 0;
    data = (Pixel*)stbi_load_from_memory((stbi_uc*) fileBuffer, fileSize, &width, &height, &channelsInFile, 4);
    FreeFileBuffer();

    SDL_GPUTransferBufferCreateInfo createInfo = {
        SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        (unsigned int)(width*height*sizeof(Pixel)),
        0
    };
    transferBuffer = SDL_CreateGPUTransferBuffer(TheGPU, &createInfo);
    Pixel* buf = (Pixel*)SDL_MapGPUTransferBuffer(TheGPU, transferBuffer, false);
    memcpy(buf, data, width * height * sizeof(Pixel));
    SDL_UnmapGPUTransferBuffer(TheGPU, transferBuffer);


    AddToFinalizeQueue();
}

void TextureAsset::Finalize(SDL_GPUCopyPass* copyPass) {
    ZoneScoped
    SDL_GPUTextureCreateInfo createInfo = {
        SDL_GPU_TEXTURETYPE_2D,
        SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
        SDL_GPU_TEXTUREUSAGE_SAMPLER,
        (unsigned int)width,
        (unsigned int)height,
        1,
        1,
        SDL_GPU_SAMPLECOUNT_1,
        0
    };
    texture = SDL_CreateGPUTexture(TheGPU, &createInfo);
    SDL_GPUTextureTransferInfo transferInfo = {
        transferBuffer,
        0,
        (unsigned int)width,
        (unsigned int)height
    };
    SDL_GPUTextureRegion region = {
        texture,
        0,
        0,
        0,
        0,
        0,
        (unsigned int)width,
        (unsigned int)height,
        1
    };
    SDL_UploadToGPUTexture(copyPass, &transferInfo, &region, false);
    state = LOADED;
}

void TextureAsset::Unload() {

}

void Assets::AddRingsAndInstruments() {
    // for (int i = 1; i <= 6; i++) {
    //     TextureAsset *tex = new TextureAsset(TextFormat("ui/hugh ring/rings-%i.png", i),
    //                                          true);
    //     YargRings.push_back(tex);
    //     // Grabbing from the vec because it moved
    //     mainMenuSet.AddAsset(tex);
    // }
    // InstIcons.push_back(new TextureAsset("ui/hugh ring/drums-inv.png", true));
    // InstIcons.push_back(new TextureAsset("ui/hugh ring/bass-inv.png", true));
    // InstIcons.push_back(new TextureAsset("ui/hugh ring/lead-inv.png", true));
    // InstIcons.push_back(new TextureAsset("ui/hugh ring/keys-inv.png", true));
    // InstIcons.push_back(new TextureAsset("ui/hugh ring/vox-inv.png", true));
    // for (auto icon : InstIcons) {
    //     mainMenuSet.AddAsset(icon);
    // }
}

#pragma once

#include "SDL3/SDL_filesystem.h"
#include "SDL3/SDL_gpu.h"
#include "SDL3_shadercross/SDL_shadercross.h"

#include <cassert>
#include <atomic>
#include <deque>
#include <mutex>
#include <filesystem>
#include <functional>
#include <map>
#include <thread>
#include <unordered_map>
#include <vector>

enum AssetState : uint8_t {
    UNLOADED,
    LOADING,
    PREFINALIZED,
    LOADED
};

const char *AssetStateName(AssetState state);

class Asset {
protected:
    virtual void Load() {
    }

    void AddToFinalizeQueue();

    SDL_GPUTransferBuffer* transferBuffer = nullptr;

    //Asset(Asset& other) {}
public:
    AssetState state = UNLOADED;
    std::string id = {};
    std::thread loadingThread;
    /// Used for assets created by another and not stored in TheAssets (ShaderAsset)
    Asset *parent = nullptr;

    Asset(const std::string &id);

    /// Empty constructor provided so vectors can initialize. Do not use!
    Asset() = default;

    /// Starts loading this asset.
    virtual void StartLoad();

    /// Starts loading this asset and blocks until it is loaded.
    virtual void LoadImmediate();
    /// Checks if this asset is loaded. Only use in the render thread!
    void CheckForFetch();
    void SetAssetParent(Asset *newParent);

    /// Removes the asset from the internal list, hiding it from the debug Assets window
    void DelistAsset();

    /// Unloads this asset. Only use in render thread!
    virtual void Unload() {
    }

    virtual void Finalize(SDL_GPUCopyPass* copyPass) {
    }
    /// Call when you're polling the asset's for when it's loaded.
    bool CanFetch() const {
        return state == LOADED;
    }

    ~Asset();
};

class FileAsset : public Asset {
protected:
    size_t fileSize;
    void LoadFile();
    virtual void Load();
    void FreeFileBuffer();

public:
    char *fileBuffer;
    bool addNullTerminator = false;

    static const std::filesystem::path GetBaseDirectory();

    FileAsset(const std::string &id)
        : Asset(id) {
    }

    FileAsset() {
    }

    virtual void Unload();

    std::filesystem::path GetPath() {
        return GetBaseDirectory() / id;
    }

    size_t GetFileSize();
    char *FetchRaw();

    operator const unsigned char *() {
        return (const unsigned char *)FetchRaw();
    }

    virtual ~FileAsset() {
        //FreeFileBuffer();
    }

    FileAsset(FileAsset &&other) noexcept
        : fileSize(other.fileSize),
          fileBuffer(other.fileBuffer) {
    }
};

typedef struct Pixel {
    uint8_t r, g, b, a;
} Pixel;

class TextureAsset : public FileAsset {
    virtual void Load();

    void CopyToTransferBuffer();

public:
    int width = 0;
    int height = 0;
    Pixel* data;
    SDL_GPUTexture* texture;
    bool keepRawData = false;
    int mips = 1;

    TextureAsset(const std::string &id, bool keepRawData = false, int mips = 1)
        : FileAsset(id), keepRawData(keepRawData), mips(mips) {
    }
    virtual void Finalize(SDL_GPUCopyPass* copyPass);

    TextureAsset() {
    }

    virtual void Unload();
};


class ShaderAsset : public FileAsset {
    virtual void Load();

public:
    SDL_ShaderCross_ShaderStage stage;
    SDL_GPUShader* shader;

    ShaderAsset(const std::string &id, SDL_ShaderCross_ShaderStage stage) : FileAsset(id), stage(stage) {
        addNullTerminator = true;
    }

    ShaderAsset() {}

    operator SDL_GPUShader*() {
        return shader;
    }

    virtual void Unload();
};

#define ASSET(varname) TheAssets.varname
#define ASSETPTR(varname) &TheAssets.varname

#define NEWFILEASSET(varname, path) FileAsset varname = FileAsset(path)
#define NEWTEXASSET(varname, path) TextureAsset varname = TextureAsset(path)
#define NEWTEXASSET_KEEPRAW(varname, path) TextureAsset varname = TextureAsset(path, true)
#define FRAG SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT
#define VERT SDL_SHADERCROSS_SHADERSTAGE_VERTEX
#define NEWSHADERASSET(varname, path, stage) ShaderAsset varname = ShaderAsset(path, stage)


class Assets {
private:
    std::filesystem::path directory = SDL_GetBasePath() / std::filesystem::path("Assets");

public:
    std::vector<Asset *> assets; // Stored for debugging
    std::deque<Asset *> finalizeQueue;
    std::mutex finalizeQueueMutex;
    Assets() {
    }


    NEWTEXASSET_KEEPRAW(faviconTex, "encore_favicon-NEW.png");
    NEWSHADERASSET(testVert, "testshaders/test.vert.hlsl", VERT);
    NEWSHADERASSET(testFrag, "testshaders/test.frag.hlsl", FRAG);

    void AddRingsAndInstruments();

    static Assets &getInstance();

    void setDirectory(std::filesystem::path assetsDirectory) {
        directory = assetsDirectory;
    }

    std::filesystem::path getDirectory() {
        return directory;
    }

    Assets(const Assets &) = delete;
    void operator=(const Assets &) = delete;
};

extern Assets TheAssets;

/// Used for easily loading groups of assets and polling their state as one.
class AssetSet {
    std::vector<Asset *> assets;

public:
    AssetSet(std::initializer_list<Asset *> l) {
        assets = std::vector<Asset *>(l);
    }

    void AddAsset(Asset *asset) {
        assets.push_back(asset);
    }

    void StartLoad() {
        for (long unsigned int i = 0; i < assets.size(); i++) {
            auto asset = assets[i];
            if (asset->state == UNLOADED) {
                asset->StartLoad();
            }
        }
    }

    bool PollLoaded() {
        bool loaded = true;
        for (long unsigned int i = 0; i < assets.size(); i++) {
            auto asset = assets[i];
            if (!asset->CanFetch()) {
                loaded = false;
            }
        }
        return loaded;
    }

    int CountLoaded() {
        int loaded = 0;
        for (long unsigned int i = 0; i < assets.size(); i++) {
            if (assets[i]->CanFetch()) {
                loaded++;
            }
        }
        return loaded;
    }

    int AssetCount() {
        return assets.size();
    }

    float GetProgress() {
        return static_cast<float>(CountLoaded()) / static_cast<float>(AssetCount());
    }

    void BlockUntilLoaded() {
        while (!PollLoaded()) {
            usleep(1);
        }
    }
};

extern AssetSet initialSet;
extern AssetSet mainMenuSet;
extern AssetSet gameplaySet;

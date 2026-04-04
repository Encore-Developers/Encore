#pragma once

#include <cassert>
#include <atomic>
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

    virtual void Finalize() {
    }

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

    /// Call when you're polling the asset's for when it's loaded.
    bool CanFetch() const {
        return state == LOADED || state == PREFINALIZED;
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



#define ASSET(varname) TheAssets.varname
#define ASSETPTR(varname) &TheAssets.varname




class Assets {
private:
    std::filesystem::path directory = "Assets";

public:
    std::vector<Asset *> assets; // Stored for debugging
    Assets() {
    }

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

    bool PollLoaded(bool doFinalize = false) {
        bool loaded = true;
        for (long unsigned int i = 0; i < assets.size(); i++) {
            auto asset = assets[i];
            if (!asset->CanFetch()) {
                loaded = false;
            }
            if (doFinalize && asset->state == PREFINALIZED) {
                asset->CheckForFetch();
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
        }
        for (long unsigned int i = 0; i < assets.size(); i++) {
            // This finalizes any assets that need it
            // We're blocking anyways so why not
            assets[i]->CheckForFetch();
        }
    }
};

extern AssetSet initialSet;
extern AssetSet mainMenuSet;
extern AssetSet gameplaySet;

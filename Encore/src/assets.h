#pragma once

#include "hb.h"
#include "SDL3/SDL_filesystem.h"
#include "SDL3/SDL_gpu.h"
#include "SDL3_shadercross/SDL_shadercross.h"
#include "math/glm.h"

#include <cassert>
#include <atomic>
#include <deque>
#include <mutex>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <thread>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <unordered_set>
#include "nlohmann/json.hpp"
#include "util/threadpool.h"

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

    void DoFinalize();

    SDL_GPUTransferBuffer* transferBuffer = nullptr;

    //Asset(Asset& other) {}
public:
    AssetState state = UNLOADED;
    std::string id = {};
    bool noPreload = false;
    /// Used for assets created by another and not stored in TheAssets (ShaderAsset)
    Asset *parent = nullptr;

    Asset(const nlohmann::json &info);

    /// Empty constructor provided so vectors can initialize. Do not use!
    Asset() = default;

    /// Starts loading this asset.
    virtual void StartLoad();

    /// Starts loading this asset and blocks until it is loaded.
    virtual void LoadImmediate();
    /// Checks if this asset is loaded. Only use in the render thread!
    void CheckForFetch();
    void SetAssetParent(Asset *newParent);

    void BlockUntilLoaded();

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
    std::string path = {};

    static const std::filesystem::path GetBaseDirectory();

    FileAsset(const nlohmann::json &info)
        : Asset(info), path(info["path"]) {
    }

    // Used for temporary assets while loading shaders
    FileAsset(const std::string& path) : Asset(), path(path) {

    }

    FileAsset() {
    }

    virtual void Unload();

    std::filesystem::path GetPath() {
        return GetBaseDirectory() / path;
    }
    std::unique_ptr<std::istream> GetStream(std::ios::openmode mode) {
        return std::make_unique<std::ifstream>(GetPath(), mode);
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

    bool rawDataLoaded = false;

    TextureAsset(const nlohmann::json &info)
        : FileAsset(info), keepRawData(info["keepRawData"]), mips(info["mips"]) {
    }
    virtual void Finalize(SDL_GPUCopyPass* copyPass);

    TextureAsset() {
    }

    virtual void Unload();

    operator SDL_GPUTexture*() {
        return texture;
    }
};

class FontAsset : public FileAsset {
    virtual void Load();

    hb_blob_t* blob;
    hb_face_t* face;
public:
    int faceIndex = 0;

    FontAsset(const nlohmann::json &info)
        : FileAsset(info) {
        if (info.contains("faceIndex")) {
            faceIndex = info["faceIndex"];
        }
    }
    virtual void Finalize(SDL_GPUCopyPass* copyPass) {};

    FontAsset() {
    }

    virtual void Unload();
};


class ShaderAsset : public FileAsset {
    virtual void Load();
    void GenerateVertexInputState(SDL_ShaderCross_GraphicsShaderMetadata* metadata);

    static void PreprocessShader(std::string &input, std::unordered_set<std::string>* includedFiles = nullptr);

    std::vector<SDL_GPUVertexBufferDescription> bufferDescriptions;
    std::vector<SDL_GPUVertexAttribute> vertexAttributes;
    SDL_GPUShader* shader = nullptr;
public:
    SDL_ShaderCross_ShaderStage stage;
    SDL_GPUVertexInputState vertexInputState = {};

    static SDL_ShaderCross_ShaderStage StageFromString(const std::string &str) {
        if (str == "vertex") {
            return SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
        }
        if (str == "fragment") {
            return SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
        }
        if (str == "computer") {
            return SDL_SHADERCROSS_SHADERSTAGE_COMPUTE;
        }
        throw std::runtime_error("Unknown shader stage " + str);
    }

    ShaderAsset(const nlohmann::json &info) : FileAsset(info) {
        addNullTerminator = true;
        stage = StageFromString(info["stage"]);
    }

    ShaderAsset() {}

    operator SDL_GPUShader*() const {
        return shader;
    }

    virtual void Unload();
};

struct MeshVertex {
    Vector3 position;
    Vector2 uv;
};

struct Face {
    unsigned int indices[3];
};

class MeshAsset : public FileAsset {
    virtual void Load();
    virtual void Finalize(SDL_GPUCopyPass *copyPass);

    void CopyToTransferBuffer();
public:

    unsigned int vertexBufferSize;
    SDL_GPUBuffer* vertexBuffer;
    unsigned int indexBufferSize;
    SDL_GPUBuffer* indexBuffer;

    unsigned int numVertices;
    unsigned int numFaces;

    std::vector<MeshVertex> vertices;
    std::vector<Face> faces;

    MeshAsset(const nlohmann::json &info) : FileAsset(info) {
        addNullTerminator = true;
    }

    MeshAsset() {}

    virtual void Unload();
};

#define GET_ASSET_STATIC(type, id) static std::shared_ptr<type> id = TheAssets.GetAsset<type>(#id);

class Assets {
private:
    std::filesystem::path directory = SDL_GetBasePath() / std::filesystem::path("Assets");

public:
    std::unordered_map<std::string, std::shared_ptr<Asset>> assets;
    std::deque<Asset *> finalizeQueue;
    std::mutex finalizeQueueMutex;
    ThreadPool loadingPool = std::thread::hardware_concurrency()-1;
    Assets() {
    }


    void LoadAssetSet(const nlohmann::json &set);
    void Init();

    void AddRingsAndInstruments();

    static Assets &getInstance();

    void setDirectory(std::filesystem::path assetsDirectory) {
        directory = assetsDirectory;
    }

    std::filesystem::path getDirectory() {
        return directory;
    }

    template<class T>
    std::shared_ptr<T> GetAsset(const std::string& id) {
        auto asset = assets.find(id);
        if (asset == assets.end()) {
            throw std::runtime_error("Asset not found: " + id);
        }
        if (auto typed = std::dynamic_pointer_cast<T>(asset->second)) {
            return typed;
        }
        throw std::runtime_error("Cannot get asset " + id + " as " + typeid(T).name());
    }

    Assets(const Assets &) = delete;
    void operator=(const Assets &) = delete;
};

extern Assets TheAssets;

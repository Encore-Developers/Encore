//
// Created by marie on 02/05/2024.
//

#include "assets.h"

#include "graphicsState.h"
#include "stb_image.h"

#include <filesystem>
#include "SDL3/SDL_log.h"
#include "SDL3_shadercross/SDL_shadercross.h"
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
    auto file = GetStream(std::ios::binary | std::ios::ate);
    fileSize = file->tellg();
    int realFileSize = fileSize;
    if (addNullTerminator)
        fileSize++;
    file->seekg(0, std::ios::beg);

    fileBuffer = (char *)malloc(fileSize);
    file->read(fileBuffer, realFileSize);
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

    // Allow textures to be loaded even when the GPU is not initialized yet, while still preferring to copy to the
    // transfer buffer on the asset thread.
    if (TheGPU) {
        CopyToTransferBuffer();
    }

    AddToFinalizeQueue();
}

void TextureAsset::Finalize(SDL_GPUCopyPass* copyPass) {
    ZoneScoped

    if (!transferBuffer) {
        CopyToTransferBuffer();
    }

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
    SDL_ReleaseGPUTransferBuffer(TheGPU, transferBuffer);
    transferBuffer = nullptr;
    state = LOADED;
}

void TextureAsset::CopyToTransferBuffer() {
    SDL_GPUTransferBufferCreateInfo transferCreateInfo = {
        SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        (unsigned int)(width*height*sizeof(Pixel)),
        0
    };
    transferBuffer = SDL_CreateGPUTransferBuffer(TheGPU, &transferCreateInfo);
    Pixel* buf = (Pixel*)SDL_MapGPUTransferBuffer(TheGPU, transferBuffer, false);
    memcpy(buf, data, width * height * sizeof(Pixel));
    SDL_UnmapGPUTransferBuffer(TheGPU, transferBuffer);
    if (!keepRawData) {
        stbi_image_free(data);
        data = nullptr;
    }
}

void TextureAsset::Unload() {
    SDL_ReleaseGPUTexture(TheGPU, texture);
    texture = nullptr;
    state = UNLOADED;
}
void ShaderAsset::Load() {
    ZoneScopedN("Shader Compile")
    LoadFile();

    size_t size = 0;
    SDL_ShaderCross_HLSL_Info hlslInfo = {
        .source = fileBuffer,
        .entrypoint = "main",
        .include_dir = nullptr,
        .defines = nullptr,
        .shader_stage = stage,
        .props = 0
    };
    const uint8_t *spirv =
        (const uint8_t *)SDL_ShaderCross_CompileSPIRVFromHLSL(&hlslInfo, &size);

    if (spirv == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to compile shader: %s", SDL_GetError());
        return;
    }

    SDL_ShaderCross_SPIRV_Info spirvInfo = {
        .bytecode = spirv,
        .bytecode_size = size,
        .entrypoint = "main",
        .shader_stage = stage,
        .props = 0

    };
    auto resourceInfo = SDL_ShaderCross_ReflectGraphicsSPIRV(spirv, size, 0);
    shader = SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(TheGPU, &spirvInfo, &resourceInfo->resource_info, 0);
    state = LOADED;
    FreeFileBuffer();
    SDL_free((void*)spirv);
    SDL_free(resourceInfo);
}
void ShaderAsset::Unload() {
    SDL_ReleaseGPUShader(TheGPU, shader);
}
void MeshAsset::Load() {
    ZoneScopedN("Mesh Load")
    std::vector<Vector3> positions;
    std::vector<Vector2> uvs;
    auto stream = GetStream(std::ios::in);


    for ( std::string line; std::getline(*stream, line);) {
        std::istringstream lineStream(line);
        std::string type;
        lineStream >> type;
        //std::cout << "type: " << type << std::endl;

        if (type == "v") {
            Vector3 pos;
            lineStream >> pos.x >> pos.y >> pos.z;
            positions.push_back(pos);
        }
        if (type == "vt") {
            Vector2 uv;
            lineStream >> uv.x >> uv.y;
            uv.y = 1 - uv.y;
            uvs.push_back(uv);
        }
        if (type == "f") {
            std::vector<Vector3> thisPositions;
            std::vector<int> indices;
            while (!lineStream.eof()) {
                std::string indexEntry;
                lineStream >> indexEntry;
                std::string positionIndexStr;
                std::string uvIndexStr;
                int i;
                for (i = 0; indexEntry[i] != '/'; i++) {
                    positionIndexStr += indexEntry[i];
                }
                i++;
                for (; i < indexEntry.size() && indexEntry[i] != '/'; i++) {
                    uvIndexStr += indexEntry[i];
                }
                int positionIndex = std::stoi(positionIndexStr)-1;
                int uvIndex = std::stoi(uvIndexStr)-1;

                // This doesn't reuse vertices when faces share them with equivalent UVs.
                // TODO: Merge vertices if they share a position and UV
                // Maybe as a post-processing step?
                MeshVertex vert;
                vert.position = positions[positionIndex];
                vert.uv = uvs[uvIndex];
                indices.push_back(vertices.size());
                vertices.push_back(vert);
            }
            for (int i = 1; i < indices.size()-1; i++) {
                Face face;
                face.indices[0] = indices[0];
                face.indices[1] = indices[i];
                face.indices[2] = indices[i+1];
                faces.push_back(face);
            }
        }
    }

    if (TheGPU) {
        CopyToTransferBuffer();
    }

    AddToFinalizeQueue();
}
void MeshAsset::Finalize(SDL_GPUCopyPass *copyPass) {
    if (!transferBuffer) {
        CopyToTransferBuffer();
    }

    SDL_GPUTransferBufferLocation vertLoc = {
        transferBuffer,
        0
    };
    SDL_GPUBufferRegion vertDest = {
        vertexBuffer,
        0,
        vertexBufferSize
    };
    SDL_GPUTransferBufferLocation indexLoc = {
        transferBuffer,
        vertexBufferSize
    };
    SDL_GPUBufferRegion indexDest = {
        indexBuffer,
        0,
        indexBufferSize
    };
    SDL_UploadToGPUBuffer(copyPass, &vertLoc, &vertDest, false);
    SDL_UploadToGPUBuffer(copyPass, &indexLoc, &indexDest, false);

    SDL_ReleaseGPUTransferBuffer(TheGPU, transferBuffer);
    transferBuffer = nullptr;

    state = LOADED;
}
void MeshAsset::CopyToTransferBuffer() {
    SDL_GPUTransferBufferCreateInfo createInfo = {
        SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        (unsigned int)(sizeof(MeshVertex) * vertices.size() + sizeof(Face) * faces.size()),
        0
    };
    transferBuffer = SDL_CreateGPUTransferBuffer(TheGPU, &createInfo);
    auto data = SDL_MapGPUTransferBuffer(TheGPU, transferBuffer, false);
    memcpy(data, vertices.data(), sizeof(MeshVertex) * vertices.size());
    memcpy((char*)data + sizeof(MeshVertex) * vertices.size(), faces.data(), sizeof(Face) * faces.size());
    SDL_UnmapGPUTransferBuffer(TheGPU, transferBuffer);
    vertexBufferSize = sizeof(MeshVertex) * vertices.size();
    indexBufferSize = sizeof(Face) * faces.size();
    numVertices = vertices.size();
    numFaces = faces.size();
    vertices.clear();
    faces.clear();
    vertices.shrink_to_fit();
    faces.shrink_to_fit();

    SDL_GPUBufferCreateInfo vertCreateInfo = {
        SDL_GPU_BUFFERUSAGE_VERTEX,
        vertexBufferSize,
        0
    };
    vertexBuffer = SDL_CreateGPUBuffer(TheGPU, &vertCreateInfo);

    SDL_GPUBufferCreateInfo indexCreateInfo = {
        SDL_GPU_BUFFERUSAGE_INDEX,
        indexBufferSize,
        0
    };
    indexBuffer = SDL_CreateGPUBuffer(TheGPU, &indexCreateInfo);
}
void MeshAsset::Unload() {

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

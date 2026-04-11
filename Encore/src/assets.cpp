//
// Created by marie on 02/05/2024.
//

#include "assets.h"

#include "graphicsState.h"
#include "stb_image.h"
#include "tiny_obj_loader.h"

#include <filesystem>
#include "SDL3/SDL_log.h"
#include "SDL3_shadercross/SDL_shadercross.h"
#include "tracy/Tracy.hpp"
#include "tracy/TracyC.h"

#include <fstream>
#include <iostream>
#include <regex>
#include <unordered_set>

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
void Asset::BlockUntilLoaded() {
    while (state == LOADED) {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
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
    //SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Loading asset %s immediately...", id.c_str());
    Load();
    assert(state == LOADED);
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

    rawDataLoaded = true;

    BlockUntilGPUReady();
    CopyToTransferBuffer();

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
void ShaderAsset::PreprocessShader(std::string &input, std::unordered_set<std::string>* includedFiles) {
    static const std::regex includeRegex("#include <(.+?)>");
    std::smatch match;

    bool ownsIncludeSet = false;
    if (!includedFiles) {
        includedFiles = new std::unordered_set<std::string>();
        ownsIncludeSet = true;
    }

    while (std::regex_search(input, match, includeRegex), match[0].matched) {
        auto file = std::string(match[1]);
        if (includedFiles->contains(file)) {
            continue;
        }
        includedFiles->insert(file);
        FileAsset includedFile("shader_includes/" + file);
        includedFile.addNullTerminator = true;
        includedFile.LoadImmediate();
        std::string includeContents = includedFile.fileBuffer;
        includedFile.Unload();
        PreprocessShader(includeContents, includedFiles);
        input.replace(match[0].first, match[0].second, includeContents.c_str());
    }

    if (ownsIncludeSet) {
        delete includedFiles;
    }
}
void ShaderAsset::Load() {
    ZoneScopedN("Shader Compile")
    LoadFile();

    std::string shaderContents(fileBuffer);
    FreeFileBuffer();

    PreprocessShader(shaderContents);

    size_t size = 0;
    SDL_ShaderCross_HLSL_Info hlslInfo = {
        .source = shaderContents.c_str(),
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
    if (stage == SDL_SHADERCROSS_SHADERSTAGE_VERTEX) {
        GenerateVertexInputState(resourceInfo);
    }
    BlockUntilGPUReady();
    shader = SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(TheGPU, &spirvInfo, &resourceInfo->resource_info, 0);
    state = LOADED;
    SDL_free((void*)spirv);
    SDL_free(resourceInfo);
}

// Only use vertex attribute types with this macro in vector sizes of 2 or 4.
// In fact, these ones are rather buggy. Avoid them!
#define ONLY_2_OR_4(type) (SDL_GPUVertexElementFormat)(type-1)


const SDL_GPUVertexElementFormat SHADERCROSS_TYPE_TO_SDL[] = {
    SDL_GPU_VERTEXELEMENTFORMAT_INVALID, //SDL_SHADERCROSS_IOVAR_TYPE_UNKNOWN
    ONLY_2_OR_4(SDL_GPU_VERTEXELEMENTFORMAT_BYTE2), //SDL_SHADERCROSS_IOVAR_TYPE_INT8
    ONLY_2_OR_4(SDL_GPU_VERTEXELEMENTFORMAT_UBYTE2), //SDL_SHADERCROSS_IOVAR_TYPE_UINT8
    ONLY_2_OR_4(SDL_GPU_VERTEXELEMENTFORMAT_SHORT2), //SDL_SHADERCROSS_IOVAR_TYPE_INT16
    ONLY_2_OR_4(SDL_GPU_VERTEXELEMENTFORMAT_USHORT2), //SDL_SHADERCROSS_IOVAR_TYPE_UINT16
    SDL_GPU_VERTEXELEMENTFORMAT_INT, //SDL_SHADERCROSS_IOVAR_TYPE_INT32
    SDL_GPU_VERTEXELEMENTFORMAT_UINT, //SDL_SHADERCROSS_IOVAR_TYPE_UINT32
    SDL_GPU_VERTEXELEMENTFORMAT_INT2, //SDL_SHADERCROSS_IOVAR_TYPE_INT64
    SDL_GPU_VERTEXELEMENTFORMAT_UINT2, //SDL_SHADERCROSS_IOVAR_TYPE_UINT64
    ONLY_2_OR_4(SDL_GPU_VERTEXELEMENTFORMAT_HALF2), //SDL_SHADERCROSS_IOVAR_TYPE_FLOAT16
    SDL_GPU_VERTEXELEMENTFORMAT_FLOAT, //SDL_SHADERCROSS_IOVAR_TYPE_FLOAT32
    SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2 //SDL_SHADERCROSS_IOVAR_TYPE_FLOAT64
};

const unsigned int SHADERCROSS_TYPE_TO_SIZE[] = {
    0, //SDL_SHADERCROSS_IOVAR_TYPE_UNKNOWN
    1, //SDL_SHADERCROSS_IOVAR_TYPE_INT8
    1, //SDL_SHADERCROSS_IOVAR_TYPE_UINT8
    2, //SDL_SHADERCROSS_IOVAR_TYPE_INT16
    2, //SDL_SHADERCROSS_IOVAR_TYPE_UINT16
    4, //SDL_SHADERCROSS_IOVAR_TYPE_INT32
    4, //SDL_SHADERCROSS_IOVAR_TYPE_UINT32
    8, //SDL_SHADERCROSS_IOVAR_TYPE_INT64
    8, //SDL_SHADERCROSS_IOVAR_TYPE_UINT64
    2, //SDL_SHADERCROSS_IOVAR_TYPE_FLOAT16
    4, //SDL_SHADERCROSS_IOVAR_TYPE_FLOAT32
    8  //SDL_SHADERCROSS_IOVAR_TYPE_FLOAT64
};

void ShaderAsset::GenerateVertexInputState(SDL_ShaderCross_GraphicsShaderMetadata* metadata) {
    unsigned int curOffset = 0;
    SDL_GPUVertexBufferDescription curDesc = {
        .slot = 0,
        .pitch = 0,
        .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
        .instance_step_rate = 0
    };
    for (int i = 0; i < metadata->num_inputs; i++) {
        auto& input = metadata->inputs[i];
        if (std::string(input.name).starts_with("in.var.INSTANCEINPUT")) {
            // Split the buffer
            if (curDesc.input_rate == SDL_GPU_VERTEXINPUTRATE_VERTEX) {
                // Reset to fill a new buffer
                bufferDescriptions.push_back(curDesc);
                curDesc.input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE;
                curDesc.slot += 1;
                curDesc.pitch = 0;
                curOffset = 0;
            }
        }
        SDL_GPUVertexAttribute attribute = {};
        attribute.format = (SDL_GPUVertexElementFormat)(SHADERCROSS_TYPE_TO_SDL[input.vector_type]+input.vector_size-1);
        attribute.buffer_slot = curDesc.slot;
        attribute.location = input.location;
        attribute.offset = curOffset;
        curOffset += input.vector_size*SHADERCROSS_TYPE_TO_SIZE[input.vector_type];
        curDesc.pitch = curOffset;
        vertexAttributes.push_back(attribute);
    }
    bufferDescriptions.push_back(curDesc);
    vertexInputState = {
        .vertex_buffer_descriptions = bufferDescriptions.data(),
        .num_vertex_buffers = (unsigned int)bufferDescriptions.size(),
        .vertex_attributes = vertexAttributes.data(),
        .num_vertex_attributes = (unsigned int)vertexAttributes.size(),
    };
}

void ShaderAsset::Unload() {
    SDL_ReleaseGPUShader(TheGPU, shader);
}
void MeshAsset::Load() {
    ZoneScopedN("Mesh Load")
    LoadFile();
    std::string contents(fileBuffer);
    FreeFileBuffer();

    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig config;
    config.triangulate = true;
    reader.ParseFromString(contents, "", config);

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();

    for (auto& shape: shapes) {
        auto& mesh = shape.mesh;
        auto& points = shape.points;

        int indexOffset = 0;

        for (int i = 0; i < shape.mesh.num_face_vertices.size(); i++) {
            int faceVerts = shape.mesh.num_face_vertices[i];

            static std::vector<unsigned int> indices;
            indices.reserve(3);
            indices.clear();

            for (int j = 0; j < faceVerts; j++) {
                auto idx = shape.mesh.indices[indexOffset + j];

                auto vx = attrib.vertices[3*idx.vertex_index+0];
                auto vy = attrib.vertices[3*idx.vertex_index+1];
                auto vz = attrib.vertices[3*idx.vertex_index+2];

                float tx = 0;
                float ty = 0;

                if (idx.texcoord_index >= 0) {
                    tx = attrib.texcoords[2*idx.texcoord_index+0];
                    ty = attrib.texcoords[2*idx.texcoord_index+1];
                }

                indices.push_back(vertices.size());
                MeshVertex vert {
                    {vx, vy, vz},
                    {tx, 1-ty}
                };
                vertices.push_back(vert);
            }
            faces.push_back(Face {indices[0], indices[1], indices[2]});
            indexOffset += faceVerts;
        }
    }

    numVertices = vertices.size();
    numFaces = faces.size();

    BlockUntilGPUReady();
    CopyToTransferBuffer();

    AddToFinalizeQueue();
}
void MeshAsset::Finalize(SDL_GPUCopyPass *copyPass) {

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

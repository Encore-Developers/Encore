#pragma once
#include "graphicsState.h"
#include "SDL3/SDL_gpu.h"

#include <vector>

/// Helper to populate and auto-resize a vertex buffer.
/// Used for instancing.
template <typename T>
class GPUDynamicBuffer {
    SDL_GPUBuffer* buffer = nullptr;
    SDL_GPUTransferBuffer* transferBuffer = nullptr;
    size_t currentBufferCapacity = 0;
    std::vector<T> data;
    SDL_GPUBufferUsageFlags usage;
    bool cycle;

    void ResizeBuffer(size_t newCap) {
        if (buffer) {
            SDL_ReleaseGPUBuffer(TheGPU, buffer);
            buffer = nullptr;
        }
        if (transferBuffer) {
            SDL_ReleaseGPUTransferBuffer(TheGPU, transferBuffer);
            transferBuffer = nullptr;
        }
        SDL_GPUBufferCreateInfo bufCreateInfo = {
            usage,
            (unsigned int)(sizeof(T) * newCap)
        };
        buffer = SDL_CreateGPUBuffer(TheGPU, &bufCreateInfo);

        SDL_GPUTransferBufferCreateInfo transCreateInfo = {
            SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            (unsigned int)(sizeof(T) * newCap)
        };
        transferBuffer = SDL_CreateGPUTransferBuffer(TheGPU, &transCreateInfo);
        currentBufferCapacity = newCap;
    }
public:

    GPUDynamicBuffer(size_t initialAlloc, SDL_GPUBufferUsageFlags usage, bool cycle) : usage(usage), cycle(cycle) {
        data.reserve(initialAlloc);
        ResizeBuffer(initialAlloc);
    }

    void Reset() {
        data.clear();
    }

    void Push(T value) {
        data.push_back(value);
        if (data.capacity() != currentBufferCapacity) {
            ResizeBuffer(data.capacity());
        }
    }

    size_t Size() {
        return data.size();
    }

    SDL_GPUBufferBinding GetBinding() const {
        return {
            .buffer = buffer, .offset = 0
        };
    }

    void UploadData(SDL_GPUCopyPass* copyPass) {
        if (data.empty()) {
            return;
        }

        void* mappedBuffer = SDL_MapGPUTransferBuffer(TheGPU, transferBuffer, cycle);
        memcpy(mappedBuffer, data.data(), data.size() * sizeof(T));
        SDL_UnmapGPUTransferBuffer(TheGPU, transferBuffer);

        SDL_GPUTransferBufferLocation sourceLoc = {
            transferBuffer,
            0
        };
        SDL_GPUBufferRegion destLoc = {
            buffer,
            0,
            (unsigned int)(sizeof(T) * data.size()),
        };
        SDL_UploadToGPUBuffer(copyPass, &sourceLoc, &destLoc, cycle);
    }
};
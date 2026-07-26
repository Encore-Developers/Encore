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
    SDL_GPUBufferUsageFlags usage;
    bool cycle;
public:

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
        if (!buffer)
        {
            std::cout << "Failed to create GPU buffer: " << SDL_GetError() << std::endl;
        }

        SDL_GPUTransferBufferCreateInfo transCreateInfo = {
            SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            (unsigned int)(sizeof(T) * newCap)
        };
        transferBuffer = SDL_CreateGPUTransferBuffer(TheGPU, &transCreateInfo);
        if (!transferBuffer)
        {
            std::cout << "Failed to create GPU transfer buffer: " << SDL_GetError() << std::endl;
        }
        currentBufferCapacity = newCap;
    }

    GPUDynamicBuffer(SDL_GPUBufferUsageFlags usage, bool cycle) : usage(usage), cycle(cycle) {}

    size_t Capacity() {
        return currentBufferCapacity;
    }

    SDL_GPUBufferBinding GetBinding() const {
        return {
            .buffer = buffer, .offset = 0
        };
    }

    SDL_GPUBuffer* GetBuffer() const
    {
        return buffer;
    }

    operator SDL_GPUBufferBinding()
    {
        return GetBinding();
    }

    void UploadData(SDL_GPUCopyPass* copyPass, const std::vector<T>& data) {
        if (data.size() > currentBufferCapacity)
        {
            ResizeBuffer(data.size());
        }
        if (data.empty()) {
            return;
        }

        void* mappedBuffer = SDL_MapGPUTransferBuffer(TheGPU, transferBuffer, cycle);
        if (!mappedBuffer)
        {
            std::cout << "Failed to map GPU transfer buffer: " << SDL_GetError() << std::endl;
            return;
        }
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

    ~GPUDynamicBuffer()
    {
        if (buffer)
        {
            SDL_ReleaseGPUBuffer(TheGPU, buffer);
        }
        if (transferBuffer)
        {
            SDL_ReleaseGPUTransferBuffer(TheGPU, transferBuffer);
        }
    }
};
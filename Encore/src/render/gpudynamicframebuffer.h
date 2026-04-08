#pragma once
#include "graphicsState.h"
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_render.h"

class GPUDynamicFramebuffer {
public:
    SDL_GPUTexture *colorTexture = nullptr;
    SDL_GPUTexture *depthTexture = nullptr;
    SDL_GPUTexture *resolveTexture = nullptr;
    SDL_GPUSampleCount sampleCount;
    bool forceDirty = false;
    unsigned int width = -1;
    unsigned int height = -1;

    GPUDynamicFramebuffer(SDL_GPUSampleCount sampleCount) : sampleCount(sampleCount) {}

    void Resize(unsigned int width, unsigned int height);
    void Update(unsigned int width, unsigned int height);

    void SetSampleCount(SDL_GPUSampleCount sampleCount);

    SDL_GPUStoreOp GetStoreOp() {
        if (sampleCount == SDL_GPU_SAMPLECOUNT_1) {
            return SDL_GPU_STOREOP_STORE;
        }
        return SDL_GPU_STOREOP_RESOLVE;
    }

    SDL_GPUTexture *GetBlitSource() {
        if (sampleCount == SDL_GPU_SAMPLECOUNT_1) {
            return colorTexture;
        }
        return resolveTexture;
    }
};

extern GPUDynamicFramebuffer* TheFramebuffer;
#pragma once
#include "graphicsState.h"
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_render.h"
/// An object that automatically manages target textures for rendering MSAA.
class GPUDynamicFramebuffer {
    void Resize(unsigned int width, unsigned int height);
public:
    SDL_GPUTexture *colorTexture = nullptr;
    SDL_GPUTexture *depthTexture = nullptr;
    SDL_GPUTexture *resolveTexture = nullptr;
    SDL_GPUSampleCount sampleCount;
    bool forceDirty = false;
    unsigned int width = -1;
    unsigned int height = -1;

    GPUDynamicFramebuffer(SDL_GPUSampleCount sampleCount) : sampleCount(sampleCount) {}

    /// Call every frame right after obtaining the swapchain texture.
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

    SDL_GPUColorTargetInfo GetColorTargetInfo() {
        SDL_GPUColorTargetInfo colorTargetInfo = {};
        colorTargetInfo.texture = colorTexture;
        colorTargetInfo.resolve_texture = resolveTexture;
        colorTargetInfo.store_op = GetStoreOp();
        return colorTargetInfo;
    }

    SDL_GPUDepthStencilTargetInfo GetDepthStencilTargetInfo() {
        SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo = {};
        depthStencilTargetInfo.texture = depthTexture;
        return depthStencilTargetInfo;
    }
};

extern GPUDynamicFramebuffer* TheFramebuffer;
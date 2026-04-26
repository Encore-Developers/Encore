#pragma once
#include "SDL3/SDL_gpu.h"
#include "math/glm.h"

class GPUCompositor {
    SDL_GPURenderPass* renderPass = nullptr;
    SDL_GPUSampler* sampler = nullptr;
    SDL_GPUCommandBuffer* commandBuffer = nullptr;
public:
    Color clearColor = {0.3, 0.4, 0.5, 1.0};
    bool showAlphaDebug = false;
    void BeginCompositing(SDL_GPUCommandBuffer* cmdbuf, SDL_GPUTexture* target);
    void AddTexture(SDL_GPUTexture* texture);
    void EndCompositing();
};
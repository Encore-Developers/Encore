#include "gpudynamicframebuffer.h"

void GPUDynamicFramebuffer::Resize(unsigned int width, unsigned int height) {
    this->width = width;
    this->height = height;
    if (colorTexture) {
        SDL_ReleaseGPUTexture(TheGPU, colorTexture);
        colorTexture = nullptr;
    }
    if (depthTexture) {
        SDL_ReleaseGPUTexture(TheGPU, depthTexture);
        depthTexture = nullptr;
    }
    if (resolveTexture) {
        SDL_ReleaseGPUTexture(TheGPU, resolveTexture);
        resolveTexture = nullptr;
    }
    SDL_GPUTextureCreateInfo colorCreateInfo = {
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GetGPUSwapchainTextureFormat(TheGPU, TheWindow),
        .usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | (sampleCount == SDL_GPU_SAMPLECOUNT_1 ? SDL_GPU_TEXTUREUSAGE_SAMPLER : 0),
        .width = width,
        .height = height,
        .layer_count_or_depth = 1,
        .num_levels = 1,
        .sample_count = sampleCount
    };
    colorTexture = SDL_CreateGPUTexture(TheGPU, &colorCreateInfo);
    SDL_GPUTextureCreateInfo depthCreateInfo = {
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
        .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
        .width = width,
        .height = height,
        .layer_count_or_depth = 1,
        .num_levels = 1,
        .sample_count = sampleCount
    };
    depthTexture = SDL_CreateGPUTexture(TheGPU, &depthCreateInfo);
    SDL_GPUTextureCreateInfo resolveCreateInfo = {
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GetGPUSwapchainTextureFormat(TheGPU, TheWindow),
        .usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER,
        .width = width,
        .height = height,
        .layer_count_or_depth = 1,
        .num_levels = 1,
    };
    resolveTexture = SDL_CreateGPUTexture(TheGPU, &resolveCreateInfo);
}
void GPUDynamicFramebuffer::Update(unsigned int width, unsigned int height) {
    if (width != this->width || height != this->height || forceDirty) {
        Resize(width, height);
    }
}
void GPUDynamicFramebuffer::SetSampleCount(SDL_GPUSampleCount sampleCount) {
    this->sampleCount = sampleCount;
    forceDirty = true;
}
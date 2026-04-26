#include "gpucompositor.h"

#include "graphicsState.h"
#include "pipelines.h"

void GPUCompositor::BeginCompositing(
    SDL_GPUCommandBuffer *cmdbuf, SDL_GPUTexture *target
) {
    SDL_GPUColorTargetInfo targetInfo = {
        .texture = target,
        .clear_color = ColorToSDLColor(clearColor),
        .load_op = SDL_GPU_LOADOP_CLEAR,
        .store_op = SDL_GPU_STOREOP_STORE,
        .cycle = false
    };
    if (sampler == nullptr) {
        SDL_GPUSamplerCreateInfo samplerCreate = {
            .min_filter = SDL_GPU_FILTER_LINEAR,
            .mag_filter = SDL_GPU_FILTER_NEAREST,
            .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
            .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
            .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE
        };
        sampler = SDL_CreateGPUSampler(TheGPU, &samplerCreate);
    }
    renderPass = SDL_BeginGPURenderPass(cmdbuf, &targetInfo, 1, nullptr);
    SDL_BindGPUGraphicsPipeline(renderPass, PIPELINE(compositeLayerPipeline));
    SDL_PushGPUFragmentUniformData(cmdbuf, 0, &showAlphaDebug, 8);
}

void GPUCompositor::AddTexture(SDL_GPUTexture *texture) {
    SDL_GPUTextureSamplerBinding binding = {
        .texture = texture,
        .sampler = sampler
    };
    SDL_BindGPUFragmentSamplers(renderPass, 0, &binding, 1);
    SDL_DrawGPUPrimitives(renderPass, 4, 1, 0, 0);
}

void GPUCompositor::EndCompositing() {
    SDL_EndGPURenderPass(renderPass);
    renderPass = nullptr;
}
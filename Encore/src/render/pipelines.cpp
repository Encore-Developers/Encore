#include "pipelines.h"

#include "assets.h"
#include "gpudynamicframebuffer.h"
#include "graphicsState.h"

#include <thread>

void PipelineManager::ClearPipeline(SDL_GPUGraphicsPipeline **pipelinePtr) {
    if (*pipelinePtr) {
        SDL_ReleaseGPUGraphicsPipeline(TheGPU, *pipelinePtr);
        *pipelinePtr = nullptr;
    }
}

#define CREATEPIPELINE(pipeline) pipeline = SDL_CreateGPUGraphicsPipeline(TheGPU, &pipelineCreateInfo); \
    if (!pipeline) {SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create pipeline %s: %s\n", SDL_GetError(), #pipeline); return;}

void PipelineManager::CompileAll() {
    pipelinesLoaded = false;
    ClearPipeline(&notePipeline);
    BlockUntilGPUReady();

    // Generic state values used in multiple pipelines
    SDL_GPURasterizerState genericRasterizerState = {
        .fill_mode = SDL_GPU_FILLMODE_FILL,
        .cull_mode = SDL_GPU_CULLMODE_BACK,
        .front_face = SDL_GPU_FRONTFACE_CLOCKWISE,
    };
    SDL_GPUDepthStencilState genericDepthStencilState = {
        .compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL,
        .compare_mask = 0xff,
        .write_mask = 0xff,
        .enable_depth_test = true,
        .enable_depth_write = true,
    };
    SDL_GPUColorTargetDescription colorTargetDescription = {
        .format = SDL_GetGPUSwapchainTextureFormat(TheGPU, TheWindow)
    };
    SDL_GPUGraphicsPipelineTargetInfo genericTargetInfo = {
        .color_target_descriptions = &colorTargetDescription,
        .num_color_targets = 1,
        .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
        .has_depth_stencil_target = true
    };
    SDL_GPUMultisampleState genericMultisampleState = {
        .sample_count = TheFramebuffer->sampleCount,
        .enable_alpha_to_coverage = true
    };

    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {};

    AssetSet{ASSETPTR(testVert), ASSETPTR(testFrag)}.BlockUntilLoaded();
    pipelineCreateInfo = {
        .vertex_shader = ASSET(testVert),
        .fragment_shader = ASSET(testFrag),
        .vertex_input_state = ASSET(testVert).vertexInputState,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .rasterizer_state = genericRasterizerState,
        .multisample_state = genericMultisampleState,
        .depth_stencil_state = genericDepthStencilState,
        .target_info = genericTargetInfo,
    };

    CREATEPIPELINE(notePipeline)

    pipelinesLoaded = true;
}
void PipelineManager::CompileThreaded() {
    pipelinesLoaded = false;
    std::thread compileThread([this]() {
        CompileAll();
    });
    compileThread.detach();
}
void PipelineManager::BlockUntilLoaded() {
    while (pipelinesLoaded) {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}
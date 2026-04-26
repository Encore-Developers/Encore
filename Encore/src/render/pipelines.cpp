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

#define CREATEPIPELINE(pipeline) ClearPipeline(&pipeline); \
    pipeline = SDL_CreateGPUGraphicsPipeline(TheGPU, &pipelineCreateInfo); \
    if (!pipeline) {SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create pipeline %s: %s\n", SDL_GetError(), #pipeline); return;}

#define WAITFORSHADERS(shaderOne, shaderTwo) AssetSet{ASSETPTR(shaderOne), ASSETPTR(shaderTwo)}.BlockUntilLoaded()

void PipelineManager::CompileAll() {
    pipelinesLoaded = false;
    BlockUntilGPUReady();

    // Generic state values used in multiple pipelines
    SDL_GPURasterizerState genericRasterizerState = {
        .fill_mode = SDL_GPU_FILLMODE_FILL,
        .cull_mode = SDL_GPU_CULLMODE_BACK,
        .front_face = SDL_GPU_FRONTFACE_CLOCKWISE,
    };
    SDL_GPURasterizerState generic2DRasterizerState = {
        .fill_mode = SDL_GPU_FILLMODE_FILL,
        .cull_mode = SDL_GPU_CULLMODE_BACK,
        .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
    };
    SDL_GPUDepthStencilState genericDepthStencilState = {
        .compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL,
        .compare_mask = 0xff,
        .write_mask = 0xff,
        .enable_depth_test = true,
        .enable_depth_write = true,
    };
    SDL_GPUDepthStencilState generic2DDepthStencilState = {
        .enable_depth_test = false,
        .enable_depth_write = false,
    };
    SDL_GPUColorTargetBlendState genericAlphaBlendStateOverwrite = {
        .src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
        .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ZERO,
        .color_blend_op = SDL_GPU_BLENDOP_ADD,
        .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
        .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ZERO,
        .alpha_blend_op = SDL_GPU_BLENDOP_MAX,
        .enable_blend = true
    };
    SDL_GPUColorTargetBlendState genericAlphaBlendState = {
        .src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
        .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        .color_blend_op = SDL_GPU_BLENDOP_ADD,
        .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
        .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
        .alpha_blend_op = SDL_GPU_BLENDOP_MAX,
        .enable_blend = true
    };
    SDL_GPUColorTargetBlendState genericAlphaPremultipliedBlendState = {
        .src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
        .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        .color_blend_op = SDL_GPU_BLENDOP_ADD,
        .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
        .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
        .alpha_blend_op = SDL_GPU_BLENDOP_MAX,
        .enable_blend = true
    };
    SDL_GPUColorTargetDescription colorTargetDescription = {
        .format = The3DFramebuffer->colorTextureFormat,
        .blend_state = genericAlphaBlendState
    };
    SDL_GPUColorTargetDescription color2DTargetDescription = {
        .format = The2DFramebuffer->colorTextureFormat,
        .blend_state = genericAlphaBlendState
    };
    SDL_GPUGraphicsPipelineTargetInfo genericTargetInfo = {
        .color_target_descriptions = &colorTargetDescription,
        .num_color_targets = 1,
        .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
        .has_depth_stencil_target = true
    };
    SDL_GPUGraphicsPipelineTargetInfo generic2DTargetInfo = {
        .color_target_descriptions = &color2DTargetDescription,
        .num_color_targets = 1,
        .has_depth_stencil_target = false
    };
    SDL_GPUColorTargetDescription compositorTargetDescription = {
        .format = SDL_GetGPUSwapchainTextureFormat(TheGPU, TheWindow),
        .blend_state = genericAlphaPremultipliedBlendState
    };
    SDL_GPUGraphicsPipelineTargetInfo compositorTargetInfo = {
        .color_target_descriptions = &compositorTargetDescription,
        .num_color_targets = 1,
        .has_depth_stencil_target = false
    };
    SDL_GPUMultisampleState genericMultisampleState = {
        .sample_count = The3DFramebuffer->sampleCount,
        .enable_alpha_to_coverage = false
    };
    SDL_GPUMultisampleState generic2DMultisampleState = {
        .sample_count = The2DFramebuffer->sampleCount,
        .enable_alpha_to_coverage = false
    };

    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {};

    WAITFORSHADERS(noteFrag, noteVert);
    pipelineCreateInfo = {
        .vertex_shader = ASSET(noteVert),
        .fragment_shader = ASSET(noteFrag),
        .vertex_input_state = ASSET(noteVert).vertexInputState,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .rasterizer_state = genericRasterizerState,
        .multisample_state = genericMultisampleState,
        .depth_stencil_state = genericDepthStencilState,
        .target_info = genericTargetInfo,
    };
    CREATEPIPELINE(notePipeline)

    WAITFORSHADERS(boxFrag, boxVert);
    pipelineCreateInfo = {
        .vertex_shader = ASSET(boxVert),
        .fragment_shader = ASSET(boxFrag),
        .vertex_input_state = ASSET(boxVert).vertexInputState,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP,
        .rasterizer_state = generic2DRasterizerState,
        .multisample_state = generic2DMultisampleState,
        .depth_stencil_state = generic2DDepthStencilState,
        .target_info = generic2DTargetInfo
    };
    CREATEPIPELINE(boxPipeline);

    WAITFORSHADERS(compositeFrag, compositeVert);
    pipelineCreateInfo = {
        .vertex_shader = ASSET(compositeVert),
        .fragment_shader = ASSET(compositeFrag),
        .vertex_input_state = ASSET(compositeVert).vertexInputState,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP,
        .rasterizer_state = generic2DRasterizerState,
        .multisample_state = {
            .sample_count = SDL_GPU_SAMPLECOUNT_1,
            .enable_alpha_to_coverage = false
        },
        .depth_stencil_state = generic2DDepthStencilState,
        .target_info = compositorTargetInfo
    };
    CREATEPIPELINE(compositeLayerPipeline);

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
    while (!pipelinesLoaded) {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}
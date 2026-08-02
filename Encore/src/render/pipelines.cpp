#include "pipelines.h"

#include "assets.h"
#include "gpudynamicframebuffer.h"
#include "graphicsState.h"

#include <thread>

using json = nlohmann::json;

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

    static std::unordered_map<std::string, SDL_GPUCompareOp> compareOps {
        {"never", SDL_GPU_COMPAREOP_NEVER},
        {"less", SDL_GPU_COMPAREOP_LESS},
        {"equal", SDL_GPU_COMPAREOP_EQUAL},
        {"lessOrEqual", SDL_GPU_COMPAREOP_LESS_OR_EQUAL},
        {"greater", SDL_GPU_COMPAREOP_GREATER},
        {"notEqual", SDL_GPU_COMPAREOP_NOT_EQUAL},
        {"greaterOrEqual", SDL_GPU_COMPAREOP_GREATER_OR_EQUAL},
        {"always", SDL_GPU_COMPAREOP_ALWAYS}
    };

    static std::unordered_map<std::string, SDL_GPUPrimitiveType> primitiveTypes {
        {"triangleList", SDL_GPU_PRIMITIVETYPE_TRIANGLELIST},
        {"triangleStrip", SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP},
        {"lineList", SDL_GPU_PRIMITIVETYPE_LINELIST},
        {"lineStrip", SDL_GPU_PRIMITIVETYPE_LINESTRIP},
        {"pointList", SDL_GPU_PRIMITIVETYPE_POINTLIST}
    };

    static std::unordered_map<std::string, SDL_GPUFillMode> fillModes {
        {"fill", SDL_GPU_FILLMODE_FILL},
        {"line", SDL_GPU_FILLMODE_LINE}
    };

    static std::unordered_map<std::string, SDL_GPUCullMode> cullModes {
        {"none", SDL_GPU_CULLMODE_NONE},
        {"front", SDL_GPU_CULLMODE_FRONT},
        {"back", SDL_GPU_CULLMODE_BACK},
    };

    static std::unordered_map<std::string, SDL_GPUFrontFace> frontFaces {
        {"cw", SDL_GPU_FRONTFACE_CLOCKWISE},
        {"ccw", SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE},
    };

    static std::unordered_map<std::string, SDL_GPUColorTargetBlendState> blendModes {
        {"normal", {
            .src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
            .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
            .color_blend_op = SDL_GPU_BLENDOP_ADD,
            .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
            .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
            .alpha_blend_op = SDL_GPU_BLENDOP_MAX,
            .enable_blend = true
        }},
        {"add", {
            .src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
            .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
            .color_blend_op = SDL_GPU_BLENDOP_ADD,
            .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ZERO,
            .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
            .alpha_blend_op = SDL_GPU_BLENDOP_MAX,
            .enable_blend = true
        }},
        {"normalPremultiplied", {
            .src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
            .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
            .color_blend_op = SDL_GPU_BLENDOP_ADD,
            .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
            .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
            .alpha_blend_op = SDL_GPU_BLENDOP_MAX,
            .enable_blend = true
        }}
    };

    std::unordered_map<std::string, SDL_GPUGraphicsPipelineCreateInfo> pipelinePresets {
        {"2d", {
            .rasterizer_state = generic2DRasterizerState,
            .multisample_state = generic2DMultisampleState,
            .depth_stencil_state = generic2DDepthStencilState,
            .target_info = generic2DTargetInfo
        }},
        {"3d", {
            .rasterizer_state = genericRasterizerState,
            .multisample_state = genericMultisampleState,
            .depth_stencil_state = genericDepthStencilState,
            .target_info = genericTargetInfo,
        }},
        {"compositor", {
            .rasterizer_state = generic2DRasterizerState,
            .multisample_state = {
                .sample_count = SDL_GPU_SAMPLECOUNT_1,
                .enable_alpha_to_coverage = false
            },
            .depth_stencil_state = generic2DDepthStencilState,
            .target_info = compositorTargetInfo
        }}
    };

    std::ifstream pipelinesFile(TheAssets.getDirectory() / "pipelines.jsonc");
    json defs = nlohmann::json::parse(pipelinesFile, nullptr, true, true);
    for (auto& [key, value] : defs["pipelines"].items()) {

        SDL_GPUGraphicsPipelineCreateInfo build = pipelinePresets[value["preset"]];
        auto vert = TheAssets.GetAsset<ShaderAsset>(value["vertexShader"]);
        auto frag = TheAssets.GetAsset<ShaderAsset>(value["fragmentShader"]);
        vert->BlockUntilLoaded();
        frag->BlockUntilLoaded();
        build.vertex_shader = *vert;
        build.fragment_shader = *frag;
        build.vertex_input_state = vert->vertexInputState;
        build.primitive_type = primitiveTypes[value["primitiveType"]];

        auto colorTargetDesc = build.target_info.color_target_descriptions[0];
        if (value.contains("blendMode"))
            colorTargetDesc.blend_state = blendModes[value["blendMode"]];
        build.target_info.color_target_descriptions = &colorTargetDesc;

        if (value.contains("rasterizerState")) {
            auto &rs = value["rasterizerState"];
            if (rs.contains("fillMode"))
                build.rasterizer_state.fill_mode = fillModes[rs["fillMode"]];
            if (rs.contains("cullMode"))
                build.rasterizer_state.cull_mode = cullModes[rs["cullMode"]];
            if (rs.contains("frontFace"))
                build.rasterizer_state.front_face = frontFaces[rs["frontFace"]];
        }

        if (value.contains("depthStencilState")) {
            auto &ds = value["depthStencilState"];
            if (ds.contains("enableDepthTest"))
                build.depth_stencil_state.enable_depth_test = ds["enableDepthTest"];
            if (ds.contains("enableDepthWrite"))
                build.depth_stencil_state.enable_depth_write = ds["enableDepthWrite"];
        }

        auto pipeline = SDL_CreateGPUGraphicsPipeline(TheGPU, &build);
        if (!pipeline) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create pipeline %s: %s\n", SDL_GetError(), key.c_str());
            continue;
        }

        auto exists = pipelines.find(key);
        if (exists == pipelines.end()) {
            pipelines.emplace(key, pipeline);
        } else {
            SDL_ReleaseGPUGraphicsPipeline(TheGPU, exists->second.pipeline);
            exists->second.pipeline = pipeline;
        }
    }

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

#ifdef balls

void PipelineManager::ClearPipeline(SDL_GPUGraphicsPipeline **pipelinePtr) {
    if (*pipelinePtr) {
        SDL_ReleaseGPUGraphicsPipeline(TheGPU, *pipelinePtr);
        *pipelinePtr = nullptr;
    }
}

#define CREATEPIPELINE(pipeline) ClearPipeline(&pipeline); \
    {static auto props = SDL_CreateProperties(); \
    SDL_SetStringProperty(props, SDL_PROP_GPU_GRAPHICSPIPELINE_CREATE_NAME_STRING, #pipeline); \
    pipelineCreateInfo.props = props;} \
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
#endif
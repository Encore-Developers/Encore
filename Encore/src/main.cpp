
#include "imgui.h"
#include "imgui/backends/imgui_impl_sdlgpu3.h"
#include "imgui/backends/imgui_impl_sdl3.h"
#include "SDL3/SDL.h"
#include "math/glm.h"
#include "tracy/Tracy.hpp"
#include "graphicsState.h"
#include "assets.h"
#include "math/camera.h"

#include <deque>

SDL_GPUDevice* TheGPU = nullptr;
SDL_Window* TheWindow = nullptr;

void LocateDevAssets() {
    auto execPath = std::filesystem::path(SDL_GetBasePath());
    for (int i = 0; i < 5; i++) {
        execPath /= "..";
        execPath = std::filesystem::canonical(execPath);
        //Encore::EncoreLog(LOG_INFO, TextFormat("Scanning: %s", execPath.c_str()));
        if (std::filesystem::exists(execPath / "CMakeLists.txt")) {
            execPath = std::filesystem::canonical(execPath / "Encore/Assets/");
            SDL_Log("Found dev directory: %s", execPath.c_str());
            TheAssets.setDirectory(execPath);
            break;
        }
    }
}

struct UBO {
    Matrix viewMat;
    Vector2 uvOffset;
};

int main(int argc, char *argv[]) {
    SDL_SetAppMetadata("Encore", "v0.2.0", "encore");
    LocateDevAssets();
    SDL_Log("Asset path: %s", TheAssets.getDirectory().c_str());
    //SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11");
    ASSET(faviconTex).StartLoad();
    ASSET(testMesh).StartLoad();
    ASSET(testMeshTex).StartLoad();

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD)) {
        return 1;
    }

    auto window = SDL_CreateWindow("Encore", 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    TheWindow = window;

    while (ASSET(faviconTex).state != PREFINALIZED) {}
    auto icon = SDL_CreateSurfaceFrom(ASSET(faviconTex).width, ASSET(faviconTex).height, SDL_PIXELFORMAT_RGBA32, ASSET(faviconTex).data, ASSET(faviconTex).width*sizeof(Pixel));
    if (!SDL_SetWindowIcon(window, icon)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could set icon: %s\n", SDL_GetError());
    }

    if (window == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    auto gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL, true, nullptr);
    if (gpu == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create GPU: %s\n", SDL_GetError());
        return 1;
    }

    TheGPU = gpu;


    SDL_ClaimWindowForGPUDevice(gpu, window);
    SDL_SetGPUSwapchainParameters(gpu, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);
    SDL_SetGPUAllowedFramesInFlight(TheGPU, 1);

    ASSET(testVert).StartLoad();
    ASSET(testFrag).StartLoad();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    io.Fonts->AddFontDefaultVector();

    ImGui::StyleColorsDark();

    float mainScale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    auto style = ImGui::GetStyle();
    style.ScaleAllSizes(mainScale);
    style.FontScaleDpi = mainScale;

    ImGui_ImplSDL3_InitForSDLGPU(window);
    ImGui_ImplSDLGPU3_InitInfo init_info = {};
    init_info.Device = gpu;
    init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(gpu, window);
    init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;                      // Only used in multi-viewports mode.
    init_info.SwapchainComposition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;  // Only used in multi-viewports mode.
    init_info.PresentMode = SDL_GPU_PRESENTMODE_VSYNC;
    ImGui_ImplSDLGPU3_Init(&init_info);

    bool shouldClose = false;

    AssetSet{ASSETPTR(testVert), ASSETPTR(testFrag)}.BlockUntilLoaded();
    SDL_GPUColorTargetDescription colorTargetDescription = {
        .format = SDL_GetGPUSwapchainTextureFormat(TheGPU, window)
    };

    SDL_GPUVertexBufferDescription bufDesc = {
        .slot = 0,
        .pitch = sizeof(MeshVertex),
        .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
        .instance_step_rate = 0,
    };
    SDL_GPUVertexAttribute attributes[] = {
        {
            .location = 0,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
            .offset = 0
        },
        {
            .location = 1,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
            .offset = sizeof(Vector3)
        }
    };
    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {
        .vertex_shader = ASSET(testVert),
        .fragment_shader = ASSET(testFrag),
        .vertex_input_state = {
            .vertex_buffer_descriptions = &bufDesc,
            .num_vertex_buffers = 1,
            .vertex_attributes = attributes,
            .num_vertex_attributes = 2,
        },
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .rasterizer_state = {
            .cull_mode = SDL_GPU_CULLMODE_BACK,
            .front_face = SDL_GPU_FRONTFACE_CLOCKWISE
        },
        .target_info = {
            .color_target_descriptions = &colorTargetDescription,
            .num_color_targets = 1
        },
    };
    pipelineCreateInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    auto testPipeline = SDL_CreateGPUGraphicsPipeline(TheGPU, &pipelineCreateInfo);
    if (testPipeline == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create pipeline: %s\n", SDL_GetError());
        return -1;
    }

    Camera cam = {
        { 0, 8.0f, -14.0f },
        { 0.0f, 0.0f, 15.0f },
        { 0.0f, 1.0f, 0.0f },
        40.0f,
    };

    SDL_GPUSamplerCreateInfo samplerCreate = {
        .min_filter = SDL_GPU_FILTER_LINEAR,
        .mag_filter = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE
    };
    auto sampler = SDL_CreateGPUSampler(TheGPU, &samplerCreate);


    while (!shouldClose) {
        ZoneScopedN("Main Loop")
        SDL_Event event;
        {
            ZoneScopedN("SDL Event Polling")
            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL3_ProcessEvent(&event);
                if (event.type == SDL_EVENT_QUIT) {
                    shouldClose = true;
                }
            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplSDLGPU3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();
        static Vector2 uvOffset = {0, 0};

        if (ImGui::Begin("Test")) {
            ImGui::DragFloat3("Camera Position", &cam.position[0], 0.01f);
            ImGui::DragFloat3("Camera Target", &cam.target[0], 0.01f);
            ImGui::DragFloat("FOV", &cam.fovy, 0.01f);
            ImGui::DragFloat2("UV Offset", &uvOffset[0], 0.01f);
        }
        ImGui::End();


        ImGui::Render();
        auto cmdbuf = SDL_AcquireGPUCommandBuffer(gpu);
        if (cmdbuf == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create command buffer: %s\n", SDL_GetError());
            return 1;
        }


        if (!TheAssets.finalizeQueue.empty()) {
            static std::deque<Asset*> pulled;
            pulled.clear();
            TheAssets.finalizeQueueMutex.lock();
            for (auto asset : TheAssets.finalizeQueue) {
                pulled.push_back(asset);
            }
            TheAssets.finalizeQueue.clear();
            TheAssets.finalizeQueueMutex.unlock();
            auto copyPass = SDL_BeginGPUCopyPass(cmdbuf);
            for (auto asset : pulled) {
                asset->Finalize(copyPass);
            }
            SDL_EndGPUCopyPass(copyPass);
        }

        SDL_GPUTexture* swapchainTexture;
        {
            ZoneScopedN("Acquire Swapchain")
            if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, window, &swapchainTexture, NULL, NULL)) {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create swapchain texture: %s\n", SDL_GetError());
                return 1;
            }
        }


        SDL_GPUColorTargetInfo colorTargetInfo = {};
        colorTargetInfo.texture = swapchainTexture;
        colorTargetInfo.resolve_texture = swapchainTexture;
        colorTargetInfo.clear_color = (SDL_FColor){0.3f, 0.4f, 0.5f, 1.0f};
        colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
        colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
        static AssetSet meshStuff = {ASSETPTR(testMesh), ASSETPTR(testMeshTex)};
        if (meshStuff.PollLoaded()) {
            ZoneScopedN("Build Render Pass")
            auto renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);
            SDL_BindGPUGraphicsPipeline(renderPass, testPipeline);
            SDL_GPUBufferBinding vertBind = { .buffer = ASSET(testMesh).vertexBuffer, .offset = 0 };
            SDL_GPUBufferBinding indexBind = { .buffer = ASSET(testMesh).indexBuffer, .offset = 0 };
            SDL_BindGPUVertexBuffers(renderPass, 0, &vertBind, 1);
            SDL_BindGPUIndexBuffer(renderPass, &indexBind, SDL_GPU_INDEXELEMENTSIZE_32BIT);
            SDL_GPUTextureSamplerBinding samplerBinding = {ASSET(testMeshTex), sampler};
            SDL_BindGPUFragmentSamplers(renderPass, 0, &samplerBinding, 1);
            UBO uniform {
                cam.getMatrix(),
                uvOffset
            };
            SDL_PushGPUVertexUniformData(cmdbuf, 0, &uniform, sizeof(uniform));
            SDL_DrawGPUIndexedPrimitives(renderPass, ASSET(testMesh).numFaces*3, 1, 0, 0, 0);
            SDL_EndGPURenderPass(renderPass);
        }

        {
            ZoneScopedN("ImGui Render")
            ImDrawData* drawData = ImGui::GetDrawData();

            ImGui_ImplSDLGPU3_PrepareDrawData(drawData, cmdbuf);

            // Setup and start a render pass
            SDL_GPUColorTargetInfo target_info = {};
            target_info.texture = swapchainTexture;
            target_info.load_op = SDL_GPU_LOADOP_LOAD;
            target_info.store_op = SDL_GPU_STOREOP_STORE;
            target_info.mip_level = 0;
            target_info.layer_or_depth_plane = 0;
            target_info.cycle = false;
            SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(cmdbuf, &target_info, 1, nullptr);

            // Render ImGui
            ImGui_ImplSDLGPU3_RenderDrawData(drawData, cmdbuf, render_pass);

            SDL_EndGPURenderPass(render_pass);
        }

        {
            ZoneScopedN("Submit Command Buffer")
            SDL_SubmitGPUCommandBuffer(cmdbuf);
        }
        FrameMark;
    }

    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}
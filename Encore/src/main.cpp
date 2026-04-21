
#include "arguments.h"
#include "math/glm.h"
#include "imgui.h"
#include "imgui/backends/imgui_impl_sdlgpu3.h"
#include "imgui/backends/imgui_impl_sdl3.h"
#include "SDL3/SDL.h"
#include "tracy/Tracy.hpp"
#include "graphicsState.h"
#include "assets.h"
#include "glm/gtc/random.hpp"
#include "math/camera.h"
#include "render/gpudynamicbuffer.h"
#include "render/gpudynamicframebuffer.h"
#include "render/pipelines.h"
#include "ui/box.h"

#include <deque>

SDL_GPUDevice* TheGPU = nullptr;
bool TheGPUReady = false;
SDL_Window* TheWindow = nullptr;
GPUDynamicFramebuffer* TheFramebuffer = nullptr;
PipelineManager ThePipelineManager;

std::vector<std::string> ArgumentList::arguments = {};

void BlockUntilGPUReady() {
    while (!TheGPUReady) {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}

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
struct GemInstance {
    Vector2 position;
    Vector2 scale;
    
};

int main(int argc, char *argv[]) {
    ArgumentList::InitArguments(argc, argv);
    auto videoDriver = ArgumentList::GetArgValue("video_driver");
    if (!videoDriver.empty()) {
        SDL_SetHint(SDL_HINT_VIDEO_DRIVER, videoDriver.c_str());
    }
    auto graphicsDriver = ArgumentList::GetArgValue("graphics_driver");
    if (!graphicsDriver.empty()) {
        SDL_SetHint(SDL_HINT_GPU_DRIVER, graphicsDriver.c_str());
    }
    SDL_SetAppMetadata("Encore", "v0.2.0", "encore");
    LocateDevAssets();
    SDL_Log("Asset path: %s", TheAssets.getDirectory().c_str());
    //

    for (auto asset : TheAssets.assets) {
        asset->StartLoad();
    }

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD)) {
        return 1;
    }

    TheFramebuffer = new GPUDynamicFramebuffer(SDL_GPU_SAMPLECOUNT_4);
    auto window = SDL_CreateWindow("Encore", 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    TheWindow = window;

    while (!ASSET(faviconTex).rawDataLoaded) {}
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
    TheGPUReady = true;

    SDL_ClaimWindowForGPUDevice(gpu, window);
    SDL_SetGPUSwapchainParameters(gpu, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_MAILBOX);
    SDL_SetGPUAllowedFramesInFlight(TheGPU, 2);

    PIPELINE(CompileThreaded());
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

    GPUDynamicBuffer<GemInstance> gemInstances(100, SDL_GPU_BUFFERUSAGE_VERTEX, true);
    PIPELINE(BlockUntilLoaded());

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
        static bool randomizeInstances = true;
        static int instanceCount = 30;
        static UI::Box box({Color(0, 0.7, 0, 1), Color(0, 0.6, 0, 1)});
        static Vector2 boxPos = {20, 20};
        static Vector2 boxSize = {100, 100};

        if (ImGui::Begin("Test")) {
            ImGui::DragFloat3("Camera Position", &cam.position[0], 0.01f);
            ImGui::DragFloat3("Camera Target", &cam.target[0], 0.01f);
            ImGui::DragFloat("FOV", &cam.fovy, 0.01f);
            ImGui::DragFloat2("UV Offset", &uvOffset[0], 0.01f);
            ImGui::Checkbox("Randomize Gem Instances Every Frame", &randomizeInstances);
            ImGui::DragInt("Instance Count", &instanceCount);
            static bool antialiasing = true;
            ImGui::Checkbox("Antialiasing", &antialiasing);
            TheFramebuffer->SetSampleCount(antialiasing ? SDL_GPU_SAMPLECOUNT_4 : SDL_GPU_SAMPLECOUNT_1);
        }
        ImGui::End();

        ImGui::SetNextWindowPos(boxPos, ImGuiCond_Appearing);
        ImGui::SetNextWindowSize(boxSize, ImGuiCond_Appearing);
        if (ImGui::Begin("Rect", 0, 0)) {
            ImGui::DragFloat4("Corner Radii", (float*)&box.cornerSizes, 1);
            ImGui::ColorEdit4("Border Color Top", (float*)&box.color.top, ImGuiColorEditFlags_Float);
            ImGui::ColorEdit4("Border Color Bottom", (float*)&box.color.bottom, ImGuiColorEditFlags_Float);
            ImGui::Text("Size: %f, %f", boxSize.x, boxSize.y);
            static UI::CornerType meshes[] = {UI::SQUIRCLE, UI::CIRCULAR, UI::ANGULAR};
            static const char* meshNames[] = {"Squircle", "Circular", "No Corners"};
            static int curMesh = 0;
            ImGui::Combo("Corner Type", &curMesh, meshNames, 3);
            if (ImGui::Button("Add Border")) {
                box.borders.push_back({{5}, 5, Color(1, 1, 1, 1)});
            }
            box.cornerType = meshes[curMesh];
            for (auto& border : box.borders) {
                ImGui::PushID(&border);
                ImGui::ColorEdit4("Border Color Top", (float*)&border.color.top, ImGuiColorEditFlags_Float);
                ImGui::ColorEdit4("Border Color Bottom", (float*)&border.color.bottom, ImGuiColorEditFlags_Float);
                ImGui::DragFloat4("Size", (float*)&border.sizes, 1);
                ImGui::DragFloat("Corner Shrink", &border.cornerShrink);
                ImGui::PopID();
            }
        }
        ImGui::End();

        ImGui::Begin("Rect2", 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar);
        boxPos = ImGui::GetWindowPos();
        boxSize = ImGui::GetWindowSize();
        ImGui::End();

        if (randomizeInstances) {
            ZoneScopedN("Generate Gem Instances")
            gemInstances.Reset();
            for (int i = 0; i < instanceCount; i++) {
                GemInstance instance {
                    {i % 5 - 2, (1-(float)i/instanceCount)*150.0f},
                    {1, 1}
                };

                gemInstances.Push(instance);
            }
        }

        ImGui::Render();
        auto cmdbuf = SDL_AcquireGPUCommandBuffer(gpu);
        if (cmdbuf == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create command buffer: %s\n", SDL_GetError());
            return 1;
        }


        auto copyPass = SDL_BeginGPUCopyPass(cmdbuf);
        if (randomizeInstances) {
            ZoneScopedN("Upload Gem Instances")
            gemInstances.UploadData(copyPass);
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
            for (auto asset : pulled) {
                asset->Finalize(copyPass);
            }
        }
        UI::Box::InitializeMeshes(copyPass);
        SDL_EndGPUCopyPass(copyPass);

        SDL_GPUTexture* swapchainTexture;
        {
            unsigned int width;
            unsigned int height;
            ZoneScopedN("Acquire Swapchain")
            if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, window, &swapchainTexture, &width, &height)) {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create swapchain texture: %s\n", SDL_GetError());
                return 1;
            }
            TheFramebuffer->Update(width, height);
        }


        SDL_GPUColorTargetInfo colorTargetInfo = TheFramebuffer->GetColorTargetInfo();
        colorTargetInfo.clear_color = {0.3f, 0.4f, 0.5f, 1.0f};
        colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
        colorTargetInfo.cycle = true;
        SDL_GPUDepthStencilTargetInfo depthTargetInfo = TheFramebuffer->GetDepthStencilTargetInfo();
        depthTargetInfo.clear_depth = 1;
        depthTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
        depthTargetInfo.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
        depthTargetInfo.cycle = true;
        if (true) {
            ZoneScopedN("2D Render Pass")
            auto renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, &depthTargetInfo);
            UBO uniform {
                glm::orthoNO(0.0f, (float)TheFramebuffer->width, (float)TheFramebuffer->height, 0.0f, -1.0f, 1.0f),
                uvOffset
            };
            SDL_PushGPUVertexUniformData(cmdbuf, 0, &uniform, sizeof(uniform));

            box.Draw(renderPass, cmdbuf, boxPos, boxSize);

            SDL_EndGPURenderPass(renderPass);
        }

        {
            ZoneScopedN("ImGui Render")
            ImDrawData* drawData = ImGui::GetDrawData();

            ImGui_ImplSDLGPU3_PrepareDrawData(drawData, cmdbuf);

            // Setup and start a render pass
            SDL_GPUColorTargetInfo target_info = {};
            target_info.texture = TheFramebuffer->GetBlitSource();
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
            ZoneScopedN("Blit Framebuffer")
            SDL_GPUBlitInfo blitInfo = {
                .source = {
                    .texture = TheFramebuffer->GetBlitSource(),
                    .mip_level = 0,
                    .layer_or_depth_plane = 0,
                    .x = 0,
                    .y = 0,
                    .w = TheFramebuffer->width,
                    .h = TheFramebuffer->height,
                },
                .destination = {
                    .texture = swapchainTexture,
                    .mip_level = 0,
                    .layer_or_depth_plane = 0,
                    .x = 0,
                    .y = 0,
                    .w = TheFramebuffer->width,
                    .h = TheFramebuffer->height,
                },
                .load_op = SDL_GPU_LOADOP_DONT_CARE,
                .flip_mode = SDL_FLIP_NONE,
                .filter = SDL_GPU_FILTER_NEAREST,
                .cycle = false
            };
            SDL_BlitGPUTexture(cmdbuf, &blitInfo);
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
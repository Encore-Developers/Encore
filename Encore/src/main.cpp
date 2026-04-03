
#include "imgui.h"
#include "imgui/backends/imgui_impl_sdlgpu3.h"
#include "imgui/backends/imgui_impl_sdl3.h"
#include "SDL3/SDL.h"
#include "math/Matrix.h"
#include "math/Vector.h"
#include "tracy/Tracy.hpp"

int main(int argc, char *argv[]) {
    SDL_SetAppMetadata("Encore", "v0.2.0", "encore");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD)) {
        return 1;
    }

    auto window = SDL_CreateWindow("Encore", 1280, 720, SDL_WINDOW_RESIZABLE);

    if (window == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    auto gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL, false, nullptr);
    if (gpu == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create GPU: %s\n", SDL_GetError());
        return 1;
    }

    SDL_ClaimWindowForGPUDevice(gpu, window);
    SDL_SetGPUSwapchainParameters(gpu, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);

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

        if (ImGui::Begin("Test")) {
            ImGui::Text("Hello World!");
        }
        ImGui::End();



        ImGui::Render();
        auto cmdbuf = SDL_AcquireGPUCommandBuffer(gpu);
        if (cmdbuf == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create command buffer: %s\n", SDL_GetError());
            return 1;
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
        colorTargetInfo.clear_color = (SDL_FColor){0.3f, 0.4f, 0.5f, 1.0f};
        colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
        colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

        {
            ZoneScopedN("Build Render Pass")
            auto renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);
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
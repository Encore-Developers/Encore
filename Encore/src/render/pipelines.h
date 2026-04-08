#pragma once
#include "SDL3/SDL_gpu.h"

class PipelineManager {
    static void ClearPipeline(SDL_GPUGraphicsPipeline** pipelinePtr);
public:
    bool pipelinesLoaded = false;
    SDL_GPUGraphicsPipeline* testPipeline = nullptr;

    void CompileAll();
    void CompileThreaded();
};
#pragma once
#include "SDL3/SDL_gpu.h"

class PipelineManager {
    static void ClearPipeline(SDL_GPUGraphicsPipeline** pipelinePtr);
public:
    bool pipelinesLoaded = false;
    SDL_GPUGraphicsPipeline* notePipeline = nullptr;
    SDL_GPUGraphicsPipeline* boxPipeline = nullptr;

    void CompileAll();
    void CompileThreaded();
    void BlockUntilLoaded();
};


extern PipelineManager ThePipelineManager;

#define PIPELINE(pipeline) ThePipelineManager.pipeline

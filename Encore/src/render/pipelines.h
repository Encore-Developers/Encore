#pragma once
#include "SDL3/SDL_gpu.h"

#define DEFINEPIPELINE(name) SDL_GPUGraphicsPipeline* name = nullptr

class PipelineManager {
    static void ClearPipeline(SDL_GPUGraphicsPipeline** pipelinePtr);
public:
    bool pipelinesLoaded = false;

    DEFINEPIPELINE(notePipeline);
    DEFINEPIPELINE(boxPipeline);
    DEFINEPIPELINE(compositeLayerPipeline);

    void CompileAll();
    void CompileThreaded();
    void BlockUntilLoaded();
};


extern PipelineManager ThePipelineManager;

#define PIPELINE(pipeline) ThePipelineManager.pipeline

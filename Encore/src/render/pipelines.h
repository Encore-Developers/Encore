#pragma once
#include "graphicsState.h"
#include "SDL3/SDL_gpu.h"

#include <unordered_map>
#include <nlohmann/json.hpp>


// go my RAII wrapper
struct GraphicsPipeline {
    SDL_GPUGraphicsPipeline* pipeline;

    GraphicsPipeline(SDL_GPUGraphicsPipeline* pipeline) : pipeline(pipeline) {}
    GraphicsPipeline(const GraphicsPipeline &other) = delete;

    operator SDL_GPUGraphicsPipeline *() const { return pipeline; }

    ~GraphicsPipeline() {
        SDL_ReleaseGPUGraphicsPipeline(TheGPU, pipeline);
    }
};

class PipelineManager {
    static void ClearPipeline(SDL_GPUGraphicsPipeline** pipelinePtr);
public:
    std::unordered_map<std::string, GraphicsPipeline> pipelines;

    bool pipelinesLoaded = false;



    void CompileAll();
    void CompileThreaded();
    void BlockUntilLoaded();
};


extern PipelineManager ThePipelineManager;

#define GET_PIPELINE(pipeline) ThePipelineManager.pipelines.at(pipeline)
#include "pipelines.h"
#include "graphicsState.h"

#include <thread>

void PipelineManager::ClearPipeline(SDL_GPUGraphicsPipeline **pipelinePtr) {
    if (*pipelinePtr) {
        SDL_ReleaseGPUGraphicsPipeline(TheGPU, *pipelinePtr);
        *pipelinePtr = nullptr;
    }
}
void PipelineManager::CompileAll() {
    ClearPipeline(&testPipeline);
}
void PipelineManager::CompileThreaded() {
    pipelinesLoaded = false;
    std::thread compileThread([this]() {
        CompileAll();
    });
    compileThread.detach();
}
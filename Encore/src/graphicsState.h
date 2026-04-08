#pragma once
#include "SDL3/SDL_gpu.h"

// This file is just here to store globals for the window and the graphics device
// If you have a better idea please let me know

extern SDL_GPUDevice* TheGPU;
extern bool TheGPUReady;
extern SDL_Window* TheWindow;

void BlockUntilGPUReady();
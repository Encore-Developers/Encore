//
// Created by maria on 16/02/2025.
//

#include "frame-manager.h"

#include "raylib.h"
#include "settings.h"
#include "menus/MenuManager.h"

void Encore::FrameManager::InitFrameManager() {
    previousTime = GetTime();
    currentTime = 0.0;
    updateDrawTime = 0.0;
    waitTime = 0.0;
    deltaTime = 0.0;
    removeFPSLimit = false;
    menuFPS = GetMonitorRefreshRate(GetCurrentMonitor()) / 2;
}
void Encore::FrameManager::WaitForFrame() {
    if (!removeFPSLimit) {
        currentTime = GetTime();
        updateDrawTime = currentTime - previousTime;
        int Target = TheGameSettings.Framerate;
        if (TheMenuManager.currentScreen != GAMEPLAY)
            Target = menuFPS;

        if (Target > 0) {
            waitTime = (1.0f / (float)Target) - updateDrawTime;
            if (waitTime > 0.0) {
                WaitTime((float)waitTime);
                currentTime = GetTime();
                deltaTime = (float)(currentTime - previousTime);
            }
        } else
            deltaTime = (float)updateDrawTime;

        previousTime = currentTime;
    }
}

//
// Created by maria on 16/02/2025.
//

#ifndef FRAMEMANAGER_H
#define FRAMEMANAGER_H


namespace Encore {
class FrameManager {
public:
    double previousTime;
    double currentTime;
    double updateDrawTime;
    double waitTime;
    float deltaTime;
    bool removeFPSLimit;
    int menuFPS;
    void InitFrameManager();
    void WaitForFrame();
};
}


#endif //FRAMEMANAGER_H

extern Encore::FrameManager TheFrameManager;
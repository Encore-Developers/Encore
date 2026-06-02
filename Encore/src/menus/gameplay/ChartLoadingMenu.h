//
// Created by marie on 17/11/2024.
//

#ifndef CHARTLOADINGMENU_H
#define CHARTLOADINGMENU_H
#include "../overshell/OvershellMenu.h"

class ChartLoadingMenu : public OvershellMenu {
public:
    ChartLoadingMenu() {};
    ~ChartLoadingMenu() {};
    void KeyboardInputCallback(SDL_KeyboardEvent* event) override {};
    void ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event) override {};
    void Draw() override;
    void Load() override;
};
#endif //CHARTLOADINGMENU_H

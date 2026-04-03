//
// Created by marie on 17/11/2024.
//

#ifndef CHARTLOADINGMENU_H
#define CHARTLOADINGMENU_H
#include "menu.h"

class ChartLoadingMenu : public Menu {
public:
    ChartLoadingMenu() {};
    ~ChartLoadingMenu() {};
    void KeyboardInputCallback(int key, int scancode, int action, int mods) override {};
    void ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event) override {};
    void Draw() override;
    void Load() override;
};
#endif //CHARTLOADINGMENU_H

//
// Created by marie on 17/11/2024.
//

#ifndef CHARTLOADINGMENU_H
#define CHARTLOADINGMENU_H
#include "../overshell/OvershellMenu.h"

class ChartLoadingMenu : public OvershellMenu {
public:
    Song* curSong;

    ChartLoadingMenu(Song* song) : curSong(song) {};
    ~ChartLoadingMenu() {};
    void KeyboardInputCallback(SDL_KeyboardEvent* event) override {};
    void ControllerInputCallback(Encore::ControllerEvent event) override {};
    void Draw() override;
    void Load() override;
    void LoadCharts();
};
#endif //CHARTLOADINGMENU_H

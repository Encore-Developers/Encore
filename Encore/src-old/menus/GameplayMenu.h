#pragma once
//
// Created by marie on 20/10/2024.
//

#include "assets.h"
#include "menu.h"
#include "uiUnits.h"

#include <vector>

#include "gameplay/trackRenderer/Track.h"

extern bool songPlaying;

// technically this IS a menu, but realistically, is it?
class GameplayMenu : public Menu {
    std::vector<std::shared_ptr<Encore::Track>> tracks;
public:
    GameplayMenu();
    virtual ~GameplayMenu();
    void DrawScorebox(Units &u, Assets &assets, float scoreY);
    void DrawTimerbox(Units &u, Assets &assets, float scoreY);
    void DrawGameplayStars(Units &u, Assets &assets, float scorePos, float starY);
    void KeyboardInputCallback(int key, int scancode, int action, int mods);
    void ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event);
    void Draw() override;
    void Load() override;
};

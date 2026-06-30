#pragma once
//
// Created by marie on 20/10/2024.
//

#include "../overshell/OvershellMenu.h"
#include "assets.h"
#include "../menu.h"
#include "../util/uiUnits.h"
#include "RhythmEngine/Replay.h"

#include <vector>

#include "gameplay/trackRenderer/Track.h"
#include "menus/util/ButtonActionRegistry.h"


// technically this IS a menu, but realistically, is it?
class GameplayMenu : public OvershellMenu {
protected:
    int CameraSelectionPerPlayer[4][4] {
        {0,0,0,0},
        {1,0,0,0},
        {2,1,0,0},
        {3,2,1,0}
    };
    int CameraPosPerPlayer[4][4] {
        {0,0,0,0},
        {8,-8,0,0},
        {4,0,-4,0},
        {4,12,-12,-4}
    };
    std::vector<std::shared_ptr<Encore::Track>> tracks;
    Encore::ButtonActionRegistry buttReg;
public:
    bool streamsPaused = false;
    bool songPlaying = false;

    Song* curSong;
    Encore::RhythmEngine::Replay recordingReplay;

    GameplayMenu(Song* song);
    virtual ~GameplayMenu();
    void DrawScorebox(Units &u, Assets &assets, float scoreY);
    void DrawTimerbox(Units &u, Assets &assets, float scoreY);
    void DrawGameplayStars(Units &u, Assets &assets, float scorePos, float starY);
    void KeyboardInputCallback(SDL_KeyboardEvent* event);
    void ControllerInputCallback(Encore::ControllerEvent event);
    void SaveReplay();
    void Draw() override;
    void Load() override;
    void DrawPauseMenu();
    virtual bool CheckPauseInput(Encore::ControllerEvent event);
    virtual void UpdatePauseState();
    virtual bool IsPaused();

    virtual void SetPresence() override;
    bool AllowsTempPlayers() override { return true; };
};

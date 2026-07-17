//
// Created by marie on 17/11/2024.
//

#ifndef CHARTLOADINGMENU_H
#define CHARTLOADINGMENU_H
#include "../overshell/OvershellMenu.h"
#include "RhythmEngine/ChartLoaders/ChartLoader.h"
#include "song/video/VideoBackground.h"

class ChartLoadingMenu : public OvershellMenu {
public:
    enum Gamemode {
        GAMEPLAY,
        PRACTICE
    };

    Song* curSong;
    Gamemode gamemode;
    Encore::RhythmEngine::ChartLoader chartLoader;

    std::shared_ptr<VideoBackground> videoBackground;

    ChartLoadingMenu(Song* song, Gamemode gamemode = GAMEPLAY) : curSong(song), gamemode(gamemode), chartLoader(song->midiPath)  {};
    ~ChartLoadingMenu() {};
    void KeyboardInputCallback(SDL_KeyboardEvent* event) override {};
    void ControllerInputCallback(Encore::ControllerEvent event) override {};
    void Draw() override;
    void Load() override;
    void LoadCharts();

    bool AllowsTempPlayers() override { return true; };
};
#endif //CHARTLOADINGMENU_H

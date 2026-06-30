//
// Created by marie on 14/09/2024.
//

#ifndef RESULTSMENU_H
#define RESULTSMENU_H

#include "../main/MainMenu.h"
#include "../menu.h"

class resultsMenu : public OvershellMenu {
    enum ResultsState {
        GENERAL,
        SECTIONS
    };
    // ok assets go here
    // i forgot which assets i need.
    Texture2D GoldStar;
    Texture2D Star;
    Texture2D EmptyStar;
    Shader sdfShader;
    std::vector<std::string> diffList;
    std::array<ResultsState, MAX_PLAYERS> resultsState {GENERAL};
    std::array<int, MAX_PLAYERS> topSectList {0};
    void drawPlayerResults(Player &player, int playerslot);
    void renderPlayerStars(
         Player &stats, float xPos, float yPos, float scale, bool left
    );
    //  void
    //  renderStars(BandGameplayStats *&stats, float xPos, float yPos, float scale, bool
    //  left);
    Encore::ButtonActionRegistry buttReg;
public:
    Song* curSong;

    resultsMenu(Song* song) : curSong(song) {}
    virtual void KeyboardInputCallback(SDL_KeyboardEvent* event);
    virtual void ControllerInputCallback(Encore::ControllerEvent event);
    //~resultsMenu() override;
    void Draw() override;
    void Load() override;

    bool AllowsTempPlayers() override { return true; };
};

#endif // RESULTSMENU_H
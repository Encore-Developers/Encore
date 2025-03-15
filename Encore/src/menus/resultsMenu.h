//
// Created by marie on 14/09/2024.
//

#ifndef RESULTSMENU_H
#define RESULTSMENU_H

#include "gameMenu.h"
#include "menu.h"

class resultsMenu : public OvershellMenu {
    // ok assets go here
    // i forgot which assets i need.
    Texture2D GoldStar;
    Texture2D Star;
    Texture2D EmptyStar;
    Shader sdfShader;
    std::vector<std::string> diffList;
    void drawPlayerResults(Player &player, Song song, int playerslot);
    void renderPlayerStars(
        PlayerGameplayStats *&stats, float xPos, float yPos, float scale, bool left
    );
    void
    renderStars(BandGameplayStats *&stats, float xPos, float yPos, float scale, bool left);

public:
    resultsMenu();
    virtual void KeyboardInputCallback(int key, int scancode, int action, int mods);
    virtual void ControllerInputCallback(int joypadID, GLFWgamepadstate state);
    ~resultsMenu() override;
    void Draw() override;
    void Load() override;
};

#endif // RESULTSMENU_H

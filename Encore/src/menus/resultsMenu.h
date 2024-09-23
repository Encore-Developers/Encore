//
// Created by marie on 14/09/2024.
//

#ifndef RESULTSMENU_H
#define RESULTSMENU_H

#include "gameMenu.h"
#include "menu.h"

class resultsMenu : public Menu {
    // ok assets go here
    // i forgot which assets i need.
    Font RedHatDisplayItalic; // large song text
    Font RubikItalic; // artist name text
    Font RubikBoldItalic; // praise text
    Font Rubik; // performance text
    Font RubikBold; // instrument/difficulty text
    Font JosefinSansItalic; // extra information text
    Texture2D GoldStar;
    Texture2D Star;
    Texture2D EmptyStar;
    Shader sdfShader;
    std::vector<std::string> diffList;
    std::vector<std::string> songPartsList;
    void drawPlayerResults(Player *player, Song song, int playerslot);
    void renderStars(
        PlayerGameplayStats *player, float xPos, float yPos, float scale, bool left
    );
    void
    renderStars(BandGameplayStats *player, float xPos, float yPos, float scale, bool left);

public:
    resultsMenu();
    virtual ~resultsMenu();
    virtual void Draw();
    virtual void Load();
};

#endif // RESULTSMENU_H

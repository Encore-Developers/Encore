#pragma once
//
// Created by marie on 20/10/2024.
//

#include "menu.h"

// technically this IS a menu, but realistically, is it?
class GameplayMenu : public Menu {
public:
    GameplayMenu();
    virtual ~GameplayMenu();
    virtual void Draw();
    virtual void Load();
};

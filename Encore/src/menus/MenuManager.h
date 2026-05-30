#pragma once
#include "menu.h"

#include <memory>

class MenuManager {
public:
    void SwitchToMenu(std::shared_ptr<Menu> menu);
    void LoadMenu();
    void DrawMenu();
    std::shared_ptr<Menu> ActiveMenu;
    bool onNewMenu = true;

    template<typename T, typename... _Args>
    void CreateAndSwitchMenu(_Args&&... __args) {
        SwitchToMenu(std::make_shared<T>(std::forward<_Args>(__args)...));
    }
};

extern MenuManager TheMenuManager;

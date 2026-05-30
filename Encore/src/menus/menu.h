#pragma once
#include "GLFW/glfw3.h"
#include "RhythmEngine/REenums.h"
#include "users/playerManager.h"
#include "util/discord.h"

class Menu {
public:
    bool UIInput = true;

    Menu() {}
    virtual ~Menu() {}

    virtual void KeyboardInputCallback(int key, int scancode, int action, int mods) = 0;
    virtual void ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event) = 0;

    virtual void Draw() = 0; // NOTE: requires BeginDrawing() to have already been called
    virtual void Load() = 0;

    virtual void SetPresence() {
        TheGameRPC.SteamOverlayPosition(false);
        TheGameRPC.DiscordUpdatePresence("In the menus", "In the menus", ThePlayerManager.PlayersActive);
        TheGameRPC.SteamUpdatePresence("steam_display", "#StatusInMenus");
    }
};

extern Menu *ActiveMenu;

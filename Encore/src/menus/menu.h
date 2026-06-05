#pragma once
#include "RhythmEngine/REenums.h"
#include "users/playerManager.h"
#include "util/discord.h"

class Menu {
    std::weak_ptr<Menu> selfRef;

    friend class MenuManager;
public:
    bool UIInput = true;
    bool AlreadyLoaded = false;

    Menu() {}
    virtual ~Menu() {}

    virtual void KeyboardInputCallback(SDL_KeyboardEvent* event) = 0;
    virtual void ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event) = 0;

    std::shared_ptr<Menu> GetRef() {
        return selfRef.lock();
    }

    virtual void Draw() = 0; // NOTE: requires BeginDrawing() to have already been called
    virtual void Load() = 0;

    virtual void SetPresence() {
        TheGameRPC.SteamOverlayPosition(false);
        TheGameRPC.DiscordUpdatePresence("In the menus", "In the menus", ThePlayerManager.PlayersActive);
        TheGameRPC.SteamUpdatePresence("steam_display", "#StatusInMenus");
    }
};

extern Menu *ActiveMenu;

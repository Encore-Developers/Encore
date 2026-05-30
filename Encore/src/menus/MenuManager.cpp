#include "MenuManager.h"

#include "gameplay/ChartLoadingMenu.h"
#include "gameplay/GameplayMenu.h"
#include "gameplay/ReadyUpMenu.h"
#include "settings/SettingsAudioVideo.h"
#include "settings/SettingsController.h"
#include "settings/SettingsGameplay.h"
#include "settings/SettingsKeyboard.h"
#include "settings/SettingsMenu.h"
#include "main/SongSelectMenu.h"
#include "main/cacheLoadingScreen.h"
#include "main/MainMenu.h"
#include "menu.h"
#include "raygui.h"
#include "gameplay/resultsMenu.h"
#include "settings/settings.h"
#include "sndTestMenu.h"
#include "gameplay/inputCallbacks.h"
#include "users/playerManager.h"
#include "util/discord.h"
#include "tracy/Tracy.hpp"

#include <cstddef>
#include <utility>

#include "settings/keybinds.h"

void MenuManager::SwitchToMenu(std::shared_ptr<Menu> menu) {
    ActiveMenu = std::move(menu);
    onNewMenu = true;
}
void MenuManager::LoadMenu() {
    ZoneScoped
    TheMenuManager.onNewMenu = false;
    ActiveMenu->Load();
    glfwSetKeyCallback(glfwGetCurrentContext(), keyCallback);
    // glfwSetGamepadStateCallback(gamepadStateCallback);
}

void MenuManager::DrawMenu() {
    ZoneScoped;
    // Copy the shared pointer so it doesn't get dealloc'd mid-drawing
    std::shared_ptr<Menu> menu = ActiveMenu;
    menu->Draw();
}

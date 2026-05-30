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
    if (ScanningSongs) {
        Units &u = Units::getInstance();
        Rectangle warningBox {u.wpct(0.35), u.hpct(0.35), u.winpct(0.3), u.hinpct(0.3)};
        DrawRectangleRec(warningBox, {64, 64, 64, 196});
        float fontSize = u.hinpct(0.05f);
        std::string folderCount = std::to_string(FolderCount);
        std::string songCount = std::to_string(SongCount);
        std::string badSongCount = std::to_string(BadSongCount);
        GameMenu::mhDrawText(
            ASSET(rubik),
            "Scanning Songs",
            {  warningBox.x, warningBox.y },
            fontSize,
            WHITE,
            ASSET(sdfShader),
            LEFT
        );
        warningBox.y += fontSize;
        GameMenu::mhDrawText(
            ASSET(rubik),
            "Folders Found: ",
            {  warningBox.x, warningBox.y},
            fontSize,
            WHITE,
            ASSET(sdfShader),
            LEFT
        );
        GameMenu::mhDrawText(
            ASSET(redHatMono),
            folderCount,
            {  warningBox.x + (warningBox.width), warningBox.y },
            fontSize,
            WHITE,
            ASSET(sdfShader),
            RIGHT
        );
        warningBox.y += fontSize;
        GameMenu::mhDrawText(
            ASSET(rubik),
            "Songs Found: ",
            {  warningBox.x, warningBox.y },
            fontSize,
            WHITE,
            ASSET(sdfShader),
            LEFT
        );
        GameMenu::mhDrawText(
            ASSET(redHatMono),
            songCount,
            {  warningBox.x + warningBox.width, warningBox.y },
            fontSize,
            WHITE,
            ASSET(sdfShader),
            RIGHT
        );
        warningBox.y += fontSize;
        GameMenu::mhDrawText(
            ASSET(rubik),
            "Bad Songs: ",
            {  warningBox.x, warningBox.y },
            fontSize,
            WHITE,
            ASSET(sdfShader),
            LEFT
        );
        GameMenu::mhDrawText(
            ASSET(redHatMono),
            badSongCount,
            {  warningBox.x + warningBox.width, warningBox.y },
            fontSize,
            WHITE,
            ASSET(sdfShader),
            RIGHT
        );
    }
}

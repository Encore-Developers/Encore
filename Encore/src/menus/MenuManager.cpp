#include "MenuManager.h"

#include "ChartLoadingMenu.h"
#include "GameplayMenu.h"
#include "ReadyUpMenu.h"
#include "SettingsAudioVideo.h"
#include "SettingsController.h"
#include "SettingsGameplay.h"
#include "SettingsKeyboard.h"
#include "SettingsMenu.h"
#include "SongSelectMenu.h"
#include "cacheLoadingScreen.h"
#include "gameMenu.h"
#include "menu.h"
#include "raygui.h"
#include "resultsMenu.h"
#include "settings/settings.h"
#include "sndTestMenu.h"
#include "gameplay/inputCallbacks.h"
#include "users/playerManager.h"
#include "util/discord.h"
#include "tracy/Tracy.hpp"

#include <cstddef>

#include "settings/keybinds.h"

void MenuManager::SwitchScreen(Screens screen) {
    currentScreen = screen;
    onNewMenu = true;
}
void MenuManager::LoadMenu() {
    TheMenuManager.onNewMenu = false;
    delete ActiveMenu;
    ActiveMenu = NULL;
    // this is for dropping out

    switch (TheMenuManager.currentScreen) { // NOTE: when adding a new Menu
                                            // derivative, you
        // must put its enum value in Screens, and its
        // assignment in this switch/case. You will also
        // add its case to the `ActiveMenu->Draw();`
        // cases.
    case MAIN_MENU: {
        TheGameRPC.DiscordUpdatePresence("In the menus", "In the menus",ThePlayerManager.PlayersActive);
        TheGameRPC.SteamUpdatePresence("steam_display", "#StatusInMenus");
        ActiveMenu = new MainMenu;
        ActiveMenu->Load();
        break;
    }
    case SETTINGS: {
        TheGameRPC.DiscordUpdatePresence("In the menus", "In the menus",ThePlayerManager.PlayersActive);
        TheGameRPC.SteamUpdatePresence("steam_display", "#StatusInMenus");
        ActiveMenu = new SettingsMenu;
        ActiveMenu->Load();
        break;
    }
    case SETTINGSAUDIOVIDEO: {
        TheGameRPC.DiscordUpdatePresence("In the menus", "In the menus",ThePlayerManager.PlayersActive);
        TheGameRPC.SteamUpdatePresence("steam_display", "#StatusInMenus");
        ActiveMenu = new SettingsAudioVideo;
        ActiveMenu->Load();
        break;
    }
    case SETTINGSGAMEPLAY: {
        TheGameRPC.DiscordUpdatePresence("In the menus", "In the menus",ThePlayerManager.PlayersActive);
        TheGameRPC.SteamUpdatePresence("steam_display", "#StatusInMenus");
        ActiveMenu = new SettingsGameplay;
        ActiveMenu->Load();
        break;
    }
    case SETTINGSKEYBOARD: {
        TheGameRPC.DiscordUpdatePresence("In the menus", "In the menus",ThePlayerManager.PlayersActive);
        TheGameRPC.SteamUpdatePresence("steam_display", "#StatusInMenus");
        ActiveMenu = new SettingsKeyboard;
        ActiveMenu->Load();
        break;
    }
    case SETTINGSCONTROLLER: {
        TheGameRPC.DiscordUpdatePresence("In the menus", "In the menus",ThePlayerManager.PlayersActive);
        TheGameRPC.SteamUpdatePresence("steam_display", "#StatusInMenus");
        ActiveMenu = new SettingsController;
        ActiveMenu->Load();
        break;
    }
    case RESULTS: {
        TheGameRPC.DiscordUpdatePresence("Viewing results", "In the menus",ThePlayerManager.PlayersActive);
        TheGameRPC.SteamUpdatePresence("steam_display", "#StatusResults");
        ActiveMenu = new resultsMenu;
        ActiveMenu->Load();
        break;
    }
    case SONG_SELECT: {
        // glfwSetGamepadStateCallback(gamepadStateCallback);
        TheGameRPC.DiscordUpdatePresence("In the menus", "In the menus",ThePlayerManager.PlayersActive);
        TheGameRPC.SteamUpdatePresence("steam_display", "#StatusInMenus");
        ActiveMenu = new SongSelectMenu;
        ActiveMenu->Load();
        break;
    }
    case READY_UP: {
        TheGameRPC.DiscordUpdatePresence("In the menus", "In the menus",ThePlayerManager.PlayersActive);
        TheGameRPC.SteamUpdatePresence("steam_display", "#StatusInMenus");
        ActiveMenu = new ReadyUpMenu;
        ActiveMenu->Load();
        break;
    }
    case SOUND_TEST: {
        ActiveMenu = new SoundTestMenu;
        ActiveMenu->Load();
        break;
    }
    case CACHE_LOADING_SCREEN: {
        TheGameRPC.DiscordUpdatePresence("In the menus", "In the menus",ThePlayerManager.PlayersActive);
        TheGameRPC.SteamUpdatePresence("steam_display", "#StatusInMenus");
        ActiveMenu = new cacheLoadingScreen;
        ActiveMenu->Load();
        break;
    }
    case CHART_LOADING_SCREEN: {
        TheGameRPC.DiscordUpdatePresence("In the menus", "In the menus",
            ThePlayerManager.PlayersActive);
        TheGameRPC.SteamUpdatePresence("steam_display", "#StatusInMenus");
        ActiveMenu = new ChartLoadingMenu;
        ActiveMenu->Load();
        break;
    }
    case GAMEPLAY: {
        TheGameRPC.DiscordUpdatePresenceSong(
            "Playing a song",
            TheSongList.curSong->title + " - " + TheSongList.curSong->artist,
            ThePlayerManager.GetActivePlayer(0).Instrument,
            ThePlayerManager.PlayersActive
        );
        TheGameRPC.SteamUpdatePresence("song", (TheSongList.curSong->title + " - " + TheSongList.curSong->artist).c_str());
        TheGameRPC.SteamUpdatePresence("steam_display", "#StatusPlayingSongNamed");

        ActiveMenu = new GameplayMenu;
        ActiveMenu->Load();
        break;
    }
    default:;
    }
    glfwSetKeyCallback(glfwGetCurrentContext(), keyCallback);
    // glfwSetGamepadStateCallback(gamepadStateCallback);
}

// calibration
bool isCalibrating = false;
double calibrationStartTime = 0.0;
double lastClickTime = 0.0;
std::vector<double> tapTimes;
const int clickInterval = 1;

bool showInputFeedback = false;
double inputFeedbackStartTime = 0.0;
const double inputFeedbackDuration = 0.6;
float inputFeedbackAlpha = 1.0f;
// end calibration

void MenuManager::DrawMenu() {
    ZoneScoped;
    switch (TheMenuManager.currentScreen) {
        case CALIBRATION: {
            // SettingsOld &settingsMain = SettingsOld::getInstance();
            Units &u = Units::getInstance();
            Assets &assets = Assets::getInstance();

            if (GuiButton(
                    { (float)GetRenderWidth() / 2 - 250,
                      (float)GetRenderHeight() - 120,
                      200,
                      60 },
                    "Start Calibration"
                )) {
                isCalibrating = true;
                calibrationStartTime = GetTime();
                lastClickTime = calibrationStartTime;
                tapTimes.clear();
            }
            if (GuiButton(
                    { (float)GetRenderWidth() / 2 + 50,
                      (float)GetRenderHeight() - 120,
                      200,
                      60 },
                    "Stop Calibration"
                )) {
                isCalibrating = false;

                if (tapTimes.size() > 1) {
                    double totalDifference = 0.0;
                    for (double tapTime : tapTimes) {
                        double expectedClickTime =
                            round((tapTime - calibrationStartTime) / clickInterval)
                                * clickInterval
                            + calibrationStartTime;
                        totalDifference += (tapTime - expectedClickTime);
                    }
                    TheGameSettings.AudioOffset = 0;
                    // Convert to milliseconds
                    std::cout
                        << static_cast<int>((totalDifference / tapTimes.size()) * 1000)
                        << "ms of latency detected" << std::endl;
                }
                std::cout << "Stopped Calibration" << std::endl;
                tapTimes.clear();
            }

            if (isCalibrating) {
                double currentTime = GetTime();
                double elapsedTime = currentTime - lastClickTime;

                if (elapsedTime >= clickInterval) {
                    //TheAudioManager.playSample("click", 1);
                    lastClickTime += clickInterval;
                    // Increment by the interval to avoid missing clicks
                    std::cout << "Click" << std::endl;
                }

                if (IsKeyPressed(TheGameKeybinds.overdriveBinds.first)) {
                    tapTimes.push_back(currentTime);
                    std::cout << "Input Registered" << std::endl;

                    showInputFeedback = true;
                    inputFeedbackStartTime = currentTime;
                    inputFeedbackAlpha = 1.0f;
                }
            }

            if (showInputFeedback) {
                double currentTime = GetTime();
                double timeSinceInput = currentTime - inputFeedbackStartTime;
                if (timeSinceInput > inputFeedbackDuration) {
                    showInputFeedback = false;
                } else {
                    inputFeedbackAlpha = 1.0f - (timeSinceInput / inputFeedbackDuration);
                }
            }

            if (showInputFeedback) {
                Color feedbackColor = {
                    0, 255, 0, static_cast<unsigned char>(inputFeedbackAlpha * 255)
                };
                DrawTextEx(
                    assets.rubikBold,
                    "Input Registered",
                    { static_cast<float>((GetRenderWidth() - u.hinpct(0.35f)) / 2),
                      static_cast<float>(int(GetRenderHeight() / 2)) },
                    u.hinpct(0.05f),
                    0,
                    feedbackColor
                );
            }

            if (GuiButton(
                    { ((float)GetRenderWidth() / 2) - 350,
                      ((float)GetRenderHeight() - 60),
                      100,
                      60 },
                    "Cancel"
                )) {
                isCalibrating = false;
                // TheGameSettings.AudioOffset = settingsMain.prevAvOffsetMS;
                // settingsMain.inputOffsetMS = settingsMain.prevInputOffsetMS;
                tapTimes.clear();

                TheGameSettings.SaveToFile((TheGameSettings.directory / "settings.json").string());
                TheMenuManager.SwitchScreen(SETTINGS);
            }

            if (GuiButton(
                    { ((float)GetRenderWidth() / 2) + 250,
                      ((float)GetRenderHeight() - 60),
                      100,
                      60 },
                    "Apply"
                )) {
                isCalibrating = false;
                // settingsMain.prevAvOffsetMS = settingsMain.avOffsetMS;
               // settingsMain.prevInputOffsetMS = settingsMain.inputOffsetMS;
                tapTimes.clear();

                TheGameSettings.SaveToFile((TheGameSettings.directory / "settings.json").string());
                TheMenuManager.SwitchScreen(SETTINGS);
            }

            break;
        }
        case MAIN_MENU:
        case SETTINGS:
        case SETTINGSAUDIOVIDEO:
        case SETTINGSGAMEPLAY:
        case SETTINGSCONTROLLER:
        case SETTINGSKEYBOARD:
        case SETTINGSCREDITS:
        case SONG_SELECT:
        case READY_UP:
        case GAMEPLAY:
        case RESULTS:
        case CHART_LOADING_SCREEN:
        case SOUND_TEST:
        case CACHE_LOADING_SCREEN: {
            ActiveMenu->Draw();
            break;
        }
        }
}

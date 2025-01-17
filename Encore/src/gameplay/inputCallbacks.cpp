//
// Created by maria on 17/12/2024.
//
#include "GameplayInputHandler.h"
#include "enctime.h"
#include "gameplayRenderer.h"
#include "settings-old.h"
#include "inputCallbacks.h"

#include "menus/MenuManager.h"
#include "song/audio.h"
#include "song/songlist.h"
#include "users/playerManager.h"

void ManagePausedGame(GameplayInputHandler inputHandler, Player &player) {
    PlayerGameplayStats *&stats = player.stats;
    stats->Paused = !stats->Paused;
    ThePlayerManager.BandStats->Paused = !ThePlayerManager.BandStats->Paused;
    if (ThePlayerManager.BandStats->Paused) {
        TheAudioManager.pauseStreams();
        TheSongTime.Pause();
    } else {
        TheAudioManager.unpauseStreams();
        TheSongTime.Resume();
        for (int i = 0; i < (player.Difficulty == 3 ? 5 : 4); i++) {
            inputHandler.handleInputs(player, i, -1);
        }
    }
}

void keyCallback(GLFWwindow *wind, int key, int scancode, int action, int mods) {
    Player &player = ThePlayerManager.GetActivePlayer(0);
    PlayerGameplayStats *&stats = player.stats;
    SettingsOld &settingsMain = SettingsOld::getInstance();
    GameplayInputHandler inputHandler;

    if (!TheGameRenderer.streamsLoaded) {
        return;
    }
    if (action < 2) {
        // if the key action is NOT repeat (release is 0, press is 1)
        int lane = -2;
        if (key == settingsMain.keybindPause && action == GLFW_PRESS) {
            ManagePausedGame(inputHandler, player);
        } else if ((key == settingsMain.keybindOverdrive
                    || key == settingsMain.keybindOverdriveAlt)) {
            inputHandler.handleInputs(player, -1, action);
        } else if (!player.Bot) {
            if (player.Instrument != PlasticDrums) {
                if (player.Difficulty == 3 || player.ClassicMode) {
                    for (int i = 0; i < 5; i++) {
                        if (key == settingsMain.keybinds5K[i]
                            && !stats->HeldFretsAlt[i]) {
                            if (action == GLFW_PRESS) {
                                stats->HeldFrets[i] = true;
                            } else if (action == GLFW_RELEASE) {
                                stats->HeldFrets[i] = false;
                                stats->OverhitFrets[i] = false;
                            }
                            lane = i;
                        } else if (key == settingsMain.keybinds5KAlt[i]
                                   && !stats->HeldFrets[i]) {
                            if (action == GLFW_PRESS) {
                                stats->HeldFretsAlt[i] = true;
                            } else if (action == GLFW_RELEASE) {
                                stats->HeldFretsAlt[i] = false;
                                stats->OverhitFrets[i] = false;
                            }
                            lane = i;
                        }
                    }
                } else {
                    for (int i = 0; i < 4; i++) {
                        if (key == settingsMain.keybinds4K[i]
                            && !stats->HeldFretsAlt[i]) {
                            if (action == GLFW_PRESS) {
                                stats->HeldFrets[i] = true;
                            } else if (action == GLFW_RELEASE) {
                                stats->HeldFrets[i] = false;
                                stats->OverhitFrets[i] = false;
                            }
                            lane = i;
                        } else if (key == settingsMain.keybinds4KAlt[i]
                                   && !stats->HeldFrets[i]) {
                            if (action == GLFW_PRESS) {
                                stats->HeldFretsAlt[i] = true;
                            } else if (action == GLFW_RELEASE) {
                                stats->HeldFretsAlt[i] = false;
                                stats->OverhitFrets[i] = false;
                            }
                            lane = i;
                        }
                    }
                }
                if (player.ClassicMode) {
                    if (key == settingsMain.keybindStrumUp) {
                        if (action == GLFW_PRESS) {
                            lane = 8008135;
                            stats->UpStrum = true;
                        } else if (action == GLFW_RELEASE) {
                            stats->UpStrum = false;
                            stats->Overstrum = false;
                        }
                    }
                    if (key == settingsMain.keybindStrumDown) {
                        if (action == GLFW_PRESS) {
                            lane = 8008135;
                            stats->DownStrum = true;
                        } else if (action == GLFW_RELEASE) {
                            stats->DownStrum = false;
                            stats->Overstrum = false;
                        }
                    }
                }

                if (lane != -1 && lane != -2) {
                    inputHandler.handleInputs(player, lane, action);
                }
            }
        }
    }
}


void gamepadStateCallback(int joypadID, GLFWgamepadstate state) {
    SettingsOld &settingsMain = SettingsOld::getInstance();
    GameplayInputHandler inputHandler;

    if (TheMenuManager.currentScreen == SONG_SELECT) {
        if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_PRESS) {
            TheSongList.SongSelectOffset -= 1;
        }
        if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] == GLFW_PRESS) {
            TheSongList.SongSelectOffset += 1;
        }
    }
    if (TheMenuManager.currentScreen == GAMEPLAY) {
        Encore::EncoreLog(
            LOG_DEBUG, TextFormat("Attempted input on joystick %01i", joypadID)
        );
        if (!ThePlayerManager.IsGamepadActive(joypadID))
            return;

        Player &player = ThePlayerManager.GetPlayerGamepad(joypadID);
        PlayerGameplayStats *&stats = player.stats;

        if (!TheGameRenderer.streamsLoaded) {
            return;
        }

        double eventTime = TheSongTime.GetSongTime();
        if (settingsMain.controllerPause >= 0) {
            if (state.buttons[settingsMain.controllerPause]
                != stats->buttonValues[settingsMain.controllerPause]) {
                stats->buttonValues[settingsMain.controllerPause] =
                    state.buttons[settingsMain.controllerPause];
                if (state.buttons[settingsMain.controllerPause] == 1) {
                    ManagePausedGame(inputHandler, player); // && !player.Bot
                }
            }
        } else if (!player.Bot) {
            if (state.axes[-(settingsMain.controllerPause + 1)]
                != stats->axesValues[-(settingsMain.controllerPause + 1)]) {
                stats->axesValues[-(settingsMain.controllerPause + 1)] =
                    state.axes[-(settingsMain.controllerPause + 1)];
                if (state.axes[-(settingsMain.controllerPause + 1)]
                    == 1.0f * (float)settingsMain.controllerPauseAxisDirection) {
                }
            }
        } //  && !player.Bot
        if (settingsMain.controllerOverdrive >= 0) {
            if (state.buttons[settingsMain.controllerOverdrive]
                != stats->buttonValues[settingsMain.controllerOverdrive]) {
                stats->buttonValues[settingsMain.controllerOverdrive] =
                    state.buttons[settingsMain.controllerOverdrive];
                inputHandler.handleInputs(
                    player, -1, state.buttons[settingsMain.controllerOverdrive]
                );
            } // // if (!player.Bot)
        } else {
            if (state.axes[-(settingsMain.controllerOverdrive + 1)]
                != stats->axesValues[-(settingsMain.controllerOverdrive + 1)]) {
                stats->axesValues[-(settingsMain.controllerOverdrive + 1)] =
                    state.axes[-(settingsMain.controllerOverdrive + 1)];
                if (state.axes[-(settingsMain.controllerOverdrive + 1)]
                    == 1.0f * (float)settingsMain.controllerOverdriveAxisDirection) {
                    inputHandler.handleInputs(player, -1, GLFW_PRESS);
                } else {
                    inputHandler.handleInputs(player, -1, GLFW_RELEASE);
                }
            }
        }
        if ((player.Difficulty == 3 || player.ClassicMode) && !player.Bot) {
            int lane = -2;
            int action = -2;
            for (int i = 0; i < 5; i++) {
                if (settingsMain.controller5K[i] >= 0) {
                    if (state.buttons[settingsMain.controller5K[i]]
                        != stats->buttonValues[settingsMain.controller5K[i]]) {
                        if (state.buttons[settingsMain.controller5K[i]] == 1
                            && !stats->HeldFrets[i])
                            stats->HeldFrets[i] = true;
                        else if (stats->HeldFrets[i]) {
                            stats->HeldFrets[i] = false;
                            stats->OverhitFrets[i] = false;
                        }
                        inputHandler.handleInputs(
                            player, i, state.buttons[settingsMain.controller5K[i]]
                        );
                        stats->buttonValues[settingsMain.controller5K[i]] =
                            state.buttons[settingsMain.controller5K[i]];
                        lane = i;
                    }
                } else {
                    if (state.axes[-(settingsMain.controller5K[i] + 1)]
                        != stats->axesValues[-(settingsMain.controller5K[i] + 1)]) {
                        if (state.axes[-(settingsMain.controller5K[i] + 1)]
                                == 1.0f * (float)settingsMain.controller5KAxisDirection[i]
                            && !stats->HeldFrets[i]) {
                            stats->HeldFrets[i] = true;
                            inputHandler.handleInputs(player, i, GLFW_PRESS);
                        } else if (stats->HeldFrets[i]) {
                            stats->HeldFrets[i] = false;
                            stats->OverhitFrets[i] = false;
                            inputHandler.handleInputs(player, i, GLFW_RELEASE);
                        }
                        stats->axesValues[-(settingsMain.controller5K[i] + 1)] =
                            state.axes[-(settingsMain.controller5K[i] + 1)];
                        lane = i;
                    }
                }
            }

            if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_PRESS
                && player.ClassicMode && !stats->UpStrum) {
                stats->UpStrum = true;
                stats->Overstrum = false;
                inputHandler.handleInputs(player, 8008135, GLFW_PRESS);
            } else if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_RELEASE
                       && player.ClassicMode && stats->UpStrum) {
                stats->UpStrum = false;
                inputHandler.handleInputs(player, 8008135, GLFW_RELEASE);
            }
            if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] == GLFW_PRESS
                && player.ClassicMode && !stats->DownStrum) {
                stats->DownStrum = true;
                stats->Overstrum = false;
                inputHandler.handleInputs(player, 8008135, GLFW_PRESS);
            } else if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] == GLFW_RELEASE
                       && player.ClassicMode && stats->DownStrum) {
                stats->DownStrum = false;
                inputHandler.handleInputs(player, 8008135, GLFW_RELEASE);
            }
        } else if (!player.Bot) {
            for (int i = 0; i < 4; i++) {
                if (settingsMain.controller4K[i] >= 0) {
                    if (state.buttons[settingsMain.controller4K[i]]
                        != stats->buttonValues[settingsMain.controller4K[i]]) {
                        if (state.buttons[settingsMain.controller4K[i]] == 1)
                            stats->HeldFrets[i] = true;
                        else {
                            stats->HeldFrets[i] = false;
                            stats->OverhitFrets[i] = false;
                        }
                        inputHandler.handleInputs(
                            player, i, state.buttons[settingsMain.controller4K[i]]
                        );
                        stats->buttonValues[settingsMain.controller4K[i]] =
                            state.buttons[settingsMain.controller4K[i]];
                    }
                } else {
                    if (state.axes[-(settingsMain.controller4K[i] + 1)]
                        != stats->axesValues[-(settingsMain.controller4K[i] + 1)]) {
                        if (state.axes[-(settingsMain.controller4K[i] + 1)]
                            == 1.0f * (float)settingsMain.controller4KAxisDirection[i]) {
                            stats->HeldFrets[i] = true;
                            inputHandler.handleInputs(player, i, GLFW_PRESS);
                        } else {
                            stats->HeldFrets[i] = false;
                            stats->OverhitFrets[i] = false;
                            inputHandler.handleInputs(player, i, GLFW_RELEASE);
                        }
                        stats->axesValues[-(settingsMain.controller4K[i] + 1)] =
                            state.axes[-(settingsMain.controller4K[i] + 1)];
                    }
                }
            }
        }
    }
}
//
// Created by marie on 20/10/2024.
//

#include "GameplayMenu.h"

#include "gameplay/GameplayInputHandler.h"
#include "gameMenu.h"
#include "overshellRenderer.h"
#include "uiUnits.h"
#include "song/audio.h"
#include "song/songlist.h"
#include "raymath.h"
#include "raygui.h"
#include "gameplay/enctime.h"
#include "styles.h"
#include "easing/easing.h"
#include "gameplay/gameplayRenderer.h"
#include "users/playerManager.h"
#include "MenuManager.h"
#include "OvershellHelper.h"
#include "settings-old.h"
#include "settings.h"
#include "GLFW/glfw3.h"

#include <raylib.h>

GameplayMenu::GameplayMenu() {}
GameplayMenu::~GameplayMenu() {}

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

void GameplayMenu::KeyboardInputCallback(int key, int scancode, int action, int mods) {
    Encore::EncoreLog(
        LOG_DEBUG,
        TextFormat(
            "Keyboard key %01i inputted on menu %s, action ",
            key,
            ToString(TheMenuManager.currentScreen),
            action
        )
    );
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
                Encore::EncoreLog(LOG_DEBUG, TextFormat("Keyboard key lane %01i", lane));
                if (lane != -1 && lane != -2) {
                    inputHandler.handleInputs(player, lane, action);
                    Encore::EncoreLog(LOG_DEBUG, "Sent key input");
                }
            }
        }
    }
};
void GameplayMenu::ControllerInputCallback(int joypadID, GLFWgamepadstate state) {
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
};
void GameplayMenu::DrawScorebox(Units &u, Assets &assets, float scoreY) {
    Rectangle scoreboxSrc {
        0, 0, float(assets.Scorebox.width), float(assets.Scorebox.height)
    };
    float WidthOfScorebox = u.hinpct(0.28);
    // float scoreY = u.hpct(0.15f);
    float ScoreboxX = u.RightSide;
    float ScoreboxY = u.hpct(0.1425f);
    float HeightOfScorebox = WidthOfScorebox / 4;
    Rectangle scoreboxDraw { ScoreboxX, ScoreboxY, WidthOfScorebox, HeightOfScorebox };
    DrawTexturePro(
        assets.Scorebox, scoreboxSrc, scoreboxDraw, { WidthOfScorebox, 0 }, 0, WHITE
    );
    GameMenu::mhDrawText(
        assets.redHatMono,
        GameMenu::scoreCommaFormatter(ThePlayerManager.BandStats->Score),
        { u.RightSide - u.winpct(0.0145f), scoreY + u.hinpct(0.0025) },
        u.hinpct(0.05),
        Color { 107, 161, 222, 255 },
        assets.sdfShader,
        RIGHT
    );
}

void GameplayMenu::DrawTimerbox(Units &u, Assets &assets, float scoreY) {
    Rectangle TimerboxSrc {
        0, 0, float(assets.Timerbox.width), float(assets.Timerbox.height)
    };
    float WidthOfTimerbox = u.hinpct(0.14);
    // float scoreY = u.hpct(0.15f);
    float TimerboxX = u.RightSide;
    float TimerboxY = u.hpct(0.1425f);
    float HeightOfTimerbox = WidthOfTimerbox / 4;
    Rectangle TimerboxDraw { TimerboxX, TimerboxY, WidthOfTimerbox, HeightOfTimerbox };
    DrawTexturePro(
        assets.Timerbox,
        TimerboxSrc,
        TimerboxDraw,
        { WidthOfTimerbox, HeightOfTimerbox },
        0,
        WHITE
    );
    int played = TheSongTime.GetSongTime();
    int length = TheSongTime.GetSongLength();
    float Width = Remap(played, 0, length, 0, WidthOfTimerbox);
    BeginScissorMode(
        TimerboxX - WidthOfTimerbox,
        TimerboxY - HeightOfTimerbox,
        Width + 1,
        HeightOfTimerbox + 1
    );
    DrawTexturePro(
        assets.TimerboxOutline,
        TimerboxSrc,
        TimerboxDraw,
        { WidthOfTimerbox, HeightOfTimerbox },
        0,
        WHITE
    );
    EndScissorMode();
    int playedMinutes = played / 60;
    int playedSeconds = played % 60;
    int songMinutes = length / 60;
    int songSeconds = length % 60;
    const char *textTime = TextFormat(
        "%i:%02i / %i:%02i", playedMinutes, playedSeconds, songMinutes, songSeconds
    );
    GameMenu::mhDrawText(
        assets.rubik,
        textTime,
        { u.RightSide - (WidthOfTimerbox / 2), scoreY - u.hinpct(SmallHeader) },
        u.hinpct(SmallHeader * 0.66),
        WHITE,
        assets.sdfShader,
        CENTER
    );
}

void GameplayMenu::DrawGameplayStars(
    Units &u, Assets &assets, float scorePos, float starY
) {
    int starsval = ThePlayerManager.BandStats->Stars();
    float starPercent = (float)ThePlayerManager.BandStats->Score
        / (float)ThePlayerManager.BandStats->BaseScore;
    for (int i = 0; i < 5; i++) {
        bool firstStar = (i == 0);
        float starX = scorePos - u.hinpct(0.26) + (i * u.hinpct(0.0525));
        float starWH = u.hinpct(0.05);
        Rectangle emptyStarWH = {
            0, 0, (float)assets.emptyStar.width, (float)assets.emptyStar.height
        };
        Rectangle starRect = { starX, starY, starWH, starWH };
        DrawTexturePro(assets.emptyStar, emptyStarWH, starRect, { 0, 0 }, 0, WHITE);
        float yMaskPos = Remap(
            starPercent,
            firstStar ? 0 : BAND_STAR_THRESHOLD[i - 1],
            BAND_STAR_THRESHOLD[i],
            0,
            u.hinpct(0.05)
        );
        BeginScissorMode(starX, (starY + starWH) - yMaskPos, starWH, yMaskPos);
        DrawTexturePro(
            assets.star,
            emptyStarWH,
            starRect,
            { 0, 0 },
            0,
            i == starsval ? Color { 192, 192, 192, 128 } : WHITE
        );
        EndScissorMode();
    }
    if (starPercent >= BAND_STAR_THRESHOLD[4]
        && ThePlayerManager.BandStats->EligibleForGoldStars) {
        float starWH = u.hinpct(0.05);
        Rectangle emptyStarWH = {
            0, 0, (float)assets.goldStar.width, (float)assets.goldStar.height
        };
        float yMaskPos = Remap(
            starPercent, BAND_STAR_THRESHOLD[4], BAND_STAR_THRESHOLD[5], 0, u.hinpct(0.05)
        );
        BeginScissorMode(
            scorePos - (starWH * 6), (starY + starWH) - yMaskPos, scorePos, yMaskPos
        );
        for (int i = 0; i < 5; i++) {
            float starX = scorePos - u.hinpct(0.26) + (i * u.hinpct(0.0525));
            Rectangle starRect = { starX, starY, starWH, starWH };
            DrawTexturePro(
                ThePlayerManager.BandStats->GoldStars() ? assets.goldStar
                                                        : assets.goldStarUnfilled,
                emptyStarWH,
                starRect,
                { 0, 0 },
                0,
                WHITE
            );
        }
        EndScissorMode();
    }
}

unsigned char BeatToCharViaTickThing(
    int tick, int MinBrightness, int MaxBrightness, int QuarterNoteLength
) {
    float TickModulo = tick % QuarterNoteLength;
    return Remap(
        TickModulo / float(QuarterNoteLength), 0, 1.0f, MaxBrightness, MinBrightness
    );
}
double TimeRangeToTickDelta(double timeStart, double timeEnd, BPM bpm) {
    double timeDelta = timeEnd - timeStart;
    double beatDelta = timeDelta * bpm.bpm / 60.0;
    return beatDelta * 480.0;
}
void GameplayMenu::Draw() {
    Units &u = Units::getInstance();
    Assets &assets = Assets::getInstance();
    // SettingsOld &settings = SettingsOld::getInstance();

    //    OvershellRenderer osr;
    double curTime = GetTime();

    // IMAGE BACKGROUNDS??????
    ClearBackground(BLACK);
    unsigned char BackgroundColor = 0;
    if (ThePlayerManager.BandStats->PlayersInOverdrive > 0) {
        BackgroundColor = BeatToCharViaTickThing(TheGameRenderer.CurrentTick, 0, 8, 960);
    }

    GameMenu::DrawAlbumArtBackground(TheSongList.curSong->albumArtBlur);
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color { 0, 0, 0, 128 });
    DrawRectangle(
        0, 0, GetScreenWidth(), GetScreenHeight(), Color { 255, 255, 255, BackgroundColor }
    );

    // ClearBackground({ 0, 0, 0, 0 });
    /* Band Multiplier Drawing
    float bandMult = u.RightSide - WidthOfScorebox;
    GameMenu::mhDrawText(
        assets.redHatDisplayItalicLarge,
        TextFormat(
            "%01ix",
            ThePlayerManager.BandStats
                .OverdriveMultiplier[ThePlayerManager.BandStats->PlayersInOverdrive]
        ),
        { bandMult, scoreY },
        u.hinpct(0.05),
        RAYWHITE,
        assets.sdfShader,
        RIGHT
    );
    */

    if (!TheGameRenderer.streamsLoaded) {
        TheAudioManager.loadStreams(TheSongList.curSong->stemsPath);
        TheGameRenderer.streamsLoaded = true;
    } else {
        for (int i = 0; i < ThePlayerManager.PlayersActive; i++) {
            for (auto &stream : TheAudioManager.loadedStreams) {
                Player &player = ThePlayerManager.GetActivePlayer(i);
                if ((player.ClassicMode ? player.Instrument - 5 : player.Instrument)
                    == stream.instrument) {
                    TheAudioManager.SetAudioStreamVolume(
                        stream.handle,
                        player.stats->Mute
                            ? TheGameSettings.avMainVolume * TheGameSettings.avMuteVolume
                            : TheGameSettings.avMainVolume
                                * TheGameSettings.avActiveInstrumentVolume
                    );
                } else {
                    TheAudioManager.SetAudioStreamVolume(
                        stream.handle,
                        TheGameSettings.avMainVolume
                            * TheGameSettings.avInactiveInstrumentVolume
                    );
                }
            }
        }

        float songPlayed = TheSongTime.GetSongLength();

        if (TheSongTime.SongComplete()) {
            TheGameRenderer.LowerHighway();
        }
        if (TheSongTime.SongComplete()) {
            TheSongList.curSong->LoadAlbumArt();
            TheGameRenderer.midiLoaded = false;
            TheGameRenderer.highwayInAnimation = false;
            TheGameRenderer.songPlaying = false;
            TheGameRenderer.highwayLevel = 0;
            TheSongTime.Stop();
            if (TheGameRenderer.streamsLoaded) {
                TheAudioManager.unloadStreams();
                TheGameRenderer.streamsLoaded = false;
            }
            TheMenuManager.SwitchScreen(RESULTS);
            Encore::EncoreLog(LOG_INFO, TextFormat("Song ended at at %f", songPlayed));
            return;
        }
    }

    for (int pnum = 0; pnum < ThePlayerManager.PlayersActive; pnum++) {
        Player &curPlayer = ThePlayerManager.GetActivePlayer(pnum);
        TheGameRenderer.cameraSel =
            CameraSelectionPerPlayer[ThePlayerManager.PlayersActive - 1][pnum];
        int pos = CameraPosPerPlayer[ThePlayerManager.PlayersActive - 1][pnum];
        if (pos == 0)
            TheGameRenderer.renderPos =
                CameraPosPerPlayer[ThePlayerManager.PlayersActive - 1][pnum];
        else
            TheGameRenderer.renderPos = GetScreenWidth()
                / CameraPosPerPlayer[ThePlayerManager.PlayersActive - 1][pnum];
        //for (int n = 0; n < TheSongList.curSong->parts[curPlayer.Instrument]
        //                        ->charts[curPlayer.Difficulty]
        //                        .notes.size();
        //     ++n) {
        //
        //}
        TheGameRenderer.CurrentTick = TheSongList.curSong->bpms[curPlayer.stats->curBPM].tick
                + TimeRangeToTickDelta(
                              TheSongList.curSong->bpms[curPlayer.stats->curBPM].time,
                              TheSongTime.GetSongTime(),
                              TheSongList.curSong->bpms[curPlayer.stats->curBPM]
                );

        if (curPlayer.ClassicMode) {
            Chart &curChart = curPlayer.stats->CurPlayingChart;
            if (curPlayer.stats->curNoteInt < curChart.notes.size()) {
                Note &curNote = curChart.notes.at(curPlayer.stats->curNoteInt);

                double OverdriveDrainPerTick = double(OVERDRIVE_DRAIN_PER_BEAT) / 480.0;
                if (curPlayer.stats->Overdrive) {
                    // THIS IS LOGIC!
                    curPlayer.stats->overdriveFill -=
                        (TheGameRenderer.CurrentTick - curPlayer.stats->LastTick) * OverdriveDrainPerTick;
                    /*
                    player.stats->overdriveFill = player.stats->overdriveActiveFill
                        - (float)((curSongTime - player.stats->overdriveActiveTime)
                                  / (1920 / song.bpms[player.stats->curBPM].bpm));
                                  */
                    if (curPlayer.stats->overdriveFill <= 0) {
                        curPlayer.stats->overdriveActivateTime = TheSongTime.GetSongTime();
                        curPlayer.stats->Overdrive = false;
                        curPlayer.stats->overdriveFill = 0;
                        curPlayer.stats->overdriveActiveFill = 0;
                        curPlayer.stats->overdriveActiveTime = 0.0;
                        ThePlayerManager.BandStats->PlayersInOverdrive -= 1;
                        ThePlayerManager.BandStats->Overdrive = false;
                    }
                }

                for (int i = curPlayer.stats->curBPM; i < TheSongList.curSong->bpms.size(); i++) {
                    if (TheSongTime.GetSongTime() > TheSongList.curSong->bpms[i].time && i < TheSongList.curSong->bpms.size() - 1)
                        curPlayer.stats->curBPM++;
                }

                TheGameRenderer.CheckPlasticNotes(
                    curPlayer,
                    curChart,
                    TheSongTime.GetSongTime(),
                    curPlayer.stats->curNoteInt
                );
                curChart.overdrive.CheckEvents(curPlayer.stats->curODPhrase, TheSongTime.GetSongTime());
                curChart.solos.CheckEvents(curPlayer.stats->curSolo, TheSongTime.GetSongTime());
                curChart.fills.CheckEvents(curPlayer.stats->curFill, TheSongTime.GetSongTime());
                curChart.sections.CheckEvents(curPlayer.stats->curFill, TheSongTime.GetSongTime());

                if (curNote.len > 0) {
                    for (auto cLane : curNote.pLanes) {
                        int lane = cLane.lane;
                        if (curNote.held) {
                            cLane.heldTime = TheSongTime.GetSongTime() - curNote.time;
                            if (cLane.heldTime >= cLane.length) {
                                cLane.heldTime = cLane.length;
                            }
                            TheGameRenderer.CalculateSustainScore(curPlayer.stats);
                            if (!((curPlayer.stats->PressedMask >> lane) & 1) && !curPlayer.Bot) {
                                curNote.held = false;
                            }
                        }
                        if (cLane.length <= cLane.heldTime) {
                            curNote.held = false;
                        }
                    }
                }
            }
            curPlayer.stats->LastTick = TheGameRenderer.CurrentTick;
        }
        TheGameRenderer.RenderGameplay(
            curPlayer, TheSongTime.GetSongTime(), *TheSongList.curSong
        );
        std::string NameText = curPlayer.Name;
        if (curPlayer.Bot)
            NameText.append(" - AUTOPLAY");
        float CenterPosForText =
            GetWorldToScreen(
                { 0, 0, 0 },
                TheGameRenderer.cameraVectors[ThePlayerManager.PlayersActive - 1]
                                             [TheGameRenderer.cameraSel]
            )
                .x;
        float fontSize = u.hinpct(0.035);
        float textWidth =
            MeasureTextEx(assets.rubikBold, NameText.c_str(), fontSize, 0).x;
        Color headerUsernameColor;
        if (curPlayer.Bot)
            headerUsernameColor = SKYBLUE;
        else {
            if (curPlayer.BrutalMode)
                headerUsernameColor = RED;
            else
                headerUsernameColor = WHITE;
        }
        DrawTextEx(
            assets.rubikBold,
            NameText.c_str(),
            { (CenterPosForText - (textWidth / 2)) - (TheGameRenderer.renderPos),
              GetScreenHeight() - u.hinpct(0.04) },
            fontSize,
            0,
            headerUsernameColor
        );
    }

    float scorePos = u.RightSide - u.hinpct(0.01f);
    float scoreY = u.hpct(0.15f);
    float starY = scoreY + u.hinpct(0.065f);
    // Draw Stars
    DrawGameplayStars(u, assets, scorePos, starY);

    // Draw Timerbox
    DrawTimerbox(u, assets, scoreY);

    // Score Drawing
    DrawScorebox(u, assets, scoreY);

    float SongNameWidth = MeasureTextEx(
                              assets.rubikBoldItalic,
                              TheSongList.curSong->title.c_str(),
                              u.hinpct(MediumHeader),
                              0
    )
                              .x;
    std::string SongArtistString = TheSongList.curSong->artist + ", "
        + TheSongList.curSong->releaseYear;
    float SongArtistWidth =
        MeasureTextEx(
            assets.rubikBoldItalic, SongArtistString.c_str(), u.hinpct(SmallHeader), 0
        )
            .x;

    float SongExtrasWidth = MeasureTextEx(
                                assets.rubikBoldItalic,
                                TheSongList.curSong->charters[0].c_str(),
                                u.hinpct(SmallHeader),
                                0
    )
                                .x;

    double SongNameDuration = 0.75f;
    unsigned char SongNameAlpha = 255;
    float SongNamePosition = 35;
    unsigned char SongArtistAlpha = 255;
    float SongArtistPosition = 35;
    unsigned char SongExtrasAlpha = 255;
    float SongExtrasPosition = 35;
    float SongNameBackgroundWidth =
        SongNameWidth >= SongArtistWidth ? SongNameWidth : SongArtistWidth;
    float SongBackgroundWidth = SongNameBackgroundWidth;
    if (curTime > TheSongTime.GetStartTime() + 7.5
        && curTime < TheSongTime.GetStartTime() + 7.5 + SongNameDuration) {
        double timeSinceStart = GetTime() - (TheSongTime.GetStartTime() + 7.5);
        SongNameAlpha = static_cast<unsigned char>(Remap(
            static_cast<float>(
                getEasingFunction(EaseOutCirc)(timeSinceStart / SongNameDuration)
            ),
            0,
            1.0,
            255,
            0
        ));
        SongNamePosition = Remap(
            static_cast<float>(
                getEasingFunction(EaseInOutBack)(timeSinceStart / SongNameDuration)
            ),
            0,
            1.0,
            35,
            -SongNameWidth
        );
    } else if (curTime > TheSongTime.GetStartTime() + 7.5 + SongNameDuration)
        SongNameAlpha = 0;

    if (curTime > TheSongTime.GetStartTime() + 7.75
        && curTime < TheSongTime.GetStartTime() + 7.75 + SongNameDuration) {
        double timeSinceStart = GetTime() - (TheSongTime.GetStartTime() + 7.75);
        SongArtistAlpha = static_cast<unsigned char>(Remap(
            static_cast<float>(
                getEasingFunction(EaseOutCirc)(timeSinceStart / SongNameDuration)
            ),
            0,
            1.0,
            255,
            0
        ));

        SongArtistPosition = Remap(
            static_cast<float>(
                getEasingFunction(EaseInOutBack)(timeSinceStart / SongNameDuration)
            ),
            0,
            1.0,
            35,
            -SongArtistWidth
        );
    }
    if (curTime > TheSongTime.GetStartTime() + 8
        && curTime < TheSongTime.GetStartTime() + 8 + SongNameDuration) {
        double timeSinceStart = GetTime() - (TheSongTime.GetStartTime() + 8);
        SongExtrasAlpha = static_cast<unsigned char>(Remap(
            static_cast<float>(
                getEasingFunction(EaseOutCirc)(timeSinceStart / SongNameDuration)
            ),
            0,
            1.0,
            255,
            0
        ));
        SongBackgroundWidth = Remap(
            static_cast<float>(
                getEasingFunction(EaseInOutCirc)(timeSinceStart / SongNameDuration)
            ),
            0,
            1.0,
            SongNameBackgroundWidth,
            0
        );

        SongExtrasPosition = Remap(
            static_cast<float>(
                getEasingFunction(EaseInOutBack)(timeSinceStart / SongNameDuration)
            ),
            0,
            1.0,
            35,
            -SongExtrasWidth
        );
    }
    if (curTime < TheSongTime.GetStartTime() + 7.75 + SongNameDuration) {
        DrawRectangleGradientH(
            0,
            u.hpct(0.19f),
            1.25 * SongBackgroundWidth,
            u.hinpct(0.02f + MediumHeader + SmallHeader + SmallHeader),
            Color { 0, 0, 0, 128 },
            Color { 0, 0, 0, 0 }
        );
        DrawTextEx(
            assets.rubikBoldItalic,
            TheSongList.curSong->title.c_str(),
            { SongNamePosition, u.hpct(0.2f) },
            u.hinpct(MediumHeader),
            0,
            Color { 255, 255, 255, SongNameAlpha }
        );
        DrawTextEx(
            assets.rubikItalic,
            SongArtistString.c_str(),
            { SongArtistPosition, u.hpct(0.2f + MediumHeader) },
            u.hinpct(SmallHeader),
            0,
            Color { 200, 200, 200, SongArtistAlpha }
        );
        DrawTextEx(
            assets.rubikItalic,
            TheSongList.curSong->charters[0].c_str(),
            { SongExtrasPosition, u.hpct(0.2f + MediumHeader + SmallHeader) },
            u.hinpct(SmallHeader),
            0,
            Color { 200, 200, 200, SongExtrasAlpha }
        );
    }

    int songLength;
    if (TheSongList.curSong->end == 0)
        songLength = static_cast<int>(TheAudioManager.GetMusicTimeLength());
    else
        songLength = static_cast<int>(TheSongList.curSong->end);

    GuiSetStyle(PROGRESSBAR, BORDER_WIDTH, 0);
    // GuiSetStyle(PROGRESSBAR, BASE_COLOR_NORMAL,
    //			ColorToInt(player.FC ? GOLD : AccentColor));
    // GuiSetStyle(PROGRESSBAR, BASE_COLOR_FOCUSED,
    //			ColorToInt(player.FC ? GOLD : AccentColor));
    // GuiSetStyle(PROGRESSBAR, BASE_COLOR_DISABLED,
    //			ColorToInt(player.FC ? GOLD : AccentColor));
    // GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED,
    //			ColorToInt(player.FC ? GOLD : AccentColor));
    GuiSetStyle(DEFAULT, TEXT_SIZE, static_cast<int>(u.hinpct(0.03f)));
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    GuiSetFont(assets.rubik);

    float floatSongLength = TheAudioManager.GetMusicTimePlayed();

    if (ThePlayerManager.BandStats->Paused) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color { 0, 0, 0, 80 });
        encOS::DrawTopOvershell(0.2f);
        SET_LARGE_BUTTON_STYLE();
        float Left = u.wpct(0.02f);
        float Width = u.winpct(0.2f);
        float Height = u.hinpct(0.08f);
        float Top = u.hpct(0.3f);
        float Spacing = u.hinpct(0.09f);
        Rectangle ResumeBox = { Left, Top, Width, Height };
        Rectangle RestartBox = { Left, Top + Spacing, Width, Height };
        Rectangle QuitBox = { Left, Top + (Spacing * 2), Width, Height };

        if (GuiButton(ResumeBox, "Resume")) {
            TheAudioManager.unpauseStreams();
            TheSongTime.Resume();
            ThePlayerManager.BandStats->Paused = false;
            for (int playerNum = 0; playerNum < ThePlayerManager.PlayersActive;
                 playerNum++) {
                ThePlayerManager.GetActivePlayer(playerNum).stats->Paused = false;
            }
        }
        if (GuiButton(RestartBox, "Restart")) {
            TheSongTime.Reset();
            TheGameRenderer.highwayInAnimation = false;
            TheGameRenderer.highwayInEndAnim = false;
            TheGameRenderer.songPlaying = false;
            TheGameRenderer.Restart = true;
            delete ThePlayerManager.BandStats;
            ThePlayerManager.BandStats = new BandGameplayStats;
            for (int playerNum = 0; playerNum < ThePlayerManager.PlayersActive;
                 playerNum++) {
                delete ThePlayerManager.GetActivePlayer(playerNum).stats;
                ThePlayerManager.GetActivePlayer(playerNum).stats =
                    new PlayerGameplayStats(
                        ThePlayerManager.GetActivePlayer(playerNum).Difficulty,
                        ThePlayerManager.GetActivePlayer(playerNum).Instrument
                    );
                ThePlayerManager.GetActivePlayer(playerNum).stats->CurPlayingChart = TheSongList.curSong->parts[ThePlayerManager.GetActivePlayer(playerNum).Instrument]->charts[ThePlayerManager.GetActivePlayer(playerNum).Difficulty];
            }

            ThePlayerManager.BandStats->ResetBandGameplayStats();
            ThePlayerManager.BandStats->Paused = false;
        }
        if (GuiButton(QuitBox, "Back to Music Library")) {
            // notes =
            // TheSongList.curSong->parts[instrument]->charts[diff].notes.size();
            // notes = TheSongList.curSong->parts[instrument]->charts[diff];

            TheSongList.curSong->LoadAlbumArt();
            ThePlayerManager.BandStats->ResetBandGameplayStats();
            TheGameRenderer.midiLoaded = false;
            TheSongTime.Reset();

            TheAudioManager.unloadStreams();
            TheGameRenderer.highwayInAnimation = false;
            TheGameRenderer.highwayInEndAnim = false;
            TheGameRenderer.songPlaying = false;
            for (int playerNum = 0; playerNum < ThePlayerManager.PlayersActive;
                 playerNum++) {
                ThePlayerManager.GetActivePlayer(playerNum).stats->CurPlayingChart.resetNotes();
                ThePlayerManager.GetActivePlayer(playerNum).stats->Quit = true;
            }
            TheMenuManager.SwitchScreen(RESULTS);
            SETDEFAULTSTYLE();
            return;
        }
        SETDEFAULTSTYLE();

        DrawTextEx(
            assets.rubikBoldItalic,
            "PAUSED",
            { u.wpct(0.02f), u.hpct(0.05f) },
            u.hinpct(0.1f),
            0,
            WHITE
        );

        float SongFontSize = u.hinpct(0.03f);

        float TitleHeight =
            MeasureTextEx(
                assets.rubikBoldItalic, TheSongList.curSong->title.c_str(), SongFontSize, 0
            )
                .y;
        float TitleWidth =
            MeasureTextEx(
                assets.rubikBoldItalic, TheSongList.curSong->title.c_str(), SongFontSize, 0
            )
                .x;
        float ArtistHeight =
            MeasureTextEx(
                assets.rubikItalic, TheSongList.curSong->artist.c_str(), SongFontSize, 0
            )
                .y;
        float ArtistWidth =
            MeasureTextEx(
                assets.rubikItalic, TheSongList.curSong->artist.c_str(), SongFontSize, 0
            )
                .x;
        if (!ThePlayerManager.BandStats->Multiplayer) {
            const char *instDiffText = TextFormat(
                "%s %s",
                diffList[ThePlayerManager.GetActivePlayer(0).Difficulty].c_str(),
                songPartsList[ThePlayerManager.GetActivePlayer(0).Instrument].c_str()
            );
            float InstDiffHeight =
                MeasureTextEx(assets.rubikBold, instDiffText, SongFontSize, 0).y;
            float InstDiffWidth =
                MeasureTextEx(assets.rubikBold, instDiffText, SongFontSize, 0).x;
            Vector2 SongInstDiffBox = { u.RightSide - InstDiffWidth - u.winpct(0.01f),
                                        u.hpct(0.1f) + (ArtistHeight / 2)
                                            + (InstDiffHeight * 0.1f) };
            DrawTextEx(
                assets.rubikBold, instDiffText, SongInstDiffBox, SongFontSize, 0, WHITE
            );
        }

        Vector2 SongTitleBox = { u.RightSide - TitleWidth - u.winpct(0.01f),
                                 u.hpct(0.1f) - (ArtistHeight / 2)
                                     - (TitleHeight * 1.1f) };
        Vector2 SongArtistBox = { u.RightSide - ArtistWidth - u.winpct(0.01f),
                                  u.hpct(0.1f) - (ArtistHeight / 2) };

        DrawTextEx(
            assets.rubikBoldItalic,
            TheSongList.curSong->title.c_str(),
            SongTitleBox,
            SongFontSize,
            0,
            WHITE
        );
        DrawTextEx(
            assets.rubikItalic,
            TheSongList.curSong->artist.c_str(),
            SongArtistBox,
            SongFontSize,
            0,
            WHITE
        );

        DrawOvershell();
    }

    GameMenu::DrawFPS(u.LeftSide, u.hpct(0.0025f) + u.hinpct(0.025f));
    GameMenu::DrawVersion();

    if (!ThePlayerManager.BandStats->Multiplayer
        && ThePlayerManager.GetActivePlayer(0).stats->Health <= 0) {
        TheSongList.curSong->LoadAlbumArt();
        ThePlayerManager.BandStats->ResetBandGameplayStats();
        TheGameRenderer.midiLoaded = false;
        TheSongTime.Reset();

        TheAudioManager.unloadStreams();
        TheGameRenderer.highwayInAnimation = false;
        TheGameRenderer.highwayInEndAnim = false;
        TheGameRenderer.songPlaying = false;

        TheSongList.curSong->parts[ThePlayerManager.GetActivePlayer(0).Instrument]
            ->charts[ThePlayerManager.GetActivePlayer(0).Difficulty]
            .resetNotes();
        ThePlayerManager.GetActivePlayer(0).stats->Quit = true;
        TheMenuManager.SwitchScreen(RESULTS);
    }
    // if (!TheGameRenderer.bot)
    //	DrawTextEx(assets.rubikBold, TextFormat("%s", player.FC ? "FC" : ""),
    //				{5, GetScreenHeight() - u.hinpct(0.05f)}, u.hinpct(0.04), 0,
    //				GOLD);
    // if (TheGameRenderer.bot)
    //	DrawTextEx(assets.rubikBold, "BOT",
    //				{5, GetScreenHeight() - u.hinpct(0.05f)}, u.hinpct(0.04), 0,
    //				SKYBLUE);
    // if (!TheGameRenderer.bot)
    /*
    GuiProgressBar(
        Rectangle { 0,
                    (float)GetScreenHeight() - u.hinpct(0.005f),
                    (float)GetScreenWidth(),
                    u.hinpct(0.01f) },
        "",
        "",
        &floatSongLength,
        0,
        (float)songLength
    );

    std::string ScriptDisplayString = "";
    lua.script_file("scripts/testing.lua");
    ScriptDisplayString = lua["TextDisplay"];
    DrawTextEx(assets.rubikBold, ScriptDisplayString.c_str(),
                                    {5, GetScreenHeight() - u.hinpct(0.1f)},
    u.hinpct(0.04), 0, GOLD);
    */

    if (ThePlayerManager.PlayersActive) {
        DrawRectangle(
            u.wpct(0.5f) - (u.winpct(0.12f) / 2),
            u.hpct(0.02f) - u.winpct(0.01f),
            u.winpct(0.12f),
            u.winpct(0.065f),
            DARKGRAY
        );
        for (int fretBox = 0;
             fretBox < ThePlayerManager.GetActivePlayer(0).stats->HeldFrets.size();
             fretBox++) {
            float leftInputBoxSize = (5 * u.winpct(0.02f)) / 2;

            Color fretColor;
            switch (fretBox) {
            default:
                fretColor = BROWN;
                break;
            case (0):
                fretColor = GREEN;
                break;
            case (1):
                fretColor = RED;
                break;
            case (2):
                fretColor = YELLOW;
                break;
            case (3):
                fretColor = BLUE;
                break;
            case (4):
                fretColor = ORANGE;
                break;
            }

            DrawRectangle(
                u.wpct(0.5f) - leftInputBoxSize + (fretBox * u.winpct(0.02f)),
                u.hpct(0.02f),
                u.winpct(0.02f),
                u.winpct(0.02f),
                ThePlayerManager.GetActivePlayer(0).stats->HeldFrets[fretBox]
                        || ThePlayerManager.GetActivePlayer(0).stats->HeldFretsAlt[fretBox]
                    ? fretColor
                    : GRAY
            );
        }
        DrawRectangle(
            u.wpct(0.5f) - ((5 * u.winpct(0.02f)) / 2),
            u.hpct(0.02f) + u.winpct(0.025f),
            u.winpct(0.1f),
            u.winpct(0.01f),
            ThePlayerManager.GetActivePlayer(0).stats->UpStrum ? WHITE : GRAY
        );
        DrawRectangle(
            u.wpct(0.5f) - ((5 * u.winpct(0.02f)) / 2),
            u.hpct(0.02f) + u.winpct(0.035f),
            u.winpct(0.1f),
            u.winpct(0.01f),
            ThePlayerManager.GetActivePlayer(0).stats->DownStrum ? WHITE : GRAY
        );
    }
    /*
    DrawTextEx(
        assets.rubik,
        TextFormat("song time: %f", TheSongTime.GetSongTime()),
        { 0, u.hpct(0.5f) },
        u.hinpct(SmallHeader),
        0,
        WHITE
    );
    DrawTextEx(
        assets.rubik,
        TextFormat("audio time: %f", TheAudioManager.GetMusicTimePlayed()),
        { 0, u.hpct(0.5f + SmallHeader) },
        u.hinpct(SmallHeader),
        0,
        WHITE
    );
    */
}

void GameplayMenu::Load() {
    TheSongList.curSong->LoadAlbumArt();
    if (ThePlayerManager.PlayersActive > 1) {
        ThePlayerManager.BandStats->Multiplayer = true;
        for (int player = 0; player < ThePlayerManager.PlayersActive; player++) {
            ThePlayerManager.GetActivePlayer(player).stats->Multiplayer = true;
        }
    } else {
        ThePlayerManager.BandStats->Multiplayer = false;
        for (int player = 0; player < ThePlayerManager.PlayersActive; player++) {
            ThePlayerManager.GetActivePlayer(player).stats->Multiplayer = false;
        }
    }

    for (int i = 0; i < ThePlayerManager.PlayersActive; i++) {
        Player &player = ThePlayerManager.GetActivePlayer(i);
        player.stats->BaseScore = TheSongList.curSong->parts[player.Instrument]
                                      ->charts[player.Difficulty]
                                      .baseScore;
        if (i == 0) {
            ThePlayerManager.BandStats->BaseScore = player.stats->BaseScore;
        } else {
            ThePlayerManager.BandStats->BaseScore += player.stats->BaseScore;
        }
    }
}

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
#include "../settings/settings.h"
#include "timingvalues.h"

#include <raylib.h>

#include "settings/keybinds.h"

GameplayMenu::GameplayMenu() {
}

GameplayMenu::~GameplayMenu() {
}

/*
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
*/
void GameplayMenu::KeyboardInputCallback(int key, int scancode, int action, int mods) {
    /*Encore::EncoreLog(
        LOG_DEBUG,
        TextFormat(
            "Keyboard key %01i inputted on menu %s, action ",
            key,
            ToString(TheMenuManager.currentScreen),
            action
        )
    );*/
    Player &player = ThePlayerManager.GetActivePlayer(0);
    Encore::RhythmEngine::BaseEngine *engine = player.engine.get();
    Encore::RhythmEngine::BaseStats<5> *stats = engine->stats.get();
    // SettingsOld &settingsMain = SettingsOld::getInstance();

    if (action == 2)
        return;
    // if (player.Bot)
    //    return;
    // if the key action is NOT repeat (release is 0, press is 1)
    /*if (key == settingsMain.keybindPause && action == GLFW_PRESS) {
        // ManagePausedGame(inputHandler, player);
    } else if ((key == settingsMain.keybindOverdrive
                || key == settingsMain.keybindOverdriveAlt)) {
        // inputHandler.handleInputs(player, -1, action);
    } else */
    Encore::RhythmEngine::ControllerEvent event;
    if (action == GLFW_PRESS) {
        event.action = Encore::RhythmEngine::Action::PRESS;
    } else if (action == GLFW_RELEASE) {
        event.action = Encore::RhythmEngine::Action::RELEASE;
    }
    if (key == TheGameKeybinds.overdriveBinds.first || key == TheGameKeybinds.
        overdriveBinds.second) {
        event.channel = Encore::RhythmEngine::InputChannel::OVERDRIVE;
    } else if (player.ClassicMode) {
        if (key == TheGameKeybinds.strumBinds.first) {
            event.channel = Encore::RhythmEngine::InputChannel::STRUM_UP;
        } else if (key == TheGameKeybinds.strumBinds.second) {
            event.channel = Encore::RhythmEngine::InputChannel::STRUM_DOWN;
        }
    }
    int DiffMax = (player.Difficulty == 3 || player.ClassicMode) ? 5 : 4;
    for (int i = 0; i < DiffMax; i++) {
        if (key == TheGameKeybinds.keybinds5k[i] || key == TheGameKeybinds.keybinds5kalt[
            i]) {
            event.channel = Encore::RhythmEngine::IntIC(i);
        }
    }

    // Encore::EncoreLog(LOG_DEBUG, TextFormat("Keyboard key lane %01i",
    // lane));
    event.timestamp = TheSongTime.GetElapsedTime();
    if (event.channel != Encore::RhythmEngine::InputChannel::INVALID)
        engine->ProcessInput(event);
};

void GameplayMenu::ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event) {
    Player &player = ThePlayerManager.GetActivePlayer(0);
    Encore::RhythmEngine::BaseEngine *engine = player.engine.get();
    Encore::RhythmEngine::BaseStats<5> *stats = engine->stats.get();



    if (engine->allowTimestampedInputs) {
        if (engine->IsWithinPracticeSection(event.timestamp) || !engine->practice)
            engine->UpdateOnFrame(event.timestamp);
    }
    engine->ProcessInput(event);
    // Encore::EncoreLog(LOG_DEBUG, "Controller inputted.");
    /*
    Player &player = ThePlayerManager.GetActivePlayer(0);
    Encore::RhythmEngine::BaseEngine *engine = player.engine.get();
    Encore::RhythmEngine::BaseStats<5> *stats = engine->stats.get();
    // SettingsOld &settingsMain = SettingsOld::getInstance();

    // if (player.Bot)
    //    return;
    // if the key action is NOT repeat (release is 0, press is 1)
    /*if (key == settingsMain.keybindPause && action == GLFW_PRESS) {
        // ManagePausedGame(inputHandler, player);
    } else if ((key == settingsMain.keybindOverdrive
                || key == settingsMain.keybindOverdriveAlt)) {
        // inputHandler.handleInputs(player, -1, action);
    } else

    Encore::RhythmEngine::Action REaction;
    Encore::RhythmEngine::InputChannel Channel =
        Encore::RhythmEngine::InputChannel::INVALID;

    switch (player.padState.GetButtonState(state, GLFW_GAMEPAD_BUTTON_A)) {
    case GLFW_PRESS: {
        engine->ProcessInput(
            Encore::RhythmEngine::InputChannel::OVERDRIVE,
            Encore::RhythmEngine::Action::PRESS
        );
        return;
    }
    case GLFW_RELEASE: {
        engine->ProcessInput(
            Encore::RhythmEngine::InputChannel::OVERDRIVE,
            Encore::RhythmEngine::Action::RELEASE
        );
        return;
    }
    default:
        break;
    }
    int DiffMax = (player.Difficulty == 3 || player.ClassicMode) ? 5 : 4;
    for (int buttonInt = 0; buttonInt < player.padState.FacePadLayout.size();
         buttonInt++) {
        switch (player.padState.GetButtonState(
            state,
            player.padState.FacePadLayout.at(buttonInt)
        )) {
        case GLFW_PRESS: {
            engine->ProcessInput(
                Encore::RhythmEngine::IntIC(buttonInt),
                Encore::RhythmEngine::Action::PRESS
            );
            break;
        }
        case GLFW_RELEASE: {
            engine->ProcessInput(
                Encore::RhythmEngine::IntIC(buttonInt),
                Encore::RhythmEngine::Action::RELEASE
            );
            break;
        }
        default:
            break;
        }
    }

    // Encore::EncoreLog(LOG_DEBUG, TextFormat("Keyboard key lane %01i",
    // lane));

     * actually kinda important tbh this isnt supposed to be here
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
    */
};

void GameplayMenu::DrawScorebox(Units &u, Assets &assets, float scoreY) {
    Rectangle scoreboxSrc{
        0, 0, float(assets.Scorebox.width), float(assets.Scorebox.height)
    };
    float WidthOfScorebox = u.hinpct(0.28);
    // float scoreY = u.hpct(0.15f);
    float ScoreboxX = u.RightSide;
    float ScoreboxY = u.hpct(0.1425f);
    float HeightOfScorebox = WidthOfScorebox / 4;
    Rectangle scoreboxDraw{ ScoreboxX, ScoreboxY, WidthOfScorebox, HeightOfScorebox };
    DrawTexturePro(
        assets.Scorebox,
        scoreboxSrc,
        scoreboxDraw,
        { WidthOfScorebox, 0 },
        0,
        WHITE
    );

    GameMenu::mhDrawText(
        assets.redHatMono,
        GameMenu::scoreCommaFormatter(
            ThePlayerManager.GetActivePlayer(0).engine->stats->Score
        ),
        { u.RightSide - u.winpct(0.0145f), scoreY + u.hinpct(0.0025) },
        u.hinpct(0.05),
        Color{ 107, 161, 222, 255 },
        assets.sdfShader,
        RIGHT
    );
}

void GameplayMenu::DrawTimerbox(Units &u, Assets &assets, float scoreY) {
    Rectangle TimerboxSrc{
        0, 0, float(assets.Timerbox.width), float(assets.Timerbox.height)
    };
    float WidthOfTimerbox = u.hinpct(0.14);
    // float scoreY = u.hpct(0.15f);
    float TimerboxX = u.RightSide;
    float TimerboxY = u.hpct(0.1425f);
    float HeightOfTimerbox = WidthOfTimerbox / 4;
    Rectangle TimerboxDraw{ TimerboxX, TimerboxY, WidthOfTimerbox, HeightOfTimerbox };
    DrawTexturePro(
        assets.Timerbox,
        TimerboxSrc,
        TimerboxDraw,
        { WidthOfTimerbox, HeightOfTimerbox },
        0,
        WHITE
    );
    int played = TheSongTime.GetElapsedTime();
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
        "%i:%02i / %i:%02i",
        playedMinutes,
        playedSeconds,
        songMinutes,
        songSeconds
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
    Units &u,
    Assets &assets,
    float scorePos,
    float starY
) {
    // todo: redo for band
    auto &player = ThePlayerManager.GetActivePlayer(0);
    int inst = player.ClassicMode ? player.Instrument - 5 : player.Instrument;
    int diff = player.Difficulty;
    double starPercent = player.engine->stats->StarThresholdValue;
    int starsVal = player.engine->stats->Stars;
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
            firstStar ? 0 : STAR_THRESHOLDS[inst][i - 1],
            STAR_THRESHOLDS[inst][i],
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
            i == starsVal ? Color{ 192, 192, 192, 128 } : WHITE
        );
        EndScissorMode();
    }
    if (starPercent >= STAR_THRESHOLDS[inst][4]) {
        float starWH = u.hinpct(0.05);
        Rectangle emptyStarWH = {
            0, 0, (float)assets.goldStar.width, (float)assets.goldStar.height
        };
        float yMaskPos = Remap(
            starPercent,
            STAR_THRESHOLDS[inst][4],
            STAR_THRESHOLDS[inst][5],
            0,
            u.hinpct(0.05)
        );
        BeginScissorMode(
            scorePos - (starWH * 6),
            (starY + starWH) - yMaskPos,
            scorePos,
            yMaskPos
        );
        for (int i = 0; i < 5; i++) {
            float starX = scorePos - u.hinpct(0.26) + (i * u.hinpct(0.0525));
            Rectangle starRect = { starX, starY, starWH, starWH };
            DrawTexturePro(
                starPercent >= STAR_THRESHOLDS[inst][5]
                ? assets.goldStar
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
    int tick,
    int MinBrightness,
    int MaxBrightness,
    int QuarterNoteLength
) {
    float TickModulo = tick % QuarterNoteLength;
    return Remap(
        TickModulo / float(QuarterNoteLength),
        0,
        1.0f,
        MaxBrightness,
        MinBrightness
    );
}

bool songPlaying = false;

double GetNotePos(double noteTime, double songTime, float length, float end) {
    return ((noteTime - songTime) * (length * 2.5)) - end;
}

void GameplayMenu::Draw() {
    Units &u = Units::getInstance();
    Assets &assets = Assets::getInstance();
    // SettingsOld &settings = SettingsOld::getInstance();
    TheSongTime.UpdateTick();
    TheSongTime.UpdateOverdriveTick();
    //    OvershellRenderer osr;
    double curTime = GetTime();

    // IMAGE BACKGROUNDS??????
    ClearBackground(BLACK);
    unsigned char BackgroundColor = 0;
    // if (ThePlayerManager.BandStats->PlayersInOverdrive > 0) {
    // BackgroundColor = BeatToCharViaTickThing(TheSongTime.GetCurrentTick(), 0, 8, 960);
    //}
    if (!songPlaying) {
        TheSongTime.Reset();
        double songEnd = floor(TheAudioManager.GetMusicTimeLength());
        TheSongTime.Start(songEnd);
        TheAudioManager.BeginPlayback(TheAudioManager.loadedStreams[0].handle);
        songPlaying = true;
    }
    std::array<Color, 5> grybo = { GREEN, RED, YELLOW, BLUE, ORANGE };
    std::array<Color, 5> orybg = { ORANGE, RED, YELLOW, BLUE, GREEN };
    GameMenu::DrawAlbumArtBackground(TheSongList.curSong->albumArtBlur);
    DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), Color{ 0, 0, 0, 128 });
    DrawRectangle(
        0,
        0,
        GetRenderWidth(),
        GetRenderHeight(),
        Color{ 255, 255, 255, BackgroundColor }
    );

    if (TheSongTime.GetElapsedTime() > TheSongList.curSong->end - 1) {
        // TODO: endgame
        TheSongTime.Reset();
        TheAudioManager.unloadStreams();
        songPlaying = false;
        TheSongTime.Beatlines.erase(
            TheSongTime.Beatlines.begin(),
            TheSongTime.Beatlines.end()
        );
        TheSongTime.OverdriveTicks.erase(
            TheSongTime.OverdriveTicks.begin(),
            TheSongTime.OverdriveTicks.end()
        );
        TheSongTime.TimeSigChanges.erase(
            TheSongTime.TimeSigChanges.begin(),
            TheSongTime.TimeSigChanges.end()
        );
        TheSongTime.BPMChanges.erase(
            TheSongTime.BPMChanges.begin(),
            TheSongTime.BPMChanges.end()
        );
        TheSongTime.LastTick = 0;
        TheSongTime.CurrentTick = 0;
        TheSongTime.LastODTick = 0;
        TheSongTime.CurrentODTick = 0;
        TheSongTime.CurrentBPM = 0;
        TheSongTime.CurrentODTickItr = 0;
        TheSongTime.CurrentTimeSig = 0;
        TheSongTime.CurrentBeatline = 0;
        TheMenuManager.SwitchScreen(RESULTS);
        return;
    }

    int denom = TheSongTime.TimeSigChanges.at(TheSongTime.CurrentTimeSig).denom;
    int numer = TheSongTime.TimeSigChanges.at(TheSongTime.CurrentTimeSig).numer;
    int flashInterval = (numer * 480) / denom;

    for (int i = 0; i < ThePlayerManager.PlayersActive; i++) {
        Player &player = ThePlayerManager.GetActivePlayer(i);

        if (player.engine->IsWithinPracticeSection(TheSongTime.GetElapsedTime()) || !player.engine->practice)
            player.engine->UpdateOnFrame(TheSongTime.GetElapsedTime());
        player.engine->UpdateStats(player.Instrument, player.Difficulty);

        tracks.at(i)->Draw();
        int TopOfScreen = GetRenderHeight(); // width
        int FakeStrikeline = (TopOfScreen / 5) * 4;
        constexpr int NoteXWidth = 150;
        constexpr int NoteHeight = 25;

        int mospos =
            ((GetRenderWidth() + (ThePlayerManager.PlayersActive * NoteXWidth * 5))
                / (1 + ThePlayerManager.PlayersActive))
            - ((ThePlayerManager.PlayersActive * NoteXWidth * 5) / 2);
        int MiddleOfScreen = mospos + (mospos * i); // height
        int TrackLeft = MiddleOfScreen - (NoteXWidth / 2) - NoteXWidth - NoteXWidth;
        auto chart = player.engine->chart;
        int SidesWidth = 20;
        int RailWidth = SidesWidth / 2;

        GameMenu::mhDrawText(
            assets.JetBrainsMono,
            GameMenu::scoreCommaFormatter(player.engine->stats->Score),
            { MiddleOfScreen + (NoteXWidth * 3.0f),
              float(FakeStrikeline) - u.hinpct(0.05) },
            u.hinpct(0.05),
            WHITE,
            assets.sdfShader,
            0
        );
        int scoreWidth = MeasureTextEx(assets.JetBrainsMono,
                                       GameMenu::scoreCommaFormatter(
                                           player.engine->stats->Score).c_str(),
                                       u.hinpct(0.05),
                                       0).x;
        GameMenu::mhDrawText(
            assets.JetBrainsMono,
            std::to_string(player.engine->stats->StarThresholdValue) + "%*",
            { MiddleOfScreen + (NoteXWidth * 3.3f) + scoreWidth,
              float(FakeStrikeline) },
            u.hinpct(0.05),
            WHITE,
            assets.sdfShader,
            0
        );
        GameMenu::mhDrawText(
            assets.JetBrainsMono,
            "*" + std::to_string(player.engine->stats->Stars),
            { MiddleOfScreen + (NoteXWidth * 3.3f) + scoreWidth,
              float(FakeStrikeline) - u.hinpct(0.05) },
            u.hinpct(0.05),
            WHITE,
            assets.sdfShader,
            0
        );
        GameMenu::mhDrawText(
            assets.JetBrainsMono,
            std::to_string(player.engine->stats->Combo),
            { MiddleOfScreen + (NoteXWidth * 3.0f), float(FakeStrikeline) },
            u.hinpct(0.05),
            WHITE,
            assets.sdfShader,
            0
        );
        GameMenu::mhDrawText(
            assets.JetBrainsMono,
            std::to_string(player.engine->stats->multiplier()) + "x",
            { MiddleOfScreen + (NoteXWidth * 3.0f),
              float(FakeStrikeline) + u.hinpct(0.05) },
            u.hinpct(0.05),
            WHITE,
            assets.sdfShader,
            0
        );
        Color OverdriveText = player.engine->stats->overdrive.Active ? GOLD : WHITE;
        double ODNum = player.engine->stats->overdrive.Fill * 100.0;
        GameMenu::mhDrawText(
            assets.JetBrainsMono,
            TextFormat("%05.2f%%", ODNum),
            { MiddleOfScreen + (NoteXWidth * 3.0f),
              float(FakeStrikeline) + u.hinpct(0.1) },
            u.hinpct(0.05),
            OverdriveText,
            assets.sdfShader,
            0
        );
        std::string FASState =
            player.engine->Timers["FAS"].CanBeUsedUp(curTime) ? "FAS" : "";
        GameMenu::mhDrawText(
            assets.JetBrainsMono,
            FASState,
            { MiddleOfScreen + (NoteXWidth * 5.0f), float(FakeStrikeline) },
            u.hinpct(0.05),
            GREEN,
            assets.sdfShader,
            0
        );
        std::string SAHState =
            player.engine->Timers["SAH"].CanBeUsedUp(curTime) ? "SAH" : "";
        GameMenu::mhDrawText(
            assets.JetBrainsMono,
            SAHState,
            { MiddleOfScreen + (NoteXWidth * 5.0f),
              float(FakeStrikeline) + u.hinpct(0.5) },
            u.hinpct(0.05),
            GREEN,
            assets.sdfShader,
            0
        );
        {
            float TimeSinceHit = (TheSongTime.GetElapsedTime() - player.engine->stats->
                LastPerfectTime);
            if (TimeSinceHit <= 0.75f) {
                unsigned char HitAlpha = Remap(
                    getEasingFunction(EaseOutQuad)(TimeSinceHit / 0.75f),
                    0,
                    1.0,
                    255,
                    0
                );
                GameMenu::mhDrawText(
                    assets.JetBrainsMono,
                    TextFormat("Perfect (%.0fms)",
                               player.engine->stats->LastHitAccuracy * 1000),
                    { MiddleOfScreen + (NoteXWidth * 3.0f),
                      float(FakeStrikeline) - u.hinpct(0.1) },
                    u.hinpct(0.05),
                    { GOLD.r, GOLD.g, GOLD.b, HitAlpha },
                    assets.sdfShader,
                    0
                );
            }
        }
        // DrawRectangle(TrackLeft, 0, NoteXWidth * 5, GetRenderHeight(), { 0, 0, 0, 128 });
        // if (!TheSongTime.Beatlines.empty()) {
        //     for (auto &beatline : BeatlinePool) {
        //         int ScrollPos = -1
        //             * GetNotePos(
        //                 beatline.time,
        //                 TheSongTime.GetElapsedTime(),
        //                 FakeStrikeline / 2,
        //                 FakeStrikeline
        //             );
        //         int Height = 0;
        //         Color beatlineColor = WHITE;
        //         switch (beatline.type) {
        //         case Major: {
        //             beatlineColor = GRAY;
        //             Height = 10;
        //             break;
        //         }
        //         case Minor: {
        //             beatlineColor = DARKGRAY;
        //             Height = 10;
        //             break;
        //         }
        //         case Measure: {
        //             beatlineColor = WHITE;
        //             Height = 10;
        //             break;
        //         }
        //         }
        //         DrawRectangle(
        //             TrackLeft,
        //             (ScrollPos) - Height,
        //             NoteXWidth * 5,
        //             Height,
        //             { beatlineColor.r, beatlineColor.g, beatlineColor.b, 128 }
        //         );
        //     }
        // }

        /*  0 is the top of the screen, and is earlier/negative in time
         *
         */

        // for (int j = 0; j <= 5; j++) {
        //     Color DividerColors = { WHITE.r, WHITE.g, WHITE.b, 128 };
        //     int DividerWidth = 2;
        //     if (player.engine->stats->overdrive.Active) {
        //         DividerColors = { GOLD.r, GOLD.g, GOLD.b, 128 };
        //         DividerWidth = 4;
        //     }
        //     if (j == 0 || j == 5) {
        //         int pos = (TrackLeft + (NoteXWidth * j)) - (RailWidth);
        //         if (j == 5)
        //             pos = (TrackLeft + (NoteXWidth * j));
        //         DrawRectangle(pos, 0, RailWidth, GetRenderHeight(), DARKGRAY);
        //     }
        //     DrawRectangle(
        //         (TrackLeft + (NoteXWidth * j)) - (DividerWidth / 2),
        //         0,
        //         DividerWidth,
        //         GetRenderHeight(),
        //         DividerColors
        //     ); // dividers
        // }
        //
        // if (!chart->solos.empty()) {
        //     solo &currentSolo = chart->solos.front();
        //     int ScrollPos = -1
        //         * GetNotePos(
        //             currentSolo.StartSec,
        //             TheSongTime.GetElapsedTime(),
        //             FakeStrikeline / 2,
        //             FakeStrikeline
        //         );
        //     int NoteLength = -1
        //         * GetNotePos(
        //             currentSolo.StartSec + currentSolo.EndSec,
        //             TheSongTime.GetElapsedTime(),
        //             FakeStrikeline / 2,
        //             FakeStrikeline
        //         );
        //     int ScrollEndPos = ScrollPos - NoteLength;
        //     int topPos =
        //         MiddleOfScreen - (NoteXWidth / 2) - NoteXWidth - NoteXWidth - SidesWidth;
        //     int bottomPos = MiddleOfScreen + (NoteXWidth / 2) + NoteXWidth + NoteXWidth;
        //     Color soloColor = { BLUE.r, BLUE.g, BLUE.b, 64 };
        //     DrawRectangle(
        //         TrackLeft,
        //         NoteLength,
        //         (NoteXWidth * 5),
        //         ScrollEndPos,
        //         soloColor
        //     );
        //     if (currentSolo.StartSec < TheSongTime.GetElapsedTime()) {
        //         // this should be some sort of track state here. like "solo active"
        //         // because you know. this is logic!
        //         GameMenu::mhDrawText(
        //             assets.rubik,
        //             std::to_string(
        //                 int((float(currentSolo.NotesHit) / float(currentSolo.NoteCount))
        //                     * 100.0f)
        //             ) + "%",
        //             { float(MiddleOfScreen), float(TopOfScreen / 5) },
        //             u.hinpct(0.075),
        //             WHITE,
        //             assets.sdfShader,
        //             1
        //         );
        //         GameMenu::mhDrawText(
        //             assets.rubik,
        //             std::to_string(currentSolo.NotesHit) + "/"
        //             + std::to_string(currentSolo.NoteCount),
        //             { float(MiddleOfScreen), float(TopOfScreen / 5) + u.hinpct(0.075) },
        //             u.hinpct(0.025),
        //             WHITE,
        //             assets.sdfShader,
        //             1
        //         );
        //     }
        //     if (currentSolo.StartSec + currentSolo.EndSec < TheSongTime.GetElapsedTime())
        //         chart->solos.erase(chart.get()->solos.begin());
        // }
        // if (!chart->overdrive.empty()) {
        //     odPhrase &currentSolo = player.engine.get()->chart.get()->overdrive.front();
        //     auto asd = player.engine->chart->overdrive.begin();
        //     int ScrollPos = -1
        //         * GetNotePos(
        //             currentSolo.StartSec,
        //             TheSongTime.GetElapsedTime(),
        //             FakeStrikeline / 2,
        //             FakeStrikeline
        //         );
        //     int NoteLength = -1
        //         * GetNotePos(
        //             currentSolo.StartSec + currentSolo.EndSec,
        //             TheSongTime.GetElapsedTime(),
        //             FakeStrikeline / 2,
        //             FakeStrikeline
        //         );
        //     int ScrollEndPos = ScrollPos - NoteLength;
        //     int topPos = MiddleOfScreen - (NoteXWidth / 2) - NoteXWidth - NoteXWidth
        //         - (SidesWidth / 2);
        //     int bottomPos = MiddleOfScreen + (NoteXWidth / 2) + NoteXWidth + NoteXWidth;
        //     DrawRectangle(topPos, NoteLength, SidesWidth / 2, ScrollEndPos, GOLD);
        //     DrawRectangle(bottomPos, NoteLength, SidesWidth / 2, ScrollEndPos, GOLD);
        // }

        // if (player.engine->stats->Type == Encore::RhythmEngine::Pad
        //     || player.engine->stats->Type == Encore::RhythmEngine::Drums) {
        //     size_t NotePoolPerLane = NOTE_POOL_SIZE / chart->Lanes.size();
        //     for (int Lane = 0; Lane < chart->Lanes.size(); ++Lane) {
        //         auto &chartLane = chart->at(Lane);
        //
        //         if (chartLane.empty()) {
        //             continue;
        //         }
        //         int NotePoolStart = std::distance(
        //             chart->at(Lane).begin(),
        //             chart->CurrentNoteIterators.at(Lane)
        //         );
        //         int NotePoolEnd = NotePoolPerLane
        //             + std::distance(chart->at(Lane).begin(),
        //                             chart->CurrentNoteIterators.at(Lane));
        //         NotePoolEnd = chart->at(Lane).size() > NotePoolEnd
        //             ? NotePoolEnd
        //             : chart->at(Lane).size();
        //         int NotePoolSize = NotePoolEnd - NotePoolStart;
        //         // size_t NotePoolSize = chartLane.size() > NotePoolPerLane
        //         //     ? NotePoolPerLane
        //         //     : chartLane.size();
        //         //  because i have to do bounds checks myself
        //         //  just might not use span for this then LMAO
        //         //  std::span<Encore::RhythmEngine::NoteVector<Encore::RhythmEngine::EncNote>>
        //         //  NotePool { chartLane.begin(), chartLane.begin() + NotePoolSize };
        //
        //         // for (auto &note : NotePool) {
        //         for (int curNote = NotePoolStart; curNote < NotePoolEnd; curNote++) {
        //             auto &note = chartLane.at(curNote);
        //
        //             int ScrollPos = -1
        //                 * GetNotePos(
        //                     note.StartSeconds,
        //                     TheSongTime.GetElapsedTime(),
        //                     FakeStrikeline / 2,
        //                     FakeStrikeline
        //                 );
        //
        //             int NoteLength = -1
        //                 * GetNotePos(
        //                     note.StartSeconds + note.LengthSeconds,
        //                     TheSongTime.GetElapsedTime(),
        //                     FakeStrikeline / 2,
        //                     FakeStrikeline
        //                 );
        //
        //             int ScrollEndPos = ScrollPos - NoteLength;
        //             bool sust = false;
        //             int sustLength = ScrollPos - NoteHeight;
        //             if (note.LengthTicks == 0) {
        //                 NoteLength = ScrollPos - NoteHeight;
        //                 ScrollEndPos = NoteHeight;
        //             } else {
        //                 sust = true;
        //             }
        //             int ScrollStartPos = ScrollPos;
        //
        //             uint8_t x = note.Lane;
        //             int NoteWidth = NoteXWidth;
        //             DrawRectangle(0, NoteXWidth, NoteXWidth, NoteXWidth * 2, GREEN);
        //
        //             int pos = TrackLeft + (NoteXWidth * Lane);
        //             Color color;
        //             if (player.engine->stats->Type == Encore::RhythmEngine::Drums) {
        //                 color = orybg[Lane];
        //                 if (Lane == 0) {
        //                     NoteWidth *= 5;
        //                     NoteLength += NoteHeight / 2;
        //                     ScrollEndPos = NoteHeight / 2;
        //                 }
        //             } else {
        //                 color = grybo[Lane];
        //             }
        //
        //             if (note.NotePassed)
        //                 color = MAROON;
        //             if (sust) {
        //                 DrawRectangle(pos + (NoteXWidth / 4),
        //                               NoteLength,
        //                               NoteXWidth / 2,
        //                               ScrollEndPos,
        //                               color);
        //             }
        //             DrawRectangle(pos, sustLength, NoteXWidth, NoteHeight, color);
        //             if (note.NoteType == 1) {
        //                 DrawRectangle(
        //                     pos + 5,
        //                     sustLength + 5,
        //                     NoteXWidth - 10,
        //                     NoteHeight - 10,
        //                     WHITE
        //                 );
        //             }
        //         }
        //         if (chart->HeldNotePointers.at(Lane)) {
        //             auto &note = chart->HeldNotePointers.at(Lane);
        //             int ScrollPos = -1
        //                 * GetNotePos(
        //                     note->StartSeconds,
        //                     TheSongTime.GetElapsedTime(),
        //                     FakeStrikeline / 2,
        //                     FakeStrikeline
        //                 );
        //
        //             int NoteLength = -1
        //                 * GetNotePos(
        //                     note->StartSeconds + note->LengthSeconds,
        //                     TheSongTime.GetElapsedTime(),
        //                     FakeStrikeline / 2,
        //                     FakeStrikeline
        //                 );
        //             int ScrollEndPos = FakeStrikeline - NoteLength;
        //             int ScrollStartPos = ScrollPos;
        //             int NoteWidth = NoteXWidth;
        //             Color color;
        //             int pos = TrackLeft + (NoteXWidth * Lane);
        //             if (player.engine->stats->Type == Encore::RhythmEngine::Drums) {
        //                 color = orybg[Lane];
        //                 if (Lane == 0) {
        //                     NoteWidth *= 5;
        //                     NoteLength += NoteHeight / 2;
        //                     ScrollEndPos = NoteHeight / 2;
        //                 }
        //             } else {
        //                 color = grybo[Lane];
        //             }
        //
        //             if (note->NotePassed)
        //                 color = MAROON;
        //             DrawRectangle(pos + (NoteXWidth / 4),
        //                           NoteLength,
        //                           NoteXWidth / 2,
        //                           ScrollEndPos,
        //                           color);
        //             //if (note->NoteType == 1) {
        //             //    DrawRectangle(
        //             //        pos + 5,
        //             //        NoteLength + 5,
        //             //        NoteWidth - 10,
        //             //        ScrollEndPos - 10,
        //             //        WHITE
        //             //    );
        //             //}
        //         }
        //     }
        // } else {
        //     if (chart->at(0).empty()) {
        //         continue;
        //     }
        //     // if the size of the chart is bigger than the selected "range"
        //     int NotePoolStart =
        //         std::distance(chart->at(0).begin(), chart->CurrentNoteIterators.at(0));
        //     int NotePoolEnd = NOTE_POOL_SIZE
        //         + std::distance(chart->at(0).begin(), chart->CurrentNoteIterators.at(0));
        //     NotePoolEnd =
        //         chart->at(0).size() > NotePoolEnd ? NotePoolEnd : chart->at(0).size();
        //     int NotePoolSize = NotePoolEnd - NotePoolStart;
        //
        //     if (!chart->rolls.empty()) {
        //         for (auto &roll : chart->rolls) {
        //             int ScrollPos = -1
        //                 * GetNotePos(
        //                     roll.StartSec,
        //                     TheSongTime.GetElapsedTime(),
        //                     FakeStrikeline / 2,
        //                     FakeStrikeline
        //                 );
        //
        //             int NoteLength = -1
        //                 * GetNotePos(
        //                     roll.StartSec + roll.EndSec,
        //                     TheSongTime.GetElapsedTime(),
        //                     FakeStrikeline / 2,
        //                     FakeStrikeline
        //                 );
        //
        //             int ScrollEndPos = ScrollPos - NoteLength;
        //             uint8_t x = roll.lane;
        //             while (x) {
        //                 uint8_t y = x & ~(x - 1);
        //                 int pos = TrackLeft;
        //                 Color color = GREEN;
        //                 if (y == Encore::RhythmEngine::PlasticFrets[1]) {
        //                     pos += NoteXWidth;
        //                     color = RED;
        //                 } else if (y == Encore::RhythmEngine::PlasticFrets[2]) {
        //                     pos += NoteXWidth * 2;
        //                     color = YELLOW;
        //                 } else if (y == Encore::RhythmEngine::PlasticFrets[3]) {
        //                     pos += NoteXWidth * 3;
        //                     color = BLUE;
        //                 } else if (y == Encore::RhythmEngine::PlasticFrets[4]) {
        //                     pos += NoteXWidth * 4;
        //                     color = ORANGE;
        //                 }
        //                 color = ColorBrightness(color, -0.75);
        //                 DrawRectangle(pos, NoteLength, NoteXWidth, ScrollEndPos, color);
        //                 x &= (x - 1);
        //             }
        //         }
        //     }
        //     if (!chart->trills.empty()) {
        //         for (auto &trill : chart->trills) {
        //             int ScrollPos = -1
        //                 * GetNotePos(
        //                     trill.StartSec,
        //                     TheSongTime.GetElapsedTime(),
        //                     FakeStrikeline / 2,
        //                     FakeStrikeline
        //                 );
        //
        //             int NoteLength = -1
        //                 * GetNotePos(
        //                     trill.StartSec + trill.EndSec,
        //                     TheSongTime.GetElapsedTime(),
        //                     FakeStrikeline / 2,
        //                     FakeStrikeline
        //                 );
        //
        //             int ScrollEndPos = ScrollPos - NoteLength;
        //             int pos = TrackLeft;
        //             Color color = GREEN;
        //             if (trill.lane1 == Encore::RhythmEngine::PlasticFrets[1]) {
        //                 pos += NoteXWidth;
        //                 color = RED;
        //             } else if (trill.lane1 == Encore::RhythmEngine::PlasticFrets[2]) {
        //                 pos += NoteXWidth * 2;
        //                 color = YELLOW;
        //             } else if (trill.lane1 == Encore::RhythmEngine::PlasticFrets[3]) {
        //                 pos += NoteXWidth * 3;
        //                 color = BLUE;
        //             } else if (trill.lane1 == Encore::RhythmEngine::PlasticFrets[4]) {
        //                 pos += NoteXWidth * 4;
        //                 color = ORANGE;
        //             }
        //             color = ColorBrightness(color, -0.5);
        //             DrawRectangle(pos, NoteLength, NoteXWidth, ScrollEndPos, color);
        //             color = GREEN;
        //             pos = TrackLeft;
        //             if (trill.lane2 == Encore::RhythmEngine::PlasticFrets[1]) {
        //                 pos += NoteXWidth;
        //                 color = RED;
        //             } else if (trill.lane2 == Encore::RhythmEngine::PlasticFrets[2]) {
        //                 pos += NoteXWidth * 2;
        //                 color = YELLOW;
        //             } else if (trill.lane2 == Encore::RhythmEngine::PlasticFrets[3]) {
        //                 pos += NoteXWidth * 3;
        //                 color = BLUE;
        //             } else if (trill.lane2 == Encore::RhythmEngine::PlasticFrets[4]) {
        //                 pos += NoteXWidth * 4;
        //                 color = ORANGE;
        //             }
        //             color = ColorBrightness(color, -0.75);
        //             DrawRectangle(pos, NoteLength, NoteXWidth, ScrollEndPos, color);
        //         }
        //     }
        //     // because i have to do bounds checks myself
        //     // std::span NotePool { chart->CurrentNoteIterators.at(0),
        //     // chart->CurrentNoteIterators.at(0) + NotePoolSize }; for (auto &note :
        //     // NotePool) {
        //
        //     for (int curNote = NotePoolStart; curNote < NotePoolEnd; curNote++) {
        //         auto &note = chart->at(0).at(curNote);
        //         // basic miss check, only delays for showing misses
        //         int ScrollPos = -1
        //             * GetNotePos(
        //                 note.StartSeconds,
        //                 TheSongTime.GetElapsedTime(),
        //                 FakeStrikeline / 2,
        //                 FakeStrikeline
        //             );
        //
        //         int NoteLength = -1
        //             * GetNotePos(
        //                 note.StartSeconds + note.LengthSeconds,
        //                 TheSongTime.GetElapsedTime(),
        //                 FakeStrikeline / 2,
        //                 FakeStrikeline
        //             );
        //
        //         int ScrollEndPos = ScrollPos - NoteLength;
        //         bool sust = false;
        //         int sustLength = ScrollPos - NoteHeight;
        //         if (note.LengthTicks == 0) {
        //             NoteLength = ScrollPos - NoteHeight;
        //             ScrollEndPos = NoteHeight;
        //         } else {
        //             sust = true;
        //         }
        //         int ScrollStartPos = ScrollPos;
        //
        //         uint8_t x = note.Lane;
        //
        //         DrawRectangle(0, NoteXWidth, NoteXWidth, NoteXWidth * 2, GREEN);
        //         if (x == 0) {
        //             int pos = TrackLeft;
        //             int width = NoteXWidth;
        //             Color color = PURPLE;
        //             width = NoteXWidth * 4;
        //             if (sust) {
        //                 DrawRectangle(pos + (NoteXWidth / 4),
        //                               NoteLength,
        //                               width / 2,
        //                               ScrollEndPos,
        //                               color);
        //             }
        //             DrawRectangle(pos, sustLength, width, NoteHeight, color);
        //             if (note.NoteType == 1) {
        //                 DrawRectangle(
        //                     pos + 5,
        //                     sustLength + 5,
        //                     width - 10,
        //                     NoteHeight - 10,
        //                     WHITE
        //                 );
        //             }
        //         }
        //         while (x) {
        //             uint8_t y = x & ~(x - 1);
        //             int pos = TrackLeft;
        //             int width = NoteXWidth;
        //             Color color = GREEN;
        //             if (x == 0) {
        //                 color = PURPLE;
        //                 width = NoteXWidth * 4;
        //             } else if (y == Encore::RhythmEngine::PlasticFrets[1]) {
        //                 pos += NoteXWidth;
        //                 color = RED;
        //             } else if (y == Encore::RhythmEngine::PlasticFrets[2]) {
        //                 pos += NoteXWidth * 2;
        //                 color = YELLOW;
        //             } else if (y == Encore::RhythmEngine::PlasticFrets[3]) {
        //                 pos += NoteXWidth * 3;
        //                 color = BLUE;
        //             } else if (y == Encore::RhythmEngine::PlasticFrets[4]) {
        //                 pos += NoteXWidth * 4;
        //                 color = ORANGE;
        //             }
        //             if (note.NotePassed)
        //                 color = MAROON;
        //             // DrawRectangle(pos, NoteLength, NoteXWidth, ScrollEndPos, color);
        //             if (sust) {
        //                 DrawRectangle(pos + (NoteXWidth / 4),
        //                               NoteLength,
        //                               width / 2,
        //                               ScrollEndPos,
        //                               color);
        //             }
        //             DrawRectangle(pos, sustLength, width, NoteHeight, color);
        //             if (note.NoteType == 1) {
        //                 DrawRectangle(
        //                     pos + 5,
        //                     sustLength + 5,
        //                     width - 10,
        //                     NoteHeight - 10,
        //                     WHITE
        //                 );
        //             }
        //             x &= (x - 1);
        //         }
        //     }
        //     if (chart->HeldNotePointers.at(0)) {
        //         auto &note = chart->HeldNotePointers.at(0);
        //         int ScrollPos = -1
        //             * GetNotePos(
        //                 note->StartSeconds,
        //                 TheSongTime.GetElapsedTime(),
        //                 FakeStrikeline / 2,
        //                 FakeStrikeline
        //             );
        //
        //         int NoteLength = -1
        //             * GetNotePos(
        //                 note->StartSeconds + note->LengthSeconds,
        //                 TheSongTime.GetElapsedTime(),
        //                 FakeStrikeline / 2,
        //                 FakeStrikeline
        //             );
        //         int ScrollEndPos = FakeStrikeline - NoteLength;
        //         int ScrollStartPos = ScrollPos;
        //
        //         uint8_t x = note->Lane;
        //         while (x) {
        //             uint8_t y = x & ~(x - 1);
        //             int pos = TrackLeft;
        //             Color color = GREEN;
        //             if (y == Encore::RhythmEngine::PlasticFrets[1]) {
        //                 pos += NoteXWidth;
        //                 color = RED;
        //             } else if (y == Encore::RhythmEngine::PlasticFrets[2]) {
        //                 pos += NoteXWidth * 2;
        //                 color = YELLOW;
        //             } else if (y == Encore::RhythmEngine::PlasticFrets[3]) {
        //                 pos += NoteXWidth * 3;
        //                 color = BLUE;
        //             } else if (y == Encore::RhythmEngine::PlasticFrets[4]) {
        //                 pos += NoteXWidth * 4;
        //                 color = ORANGE;
        //             }
        //             if (note->NotePassed)
        //                 color = MAROON;
        //             DrawRectangle(pos + (NoteXWidth / 4),
        //                           NoteLength,
        //                           NoteXWidth / 2,
        //                           ScrollEndPos,
        //                           color);
        //             //if (note->NoteType == 1) {
        //             //    DrawRectangle(
        //             //        pos + 5,
        //             //        NoteLength + 5,
        //             //        NoteXWidth - 10,
        //             //        ScrollEndPos - 10,
        //             //        WHITE
        //             //    );
        //             //}
        //             x &= (x - 1);
        //         }
        //     }
        // }
        bool maxmult = false; ///player.engine->stats->SixMultiplier
        //? player.engine->stats->multNoOD() >= 5
        //: player.engine->stats->multNoOD() == 4;
        if (maxmult) {
            unsigned char streakFlash = BeatToCharViaTickThing(
                TheSongTime.GetCurrentTick(),
                0,
                64,
                flashInterval
            );
            Color StreakColor = { 255, 255, 255, streakFlash };
            if (player.engine->stats->overdrive.Active) {
                StreakColor.r = GOLD.r;
                StreakColor.g = GOLD.g;
                StreakColor.b = GOLD.b;
            }

            int Height = TopOfScreen / 5;
            DrawRectangleGradientV(
                TrackLeft,
                FakeStrikeline - Height,
                NoteXWidth * 5,
                Height,
                { 0 },
                StreakColor
            );
            DrawRectangle(TrackLeft, FakeStrikeline, NoteXWidth * 5, Height, StreakColor);
        }
        // overdrive bar ------------
        float ODBarLength = (TopOfScreen / 5) * 2;
        float BarWidth = NoteXWidth / 2;
        double ODBarWidth =
            Remap(player.engine->stats->overdrive.Fill, 1.0, 0, ODBarLength, 0);

        unsigned char streakFlash =
            BeatToCharViaTickThing(TheSongTime.GetCurrentTick(), 0, 255, flashInterval);
        float Percentage = float(streakFlash) / 255.0f;
        Color OverdriveBarColor = ColorBrightness(GOLD, Percentage);

        float ODBarFillPosition = GetRenderHeight() - ODBarWidth;
        // ----------------

        // DrawRectangle(
        //     MiddleOfScreen - (NoteXWidth * 3),
        //     (FakeStrikeline) - 5,
        //     NoteXWidth * 6,
        //     10,
        //     BLACK
        // );

        // for (int g = 0; g < player.engine->stats->HeldFrets.size(); g++) {
        //     if (player.engine->stats->HeldFrets[g]) {
        //         Color background = grybo[g];
        //         DrawRectangle(
        //             TrackLeft + (NoteXWidth * g),
        //             (FakeStrikeline) - (NoteXWidth / 2),
        //             NoteXWidth,
        //             NoteXWidth,
        //             background
        //         );
        //     }
        // }
        //
        // if (player.engine->stats->strumState
        //     == Encore::RhythmEngine::StrumState::UpStrum) {
        //     DrawRectangle(
        //         MiddleOfScreen - NoteXWidth - NoteXWidth - NoteXWidth,
        //         (FakeStrikeline) - 10,
        //         NoteXWidth * 6,
        //         10,
        //         WHITE
        //     );
        // }
        // if (player.engine.get()->stats.get()->strumState
        //     == Encore::RhythmEngine::StrumState::DownStrum) {
        //     DrawRectangle(
        //         MiddleOfScreen - NoteXWidth - NoteXWidth - NoteXWidth,
        //         (FakeStrikeline),
        //         NoteXWidth * 6,
        //         10,
        //         WHITE
        //     );
        // }

        if (player.engine.get()->stats.get()->AudioMuted) {
            int InstrumentNum =
                player.ClassicMode ? player.Instrument - 5 : player.Instrument;
            if (TheAudioManager.GetAudioStreamByInstrument(InstrumentNum) == nullptr)
                break;
            TheAudioManager.GetAudioStreamByInstrument(InstrumentNum)->volume =
                TheGameSettings.avMainVolume * TheGameSettings.avMuteVolume;
        } else {
            int InstrumentNum =
                player.ClassicMode ? player.Instrument - 5 : player.Instrument;
            if (TheAudioManager.GetAudioStreamByInstrument(InstrumentNum) == nullptr)
                break;
            TheAudioManager.GetAudioStreamByInstrument(InstrumentNum)->volume =
                TheGameSettings.avMainVolume * TheGameSettings.avActiveInstrumentVolume;
        }
    }
    TheAudioManager.UpdateAudioStreamVolumes();
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

    // Encore::RhythmEngine::TheRhythmManager.UpdateTime();
    // Encore::RhythmEngine::TheRhythmManager.StartFrameTick();
    // for (int pnum = 0; pnum < ThePlayerManager.PlayersActive; pnum++) {
    //     Player &curPlayer = ThePlayerManager.GetActivePlayer(pnum);
    //     curPlayer.rhythmEngine->UpdateEngineOnFrame();
    // }
    // Encore::RhythmEngine::TheRhythmManager.EndFrameTick();

    for (int pnum = 0; pnum < ThePlayerManager.PlayersActive; pnum++) {
        Player &curPlayer = ThePlayerManager.GetActivePlayer(pnum);
        /*
        TheGameRenderer.cameraSel =
            CameraSelectionPerPlayer[ThePlayerManager.PlayersActive - 1][pnum];
        int pos = CameraPosPerPlayer[ThePlayerManager.PlayersActive - 1][pnum];
        if (pos == 0)
            TheGameRenderer.renderPos =
                CameraPosPerPlayer[ThePlayerManager.PlayersActive - 1][pnum];
        else
            TheGameRenderer.renderPos = GetRenderWidth()
                / CameraPosPerPlayer[ThePlayerManager.PlayersActive - 1][pnum];
        //for (int n = 0; n < TheSongList.curSong->parts[curPlayer.Instrument]
        //                        ->charts[curPlayer.Difficulty]
        //                        .notes.size();
        //     ++n) {
        //
        //}
        TheGameRenderer.CurrentTick =
    TheSongList.curSong->bpms[curPlayer.stats->curBPM].tick
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
                        (TheGameRenderer.CurrentTick - curPlayer.stats->LastTick) *
    OverdriveDrainPerTick;
                    /*
                    player.stats->overdriveFill = player.stats->overdriveActiveFill
                        - (float)((curSongTime - player.stats->overdriveActiveTime)
                                  / (1920 / song.bpms[player.stats->curBPM].bpm));

                    if (curPlayer.stats->overdriveFill <= 0) {
                        curPlayer.stats->overdriveActivateTime =
    TheSongTime.GetSongTime(); curPlayer.stats->Overdrive = false;
                        curPlayer.stats->overdriveFill = 0;
                        curPlayer.stats->overdriveActiveFill = 0;
                        curPlayer.stats->overdriveActiveTime = 0.0;
                        ThePlayerManager.BandStats->PlayersInOverdrive -= 1;
                        ThePlayerManager.BandStats->Overdrive = false;
                    }
                }

                for (int i = curPlayer.stats->curBPM; i <
    TheSongList.curSong->bpms.size(); i++) { if (TheSongTime.GetSongTime() >
    TheSongList.curSong->bpms[i].time && i < TheSongList.curSong->bpms.size() - 1)
                        curPlayer.stats->curBPM++;
                }

                TheGameRenderer.CheckPlasticNotes(
                    curPlayer,
                    curChart,
                    TheSongTime.GetSongTime(),
                    curPlayer.stats->curNoteInt
                );
                // curChart.overdrive.CheckEvents(curPlayer.stats->curODPhrase,
    TheSongTime.GetSongTime());
                // curChart.solos.CheckEvents(curPlayer.stats->curSolo,
    TheSongTime.GetSongTime());
                //  curChart.fills.CheckEvents(curPlayer.stats->curFill,
    TheSongTime.GetSongTime());
                // curChart.sections.CheckEvents(curPlayer.stats->curFill,
    TheSongTime.GetSongTime());

                if (curNote.len > 0) {
                    for (auto cLane : curNote.pLanes) {
                        int lane = cLane.lane;
                        if (curNote.held) {
                            cLane.heldTime = TheSongTime.GetSongTime() - curNote.time;
                            if (cLane.heldTime >= cLane.length) {
                                cLane.heldTime = cLane.length;
                            }
                            TheGameRenderer.CalculateSustainScore(curPlayer.stats);
                            if (!((curPlayer.stats->PressedMask >> lane) & 1) &&
    !curPlayer.Bot) { curNote.held = false;
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
              GetRenderHeight() - u.hinpct(0.04) },
            fontSize,
            0,
            headerUsernameColor
        );

    }
    */
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
    std::string SongArtistString =
        TheSongList.curSong->artist + ", " + TheSongList.curSong->releaseYear;
    float SongArtistWidth =
        MeasureTextEx(
            assets.rubikBoldItalic,
            SongArtistString.c_str(),
            u.hinpct(SmallHeader),
            0
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
            Color{ 0, 0, 0, 128 },
            Color{ 0, 0, 0, 0 }
        );
        DrawTextEx(
            assets.rubikBoldItalic,
            TheSongList.curSong->title.c_str(),
            { SongNamePosition, u.hpct(0.2f) },
            u.hinpct(MediumHeader),
            0,
            Color{ 255, 255, 255, SongNameAlpha }
        );
        DrawTextEx(
            assets.rubikItalic,
            SongArtistString.c_str(),
            { SongArtistPosition, u.hpct(0.2f + MediumHeader) },
            u.hinpct(SmallHeader),
            0,
            Color{ 200, 200, 200, SongArtistAlpha }
        );
        DrawTextEx(
            assets.rubikItalic,
            TheSongList.curSong->charters[0].c_str(),
            { SongExtrasPosition, u.hpct(0.2f + MediumHeader + SmallHeader) },
            u.hinpct(SmallHeader),
            0,
            Color{ 200, 200, 200, SongExtrasAlpha }
        );
    }

    // int songLength;
    // if (TheSongList.curSong->end == 0)
    //     songLength = static_cast<int>(TheAudioManager.GetMusicTimeLength());
    // else
    //     songLength = static_cast<int>(TheSongList.curSong->end);

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

    /*if (ThePlayerManager.BandStats->Paused) {
        DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), Color { 0, 0, 0, 80 });
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
                ThePlayerManager.GetActivePlayer(playerNum).stats->CurPlayingChart =
    TheSongList.curSong->parts[ThePlayerManager.GetActivePlayer(playerNum).Instrument]->charts[ThePlayerManager.GetActivePlayer(playerNum).Difficulty];
            }

            ThePlayerManager.BandStats->ResetBandGameplayStats();
            ThePlayerManager.BandStats->Paused = false;
        }
        if (GuiButton(QuitBox, "Back to Music Library")) {
            // notes =
            // TheSongList.curSong->parts[instrument]->charts[diff].notes.size();
            // notes = TheSongList.curSong->parts[instrument]->charts[diff];

            TheSongList.curSong->LoadAlbumArt();
            // ThePlayerManager.BandStats->ResetBandGameplayStats();
            // TheGameRenderer.midiLoaded = false;
            TheSongTime.Reset();

            TheAudioManager.unloadStreams();
            // TheGameRenderer.highwayInAnimation = false;
            // TheGameRenderer.highwayInEndAnim = false;
            // TheGameRenderer.songPlaying = false;
            /*
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
                assets.rubikBoldItalic, TheSongList.curSong->title.c_str(), SongFontSize,
    0
            )
                .y;
        float TitleWidth =
            MeasureTextEx(
                assets.rubikBoldItalic, TheSongList.curSong->title.c_str(), SongFontSize,
    0
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
        /*
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
    // }
    */
    GameMenu::DrawFPS(u.LeftSide, u.hpct(0.0025f) + u.hinpct(0.025f));
    GameMenu::DrawVersion();
    /*
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
    //				{5, GetRenderHeight() - u.hinpct(0.05f)}, u.hinpct(0.04), 0,
    //				GOLD);
    // if (TheGameRenderer.bot)
    //	DrawTextEx(assets.rubikBold, "BOT",
    //				{5, GetRenderHeight() - u.hinpct(0.05f)}, u.hinpct(0.04), 0,
    //				SKYBLUE);
    // if (!TheGameRenderer.bot)
    /*
    GuiProgressBar(
        Rectangle { 0,
                    (float)GetRenderHeight() - u.hinpct(0.005f),
                    (float)GetRenderWidth(),
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
                                    {5, GetRenderHeight() - u.hinpct(0.1f)},
    u.hinpct(0.04), 0, GOLD);


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
                        ||
    ThePlayerManager.GetActivePlayer(0).stats->HeldFretsAlt[fretBox] ? fretColor : GRAY
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
    TheAudioManager.loadStreams(TheSongList.curSong->stemsPath);
    TheSongTime.SetOffset(TheGameSettings.AudioOffset / 1000.0);

    // i dont like the game stuttering when you active or get a streak
    float widthPerPlayer = 2.0f / ThePlayerManager.PlayersActive;

    for (int i = 0; i < ThePlayerManager.PlayersActive; i++) {
        Player &player = ThePlayerManager.GetActivePlayer(i);
        tracks.at(i) = std::make_shared<Encore::Track>(player);
        tracks.at(i)->Load();
        tracks.at(i)->ColumnLeft = -1 + widthPerPlayer * i;
        tracks.at(i)->ColumnRight = -1 + widthPerPlayer * (i + 1);
        switch (player.Instrument) {
        case PlasticGuitar:
        case PlasticBass:
        case PlasticKeys:
            tracks.at(i)->Configure5Lane();
            break;
        case PlasticDrums:
            tracks.at(i)->ConfigureDrums();
            break;
        default:
            if (player.Difficulty == 3) {
                tracks.at(i)->Configure5Lane();
            } else {
                tracks.at(i)->Configure4Lane();
            }
        }
        if (player.Instrument == PlasticBass || player.Instrument == PartVocals
            || player.Instrument == PartBass) {
            player.engine->stats->SixMultiplier = true;
        }
        if (player.Bot)
            player.engine->stats->Bot = true;
    }
    for (auto &stream : TheAudioManager.loadedStreams) {
        stream.volume =
            TheGameSettings.avMainVolume * TheGameSettings.avInactiveInstrumentVolume;
    }
    /*
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
        // player.stats->BaseScore = TheSongList.curSong->parts[player.Instrument]
        //                               ->charts[player.Difficulty]
        //                               .baseScore;
        if (i == 0) {
            ThePlayerManager.BandStats->BaseScore = player.stats->BaseScore;
        } else {
            ThePlayerManager.BandStats->BaseScore += player.stats->BaseScore;
        }
    }
    */
}
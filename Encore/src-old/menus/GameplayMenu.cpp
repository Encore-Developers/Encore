//
// Created by marie on 20/10/2024.
//

#include "GameplayMenu.h"

#include "gameMenu.h"
#include "uiUnits.h"
#include "song/audio.h"
#include "song/songlist.h"
#include "raymath.h"
#include "raygui.h"
#include "gameplay/enctime.h"
#include "styles.h"
#include "easing/easing.h"

#include "users/playerManager.h"
#include "MenuManager.h"
#include "../settings/settings.h"
#include "timingvalues.h"

#include <raylib.h>

#include "settings/keybinds.h"
#include "tracy/Tracy.hpp"

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
    Player &player = *ThePlayerManager.ActivePlayers[0].player;
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
    if (key == KEY_ESCAPE && action == GLFW_PRESS) {
        event.channel = Encore::RhythmEngine::InputChannel::PAUSE;
    }

    // Encore::EncoreLog(LOG_DEBUG, TextFormat("Keyboard key lane %01i",
    // lane));
    event.timestamp = TheSongTime.GetElapsedTime();
    if (event.channel != Encore::RhythmEngine::InputChannel::INVALID)
        engine->ProcessInput(event);
};

void GameplayMenu::ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event) {
    Player &player = *ThePlayerManager.ActivePlayers[0].player;
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
            ThePlayerManager.ActivePlayers[0].player->engine->stats->Score
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
    Player &player = *ThePlayerManager.ActivePlayers[0].player;
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
    // double curTime = GetTime();

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
    double EndTime = TheSongList.curSong->end == 0.0 ? TheSongTime.GetSongLength() : TheSongList.curSong->end;
    if (TheSongTime.GetElapsedTime() > EndTime - 1) {
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

    for (int i = 0; i < ThePlayerManager.ActivePlayers.size(); i++) {
        auto& slot = ThePlayerManager.ActivePlayers[i];
        Player &player = *slot.player;
        if (!player.engine->practice || player.engine->IsWithinPracticeSection(TheSongTime.GetElapsedTime()))
        player.engine->UpdateOnFrame(TheSongTime.GetElapsedTime());
        player.engine->UpdateStats(player.Instrument, player.Difficulty);
        tracks.at(i)->Draw();
        int TopOfScreen = GetRenderHeight(); // width
        int FakeStrikeline = (TopOfScreen / 5) * 4;
        constexpr int NoteXWidth = 150;
        constexpr int NoteHeight = 25;

        int mospos =
            ((GetRenderWidth() + (ThePlayerManager.ActivePlayers.size() * NoteXWidth * 5))
                / (1 + ThePlayerManager.ActivePlayers.size()))
            - ((ThePlayerManager.ActivePlayers.size() * NoteXWidth * 5) / 2);
        int MiddleOfScreen = mospos + (mospos * i); // height
        int TrackLeft = MiddleOfScreen - (NoteXWidth / 2) - NoteXWidth - NoteXWidth;
        auto chart = player.engine->chart;
        int SidesWidth = 20;
        int RailWidth = SidesWidth / 2;

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

    // please God smite this code. flip a few bits in my hard drive. please get rid of this shit somehow
    // there's better ways. forgive me for I have sinned

    if (TheSongTime.GetElapsedTime() > TheSongTime.GetStartTime() + 7.5
        && TheSongTime.GetElapsedTime() < TheSongTime.GetStartTime() + 7.5 + SongNameDuration) {
        double timeSinceStart = TheSongTime.GetElapsedTime() - (TheSongTime.GetStartTime() + 7.5);
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
    } else if (TheSongTime.GetElapsedTime() > TheSongTime.GetStartTime() + 7.5 + SongNameDuration)
        SongNameAlpha = 0;

    if (TheSongTime.GetElapsedTime() > TheSongTime.GetStartTime() + 7.75
        && TheSongTime.GetElapsedTime() < TheSongTime.GetStartTime() + 7.75 + SongNameDuration) {
        double timeSinceStart = TheSongTime.GetElapsedTime() - (TheSongTime.GetStartTime() + 7.75);
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


    if (TheSongTime.GetElapsedTime() > TheSongTime.GetStartTime() + 8
        && TheSongTime.GetElapsedTime() < TheSongTime.GetStartTime() + 8 + SongNameDuration) {
        double timeSinceStart = TheSongTime.GetElapsedTime() - (TheSongTime.GetStartTime() + 8);
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
    if (TheSongTime.GetElapsedTime() < TheSongTime.GetStartTime() + 7.75 + SongNameDuration) {
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
    ZoneScoped;
    TheSongList.curSong->LoadAlbumArt();
    TheAudioManager.loadStreams(TheSongList.curSong->stemsPath);
    TheSongTime.SetOffset(TheGameSettings.AudioOffset / 1000.0);

    // i dont like the game stuttering when you active or get a streak
    float widthPerPlayer = 2.0f / ThePlayerManager.ActivePlayers.size();

    for (int i = 0; i < ThePlayerManager.ActivePlayers.size(); i++) {
        Player &player = *ThePlayerManager.ActivePlayers[i].player;
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
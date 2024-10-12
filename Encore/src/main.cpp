#include "menus/menu.h"
#include "menus/sndTestMenu.h"
#include "menus/cacheLoadingScreen.h"
#include "menus/resultsMenu.h"
#include "util/enclog.h"
#include "gameplay/enctime.h"

#define RAYGUI_IMPLEMENTATION

#if defined(WIN32) && defined(NDEBUG)
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif
#if defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#endif

#include <filesystem>
#include <iostream>
#include <vector>
#include <thread>
#include <condition_variable>
#include <thread>
#include <atomic>

#include "raylib.h"
#include "raygui.h"
#include "raymath.h"
#include "GLFW/glfw3.h"

#include "arguments.h"
#include "assets.h"
#include "song/audio.h"
#include "gameplay/gameplayRenderer.h"
#include "keybinds.h"
#include "old/lerp.h"
#include "menus/gameMenu.h"
#include "menus/overshellRenderer.h"

#include "menus/settingsOptionRenderer.h"

#include "menus/uiUnits.h"
#include "users/player.h"
#include "settings-old.h"
#include "timingvalues.h"
#include "gameplay/inputHandler.h"
#include "inih/INIReader.h"
#include "menus/styles.h"

#include "song/song.h"
#include "song/songlist.h"

GameMenu &menu = TheGameMenu;
SongTime &enctime = TheSongTime;
PlayerManager &playerManager = PlayerManager::getInstance();
SettingsOld &settingsMain = SettingsOld::getInstance();
AudioManager &audioManager = AudioManager::getInstance();

vector<std::string> ArgumentList::arguments;

#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH
#endif

#ifndef ENCORE_VERSION
#define ENCORE_VERSION
#endif

#define SOL_ALL_SAFETIES_ON 1
#define SOL_USE_LUA_HPP 1

bool midiLoaded = false;
bool isPlaying = false;
bool streamsLoaded = false;
bool albumArtSelectedAndLoaded = false;

bool ShowHighwaySettings = true;
bool ShowCalibrationSettings = true;
bool ShowGeneralSettings = true;

int curNoteIndex = 0;
int selLane = 0;
bool selSong = false;
int songSelectOffset = 0;
bool changingKey = false;
bool changing4k = false;
bool changingOverdrive = false;
bool changingAlt = false;
bool changingPause = false;
Vector2 viewScroll = { 0, 0 };
Rectangle view = { 0, 0, 0, 0 };

int HeldMaskShow;

bool isCalibrating = false;
double calibrationStartTime = 0.0;
double lastClickTime = 0.0;
std::vector<double> tapTimes;
const int clickInterval = 1;

bool showInputFeedback = false;
double inputFeedbackStartTime = 0.0;
const double inputFeedbackDuration = 0.6;
float inputFeedbackAlpha = 1.0f;
Color AccentColor = { 255, 0, 255, 255 };
std::string trackSpeedButton;

std::string encoreVersion = ENCORE_VERSION;
std::string commitHash = GIT_COMMIT_HASH;
bool Menu::onNewMenu = false;

gameplayRenderer gpr;

SongList &songList = SongList::getInstance();
Assets &assets = Assets::getInstance();

SortType currentSortValue = SortType::Title;
std::vector<std::string> sortTypes { "Title", "Artist", "Length" };




void Windowed() {
    ClearWindowState(FLAG_WINDOW_UNDECORATED);
    SetWindowSize(
        GetMonitorWidth(GetCurrentMonitor()) * 0.75f,
        GetMonitorHeight(GetCurrentMonitor()) * 0.75f
    );
    SetWindowPosition(
        (GetMonitorWidth(GetCurrentMonitor()) * 0.5f)
            - (GetMonitorWidth(GetCurrentMonitor()) * 0.375f),
        (0.5f * GetMonitorHeight(GetCurrentMonitor()))
            - (GetMonitorHeight(GetCurrentMonitor()) * 0.375f)
    );
}

void FullscreenBorderless() {
    SetWindowSize(
        GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor())
    );
    SetWindowState(FLAG_WINDOW_UNDECORATED);
    SetWindowPosition(0, 0);
}

static void
DrawTextRubik(const char *text, float posX, float posY, float fontSize, Color color) {
    DrawTextEx(assets.rubik, text, { posX, posY }, fontSize, 0, color);
}

static void
DrawTextRHDI(const char *text, float posX, float posY, float fontSize, Color color) {
    DrawTextEx(assets.redHatDisplayItalic, text, { posX, posY }, fontSize, 0, color);
}

static float MeasureTextRubik(const char *text, float fontSize) {
    return MeasureTextEx(assets.rubik, text, fontSize, 0).x;
}

static float MeasureTextRHDI(const char *text, float fontSize) {
    return MeasureTextEx(assets.redHatDisplayItalic, text, fontSize, 0).x;
}

template <typename CharT>
struct Separators : public std::numpunct<CharT> {
    [[nodiscard]] std::string do_grouping() const override { return "\003"; }
};

std::string scoreCommaFormatter(int value) {
    std::stringstream ss;
    ss.imbue(std::locale(std::cout.getloc(), new Separators<char>()));
    ss << std::fixed << value;
    return ss.str();
}

double StrumNoFretTime = 0.0;

bool FAS = false;
int strummedNote = 0;
int FASNote = 0;
OvershellRenderer overshellRenderer;
InputHandler inputHandler;
// what to check when a key changes states (what was the change? was it pressed? or
// released? what time? what window? were any modifiers pressed?)
static void keyCallback(GLFWwindow *wind, int key, int scancode, int action, int mods) {
    Player *player = playerManager.GetActivePlayer(0);
    PlayerGameplayStats *stats = player->stats;
    if (!streamsLoaded) {
        return;
    }
    if (action < 2) {
        // if the key action is NOT repeat (release is 0, press is 1)
        int lane = -2;
        if (key == settingsMain.keybindPause && action == GLFW_PRESS) {
            stats->Paused = !stats->Paused;
            if (stats->Paused && !playerManager.BandStats.Multiplayer) {
                audioManager.pauseStreams();
                enctime.Pause();
            } else if (!playerManager.BandStats.Multiplayer) {
                audioManager.unpauseStreams();
                enctime.Resume();
                for (int i = 0; i < (player->Difficulty == 3 ? 5 : 4); i++) {
                    inputHandler.handleInputs(player, i, -1);
                }
            } // && !player->Bot
        } else if ((key == settingsMain.keybindOverdrive
                    || key == settingsMain.keybindOverdriveAlt)) {
            inputHandler.handleInputs(player, -1, action);
        } else if (!player->Bot) {
            if (player->Instrument != PLASTIC_DRUMS) {
                if (player->Difficulty == 3 || player->ClassicMode) {
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
                        } else if (key == settingsMain.keybinds5KAlt[i] && !stats->HeldFrets[i]) {
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
                        } else if (key == settingsMain.keybinds4KAlt[i] && !stats->HeldFrets[i]) {
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
                if (player->ClassicMode) {
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

static void gamepadStateCallback(int joypadID, GLFWgamepadstate state) {
    Encore::EncoreLog(LOG_DEBUG, TextFormat("Attempted input on joystick %01i", joypadID));
    Player *player;
    if (playerManager.IsGamepadActive(joypadID))
        player = playerManager.GetPlayerGamepad(joypadID);
    else return;

    if (!IsGamepadAvailable(player->joypadID))
        return;
    PlayerGameplayStats *stats = player->stats;
    if (!streamsLoaded) {
        return;
    }

    double eventTime = enctime.GetSongTime();
    if (settingsMain.controllerPause >= 0) {
        if (state.buttons[settingsMain.controllerPause]
            != stats->buttonValues[settingsMain.controllerPause]) {
            stats->buttonValues[settingsMain.controllerPause] =
                state.buttons[settingsMain.controllerPause];
            if (state.buttons[settingsMain.controllerPause] == 1) {
                stats->Paused = !stats->Paused;
                if (stats->Paused && !playerManager.BandStats.Multiplayer) {
                    audioManager.pauseStreams();
                    enctime.Pause();
                } else if (!playerManager.BandStats.Multiplayer) {
                    audioManager.unpauseStreams();
                    enctime.Resume();
                    for (int i = 0; i < (player->Difficulty == 3 ? 5 : 4); i++) {
                        inputHandler.handleInputs(player, i, -1);
                    }
                } // && !player->Bot
            }
        }
    } else if (!player->Bot) {
        if (state.axes[-(settingsMain.controllerPause + 1)]
            != stats->axesValues[-(settingsMain.controllerPause + 1)]) {
            stats->axesValues[-(settingsMain.controllerPause + 1)] =
                state.axes[-(settingsMain.controllerPause + 1)];
            if (state.axes[-(settingsMain.controllerPause + 1)]
                == 1.0f * (float)settingsMain.controllerPauseAxisDirection) {
            }
        }
    } //  && !player->Bot
    if (settingsMain.controllerOverdrive >= 0) {
        if (state.buttons[settingsMain.controllerOverdrive]
            != stats->buttonValues[settingsMain.controllerOverdrive]) {
            stats->buttonValues[settingsMain.controllerOverdrive] =
                state.buttons[settingsMain.controllerOverdrive];
            inputHandler.handleInputs(player, -1, state.buttons[settingsMain.controllerOverdrive]);
        } // // if (!player->Bot)
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
    if ((player->Difficulty == 3 || player->ClassicMode) && !player->Bot) {
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
                    inputHandler.handleInputs(player, i, state.buttons[settingsMain.controller5K[i]]);
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
            && player->ClassicMode && !stats->UpStrum) {
            stats->UpStrum = true;
            stats->Overstrum = false;
            inputHandler.handleInputs(player, 8008135, GLFW_PRESS);
        } else if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_RELEASE
                   && player->ClassicMode
                   && stats->UpStrum) {
            stats->UpStrum = false;
            inputHandler.handleInputs(player, 8008135, GLFW_RELEASE);
        }
        if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] == GLFW_PRESS
            && player->ClassicMode && !stats->DownStrum) {
            stats->DownStrum = true;
            stats->Overstrum = false;
            inputHandler.handleInputs(player, 8008135, GLFW_PRESS);
        } else if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] == GLFW_RELEASE
                   && player->ClassicMode
                   && stats->DownStrum) {
            stats->DownStrum = false;
            inputHandler.handleInputs(player, 8008135, GLFW_RELEASE);
        }
    } else if (!player->Bot) {
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
                    inputHandler.handleInputs(player, i, state.buttons[settingsMain.controller4K[i]]);
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
/*
static void gamepadStateCallbackSetControls(int jid, GLFWgamepadstate state) {
    for (int i = 0; i < 6; i++) {
        axesValues2[i] = state.axes[i];
    }
    if (changingKey || changingOverdrive || changingPause) {
        for (int i = 0; i < 15; i++) {
            if (state.buttons[i] == 1) {
                if (buttonValues[i] == 0) {
                    controllerID = jid;
                    pressedGamepadInput = i;
                    return;
                } else {
                    buttonValues[i] = state.buttons[i];
                }
            }
        }
        for (int i = 0; i < 6; i++) {
            if (state.axes[i] == 1.0f || (i <= 3 && state.axes[i] == -1.0f)) {
                axesValues[i] = 0.0f;
                if (state.axes[i] == 1.0f) axisDirection = 1;
                else axisDirection = -1;
                controllerID = jid;
                pressedGamepadInput = -(1 + i);
                return;
            } else {
                axesValues[i] = 0.0f;
            }
        }
    } else {
        for (int i = 0; i < 15; i++) {
            buttonValues[i] = state.buttons[i];
        }
        for (int i = 0; i < 6; i++) {
            axesValues[i] = state.axes[i];
        }
        pressedGamepadInput = -999;
    }
}
*/
int minWidth = 640;
int minHeight = 480;

enum OptionsCategories {
    MAIN,
    HIGHWAY,
    VOLUME,
    KEYBOARD,
    GAMEPAD
};

enum KeybindCategories {
    kbPAD,
    kbCLASSIC,
    kbMISC,
    kbMENUS
};

enum JoybindCategories {
    gpPAD,
    gpCLASSIC,
    gpMISC,
    gpMENUS
};

Keybinds keybinds;

bool FinishedLoading = false;
bool firstInit = true;
int loadedAssets;
bool albumArtLoaded = false;

settingsOptionRenderer sor;

Menu *ActiveMenu = nullptr;

bool ReloadGameplayTexture = true;
bool songAlbumArtLoadedGameplay = false;

void LoadCharts() {
    smf::MidiFile midiFile;
    midiFile.read(songList.curSong->midiPath.string());
    songList.curSong->getTiming(midiFile, 0, midiFile[0]);
    for (int playerNum = 0; playerNum < playerManager.PlayersActive; playerNum++) {
        Player *player = playerManager.GetActivePlayer(playerNum);
        int diff = player->Difficulty;
        int inst = player->Instrument;
        for (int track = 0; track < midiFile.getTrackCount(); track++) {
            std::string trackName;
            for (int events = 0; events < midiFile[track].getSize(); events++) {
                if (midiFile[track][events].isMeta()) {
                    if ((int)midiFile[track][events][1] == 3) {
                        for (int k = 3; k < midiFile[track][events].getSize(); k++) {
                            trackName += midiFile[track][events][k];
                        }
                        SongParts songPart;
                        if (songList.curSong->ini) {
                            songPart = songList.curSong->partFromStringINI(trackName);
                            INIReader ini(songList.curSong->songInfoPath);
                            songList.curSong->hopoThreshold =
                                ini.GetInteger("song", "hopo_frequency", 170);
                        } else
                            songPart = songList.curSong->partFromString(trackName);
                        if (trackName == "BEAT") {
                            LoadingState = BEATLINES;
                            songList.curSong->parseBeatLines(
                                midiFile, track, midiFile[track]
                            );
                        } else if (trackName == "EVENTS") {
                            for (int forDiff = 0;
                                 forDiff < songList.curSong->parts[inst]->charts.size();
                                 forDiff++) {
                                songList.curSong->parts[inst]->charts[forDiff].getSections(
                                    midiFile, track
                                );
                            }
                        } else {
                            if (songPart != SongParts::Invalid && songPart == inst) {
                                for (int forDiff = 0; forDiff
                                     < songList.curSong->parts[inst]->charts.size();
                                     forDiff++) {
                                    Chart &chart =
                                        songList.curSong->parts[inst]->charts[forDiff];
                                    if (chart.valid) {
                                        Encore::EncoreLog(
                                            LOG_DEBUG,
                                            TextFormat(
                                                "Loading part %s, diff %01i",
                                                trackName.c_str(),
                                                forDiff
                                            )
                                        );
                                        LoadingState = NOTE_PARSING;
                                        if (songPart == SongParts::PlasticBass
                                            || songPart == SongParts::PlasticGuitar
                                            || songPart == SongParts::PlasticKeys) {
                                            chart.plastic = true;
                                            chart.parsePlasticNotes(
                                                midiFile,
                                                track,
                                                forDiff,
                                                (int)songPart,
                                                songList.curSong->hopoThreshold
                                            );
                                        } else if (songPart == PlasticDrums) {
                                            chart.plastic = true;
                                            chart.parsePlasticDrums(
                                                midiFile,
                                                track,
                                                midiFile[track],
                                                forDiff,
                                                (int)songPart,
                                                player->ProDrums,
                                                true
                                            );
                                        } else {
                                            chart.plastic = false;
                                            chart.parseNotes(
                                                midiFile,
                                                track,
                                                midiFile[track],
                                                forDiff,
                                                (int)songPart
                                            );
                                        }

                                        if (!chart.plastic) {
                                            LoadingState = EXTRA_PROCESSING;
                                            int noteIdx = 0;
                                            for (Note &note : chart.notes) {
                                                chart.notes_perlane[note.lane].push_back(
                                                    noteIdx
                                                );
                                                noteIdx++;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    songList.curSong->getCodas(midiFile);
    LoadingState = READY;
    this_thread::sleep_for(chrono::seconds(1));
    FinishedLoading = true;
}

bool StartLoading = true;

bool doRenderThingToLowerHighway = false;

int CurSongInt = 0;

SongParts GetSongPart(smf::MidiEventList track) {
    for (int events = 0; events < track.getSize(); events++) {
        std::string trackName;
        if (!track[events].isMeta())
            continue;
        if ((int)track[events][1] == 3) {
            for (int k = 3; k < track[events].getSize(); k++) {
                trackName += track[events][k];
            }
            if (songList.curSong->ini)
                return songList.curSong->partFromStringINI(trackName);
            else
                return songList.curSong->partFromString(trackName);
            break;
        }
    }
}

std::vector<std::vector<int> > pDiffNotes = {
    { 60, 64 }, { 72, 76 }, { 84, 88 }, { 96, 100 }
};

void IsPartValid(smf::MidiEventList track, SongParts songPart) {
    if (songPart != SongParts::Invalid && songPart != PitchedVocals) {
        for (int diff = 0; diff < 4; diff++) {
            bool StopSearching = false;
            Chart newChart;
            for (int i = 0; i < track.getSize(); i++) {

                if (track[i].isNoteOn() && !track[i].isMeta()
                    && (int)track[i][1] >= pDiffNotes[diff][0]
                    && (int)track[i][1] <= pDiffNotes[diff][1] && !StopSearching) {
                    newChart.valid = true;
                    newChart.diff = diff;
                    songList.curSong->parts[(int)songPart]->hasPart = true;
                    StopSearching = true;

                }
            }
            if (songPart > songList.curSong->parts[(int)songPart]->charts.size())
                if (songPart < PitchedVocals)
                    songList.curSong->parts[(int)songPart]->charts.push_back(newChart);
        }
    }
}

int main(int argc, char *argv[]) {
    SetTraceLogCallback(Encore::EncoreLog);
    Units u = Units::getInstance();
    commitHash.erase(7);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    // SetConfigFlags(FLAG_VSYNC_HINT);

    // SetEncore::EncoreLogLevel(LOG_NONE);

    // 800 , 600

    SetWindowState(FLAG_MSAA_4X_HINT);
    bool windowToggle = true;
    ArgumentList::InitArguments(argc, argv);

    std::string FPSCapStringVal = ArgumentList::GetArgValue("fpscap");
    std::string vSyncOn = ArgumentList::GetArgValue("vsync");
    int targetFPSArg = -1;
    int vsyncArg = 1;

    if (!FPSCapStringVal.empty()) {
        targetFPSArg = strtol(FPSCapStringVal.c_str(), NULL, 10);
        if (targetFPSArg > 0)
            Encore::EncoreLog(
                LOG_INFO, TextFormat("Argument overridden target FPS: %d", targetFPSArg)
            );
        else
            Encore::EncoreLog(
                LOG_INFO, TextFormat("Unlocked framerate. You asked for it.")
            );
    }

    if (!vSyncOn.empty()) {
        vsyncArg = strtol(vSyncOn.c_str(), NULL, 10);
        Encore::EncoreLog(
            LOG_INFO, TextFormat("Vertical sync argument toggled: %d", vsyncArg)
        );
    }
    if (vsyncArg == 1) {
        SetConfigFlags(FLAG_VSYNC_HINT);
    }

    InitWindow(1, 1, "Encore");
    // https://www.raylib.com/examples/core/loader.html?name=core_custom_frame_control

    double previousTime = GetTime();
    double currentTime = 0.0;
    double updateDrawTime = 0.0;
    double waitTime = 0.0;
    float deltaTime = 0.0f;

    float timeCounter = 0.0f;
    gpr.sustainPlane = GenMeshPlane(0.8f, 1.0f, 1, 1);
    gpr.soloPlane = GenMeshPlane(1.0f, 1.0f, 1, 1);
    std::filesystem::path executablePath(GetApplicationDirectory());

    std::filesystem::path directory = executablePath.parent_path();

#ifdef __APPLE__
    CFBundleRef bundle = CFBundleGetMainBundle();
    if (bundle != NULL) {
        // get the Resources directory for our binary for the Assets handling
        CFURLRef resourceURL = CFBundleCopyResourcesDirectoryURL(bundle);
        if (resourceURL != NULL) {
            char resourcePath[PATH_MAX];
            if (CFURLGetFileSystemRepresentation(
                    resourceURL, true, (UInt8 *)resourcePath, PATH_MAX
                ))
                assets.setDirectory(resourcePath);
            CFRelease(resourceURL);
        }
        // do the next step manually (settings/config handling)
        // "directory" is our executable directory here, hop up to the external dir
        if (directory.filename().compare("MacOS") == 0)
            directory =
                directory.parent_path().parent_path().parent_path(); // hops
                                                                     // "MacOS",
                                                                     // "Contents",
                                                                     // "Encore.app"
                                                                     // into
                                                                     // containing
                                                                     // folder

        CFRelease(bundle);
    }
#endif
    settingsMain.setDirectory(directory);

    if (std::filesystem::exists(directory / "keybinds.json")) {
        settingsMain.migrateSettings(
            directory / "keybinds.json", directory / "settings.json"
        );
    }
    settingsMain.loadSettings(directory / "settings.json");
    playerManager.LoadPlayerList(directory / "players.json");
    // player.InputOffset = settingsMain.inputOffsetMS / 1000.0f;
    // player.VideoOffset = settingsMain.avOffsetMS / 1000.0f;
    bool removeFPSLimit = 0;
    int targetFPS =
        targetFPSArg == -1 ? GetMonitorRefreshRate(GetCurrentMonitor()) : targetFPSArg;
    removeFPSLimit = targetFPSArg == 0;
    int menuFPS = 60;
    /*
    if (!settingsMain.fullscreen) {
        if (IsWindowState(FLAG_WINDOW_UNDECORATED)) {
            ClearWindowState(FLAG_WINDOW_UNDECORATED);
            SetWindowState(FLAG_MSAA_4X_HINT);
        }
        SetWindowSize(GetMonitorWidth(GetCurrentMonitor()) * 0.75f,
                    GetMonitorHeight(GetCurrentMonitor()) * 0.75f);
        SetWindowPosition(
            (GetMonitorWidth(GetCurrentMonitor()) * 0.5f) -
    (GetMonitorWidth(GetCurrentMonitor()) * 0.375f),
            (GetMonitorHeight(GetCurrentMonitor()) * 0.5f) -
            (GetMonitorHeight(GetCurrentMonitor()) * 0.375f));
    } else {
        SetWindowState(FLAG_WINDOW_UNDECORATED + FLAG_MSAA_4X_HINT);
        int CurrentMonitor = GetCurrentMonitor();
        SetWindowPosition(0, 0);
        SetWindowSize(GetMonitorWidth(CurrentMonitor), GetMonitorHeight(CurrentMonitor));
    }

    */
    if (!settingsMain.fullscreen) {
        Windowed();
    } else {
        FullscreenBorderless();
    }
    std::vector<std::string> songPartsList {
        "Drums",        "Bass",         "Guitar",         "Vocals", "Classic Drums",
        "Classic Bass", "Classic Lead", "Classic Vocals", "Keys",   "Classic Keys"
    };
    std::string diffList[4] = { "Easy", "Medium", "Hard", "Expert" };
    Encore::EncoreLog(LOG_INFO, TextFormat("Target FPS: %d", targetFPS));

    audioManager.Init();
    SetExitKey(0);
    audioManager.loadSample("Assets/combobreak.mp3", "miss");

    // Y UP!!!! REMEMBER!!!!!!
    //							  x,    y,     z
    //                         0.0f, 5.0f, -3.5f
    //								 6.5f

    // singleplayer
    // 0.0f, 0.0f, 6.5f
    float Height = 7.25f;
    float Back = -10.0f;
    float FOV = 45.0f;
    float TargetDistance = 20.0f;
    gpr.camera1p.position = Vector3 { 0.0f, Height, Back };
    gpr.camera1p.target = Vector3 { 0.0f, 0.0f, TargetDistance };
    gpr.camera1p.up = Vector3 { 0.0f, 1.0f, 0.0f };
    gpr.camera1p.fovy = FOV;

    gpr.camera1pVector.push_back(gpr.camera1p);

    // 2 player
    float SideDisplacement2p = 0.75f;

    gpr.camera2p1.position = Vector3 { SideDisplacement2p, Height, Back };
    gpr.camera2p1.target = Vector3 { SideDisplacement2p, 0.0f, TargetDistance };
    gpr.camera2p1.up = Vector3 { 0.0f, 1.0f, 0.0f };
    gpr.camera2p1.fovy = FOV;

    gpr.camera2p2.position = Vector3 { -SideDisplacement2p, Height, Back };
    gpr.camera2p2.target = Vector3 { -SideDisplacement2p, 0.0f, TargetDistance };
    gpr.camera2p2.up = Vector3 { 0.0f, 1.0f, 0.0f };
    gpr.camera2p2.fovy = FOV;

    gpr.camera2pVector.push_back(gpr.camera2p1);
    gpr.camera2pVector.push_back(gpr.camera2p2);

    // 3 player
    float SideDisplacement3p = 1.25f;
    gpr.camera3p1.position = Vector3 { 0.0f, Height, Back };
    gpr.camera3p1.target = Vector3 { 0.0f, 0.0f, TargetDistance };
    gpr.camera3p1.up = Vector3 { 0.0f, 1.0f, 0.0f };
    gpr.camera3p1.fovy = FOV;
    gpr.camera3pVector.push_back(gpr.camera3p1);

    gpr.camera3p2.position = Vector3 { SideDisplacement3p, Height, Back };
    gpr.camera3p2.target = Vector3 { SideDisplacement3p, 0.0f, TargetDistance };
    gpr.camera3p2.up = Vector3 { 0.0f, 1.0f, 0.0f };
    gpr.camera3p2.fovy = FOV;
    gpr.camera3pVector.push_back(gpr.camera3p2);

    gpr.camera3p3.position = Vector3 { -SideDisplacement3p, Height, Back };
    gpr.camera3p3.target = Vector3 { -SideDisplacement3p, 0.0f, TargetDistance };
    gpr.camera3p3.up = Vector3 { 0.0f, 1.0f, 0.0f };
    gpr.camera3p3.fovy = FOV;
    gpr.camera3pVector.push_back(gpr.camera3p3);

    float SideDisplacement4p = 3.0f;
    float SideDisplacement4p2 = 1.0f;
    float Back4p = -14.0f;
    float Height4p = 10.0f;
    gpr.camera4p1.position = Vector3 { SideDisplacement4p2, Height4p, Back4p };
    gpr.camera4p1.target = Vector3 { SideDisplacement4p2, 0.0f, TargetDistance };
    gpr.camera4p1.up = Vector3 { 0.0f, 1.0f, 0.0f };
    gpr.camera4p1.fovy = FOV;
    gpr.camera4pVector.push_back(gpr.camera4p1);

    gpr.camera4p2.position = Vector3 { SideDisplacement4p, Height4p, Back4p };
    gpr.camera4p2.target = Vector3 { SideDisplacement4p, 0.0f, TargetDistance };
    gpr.camera4p2.up = Vector3 { 0.0f, 1.0f, 0.0f };
    gpr.camera4p2.fovy = FOV;
    gpr.camera4pVector.push_back(gpr.camera4p2);

    gpr.camera4p3.position = Vector3 { -SideDisplacement4p, Height4p, Back4p };
    gpr.camera4p3.target = Vector3 { -SideDisplacement4p, 0.0f, TargetDistance };
    gpr.camera4p3.up = Vector3 { 0.0f, 1.0f, 0.0f };
    gpr.camera4p3.fovy = FOV;
    gpr.camera4pVector.push_back(gpr.camera4p3);

    gpr.camera4p4.position = Vector3 { -SideDisplacement4p2, Height4p, Back4p };
    gpr.camera4p4.target = Vector3 { -SideDisplacement4p2, 0.0f, TargetDistance };
    gpr.camera4p4.up = Vector3 { 0.0f, 1.0f, 0.0f };
    gpr.camera4p4.fovy = FOV;
    gpr.camera4pVector.push_back(gpr.camera4p4);

    gpr.cameraVectors.push_back(gpr.camera1pVector);
    gpr.cameraVectors.push_back(gpr.camera2pVector);
    gpr.cameraVectors.push_back(gpr.camera3pVector);
    gpr.cameraVectors.push_back(gpr.camera4pVector);

    char trackSpeedStr[256];
    snprintf(
        trackSpeedStr, 255, "%.3f", settingsMain.trackSpeedOptions[settingsMain.trackSpeed]
    );
    trackSpeedButton = "Track Speed " + std::string(trackSpeedStr) + "x";

    ChangeDirectory(GetApplicationDirectory());

    GLFWkeyfun origKeyCallback = glfwSetKeyCallback(glfwGetCurrentContext(), keyCallback);
    GLFWgamepadstatefun origGamepadCallback =
        glfwSetGamepadStateCallback(gamepadStateCallback);
    glfwSetKeyCallback(glfwGetCurrentContext(), origKeyCallback);
    glfwSetGamepadStateCallback(origGamepadCallback);
    // GuiLoadStyle((directory / "Assets/ui/encore.rgs").string().c_str());

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
    GuiSetStyle(
        BUTTON, BASE_COLOR_FOCUSED, ColorToInt(ColorBrightness(AccentColor, -0.5))
    );
    GuiSetStyle(
        BUTTON, BASE_COLOR_PRESSED, ColorToInt(ColorBrightness(AccentColor, -0.3))
    );
    GuiSetStyle(
        SLIDER, BORDER_COLOR_FOCUSED, ColorToInt(ColorBrightness(AccentColor, -0.3))
    );
    GuiSetStyle(
        SLIDER, BORDER_COLOR_NORMAL, ColorToInt(ColorBrightness(AccentColor, -0.5))
    );
    GuiSetStyle(
        SLIDER, BORDER_COLOR_PRESSED, ColorToInt(AccentColor)
    );
    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, 0xFFFFFFFF);
    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, 0xFFFFFFFF);
    GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, 0xFFFFFFFF);
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0x505050ff);
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0xcbcbcbFF);
    GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, 0xFFFFFFFF);
    GuiSetStyle(BUTTON, TEXT_COLOR_PRESSED, 0xFFFFFFFF);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 28);

    GuiSetStyle(TOGGLE, BASE_COLOR_NORMAL, 0x181827FF);
    GuiSetStyle(
        TOGGLE, BASE_COLOR_FOCUSED, ColorToInt(ColorBrightness(AccentColor, -0.5))
    );
    GuiSetStyle(
        TOGGLE, BASE_COLOR_PRESSED, ColorToInt(ColorBrightness(AccentColor, -0.3))
    );
    GuiSetStyle(TOGGLE, BORDER_COLOR_NORMAL, 0xFFFFFFFF);
    GuiSetStyle(TOGGLE, BORDER_COLOR_FOCUSED, 0xFFFFFFFF);
    GuiSetStyle(TOGGLE, BORDER_COLOR_PRESSED, 0xFFFFFFFF);
    GuiSetStyle(TOGGLE, TEXT_COLOR_FOCUSED, 0xFFFFFFFF);
    GuiSetStyle(TOGGLE, TEXT_COLOR_PRESSED, 0xFFFFFFFF);

    gpr.sustainPlane = GenMeshPlane(0.8f, 1.0f, 1, 1);
    // bool wideSoloPlane = player.diff == 3;
    // gpr.soloPlane = GenMeshPlane(wideSoloPlane ? 6 : 5, 1.0f, 1, 1);

    SetRandomSeed(std::chrono::system_clock::now().time_since_epoch().count());
    assets.FirstAssets();
    SetWindowIcon(assets.icon);
    GuiSetFont(assets.rubik);
    assets.LoadAssets();
    RenderTexture2D notes_tex = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    RenderTexture2D hud_tex = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    RenderTexture2D highway_tex = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    RenderTexture2D highwayStatus_tex =
        LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    RenderTexture2D smasher_tex = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    menu.currentScreen = CACHE_LOADING_SCREEN;
    Menu::onNewMenu = true;
    enctime.SetOffset(settingsMain.avOffsetMS / 1000.0);
    audioManager.loadSample("Assets/highway/clap.mp3", "clap");
    while (!WindowShouldClose()) {
        u.calcUnits();
        GuiSetStyle(DEFAULT, TEXT_SIZE, (int)u.hinpct(0.03f));
        GuiSetStyle(DEFAULT, TEXT_SPACING, 0);
        double curTime = GetTime();
        float bgTime = curTime / 5.0f;
        if (IsKeyPressed(KEY_F11)
            || (IsKeyPressed(KEY_LEFT_ALT) && IsKeyPressed(KEY_ENTER))) {
            settingsMain.fullscreen = !settingsMain.fullscreen;
            if (!settingsMain.fullscreen) {
                Windowed();
            } else {
                FullscreenBorderless();
            }
        }
        if (GetScreenWidth() < minWidth) {
            if (GetScreenHeight() < minHeight)
                SetWindowSize(minWidth, minHeight);
            else
                SetWindowSize(minWidth, GetScreenHeight());
        }
        if (GetScreenHeight() < minHeight) {
            if (GetScreenWidth() < minWidth)
                SetWindowSize(minWidth, minHeight);
            else
                SetWindowSize(GetScreenWidth(), minHeight);
        }

        // float diffDistance = player.diff == 3 ? 2.0f : 1.5f;
        // float lineDistance = player.diff == 3 ? 1.5f : 1.0f;
        BeginDrawing();
        ClearBackground(DARKGRAY);
        SetShaderValue(assets.bgShader, assets.bgTimeLoc, &bgTime, SHADER_UNIFORM_FLOAT);

        if (Menu::onNewMenu) {
            Menu::onNewMenu = false;
            delete ActiveMenu;
            ActiveMenu = NULL;
            switch (menu.currentScreen) { // NOTE: when adding a new Menu derivative, you
                                          // must put its enum value in Screens, and its
                                          // assignment in this switch/case. You will also
                                          // add its case to the `ActiveMenu->Draw();`
                                          // cases.
            case RESULTS: {
                ActiveMenu = new resultsMenu;
                ActiveMenu->Load();
                break;
            }
            case SOUND_TEST: {
                ActiveMenu = new SoundTestMenu;
                ActiveMenu->Load();
                break;
            }
            case CACHE_LOADING_SCREEN: {
                ActiveMenu = new cacheLoadingScreen;
                ActiveMenu->Load();
                break;
            }
            default:;
            }
        }

        switch (menu.currentScreen) {
        case MENU: {
            // if (!menu.songsLoaded) {
            //	if (std::filesystem::exists("songCache.encr")) {
            //		songList.LoadCache(settingsMain.songPaths);
            //		menu.songsLoaded = true;
            //	}
            // }

            menu.loadMainMenu();
            break;
        }
        case CALIBRATION: {
            static bool sampleLoaded = false;
            if (!sampleLoaded) {
                audioManager.loadSample("Assets/kick.wav", "click");
                sampleLoaded = true;
            }

            if (GuiButton(
                    { (float)GetScreenWidth() / 2 - 250,
                      (float)GetScreenHeight() - 120,
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
                    { (float)GetScreenWidth() / 2 + 50,
                      (float)GetScreenHeight() - 120,
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
                    settingsMain.avOffsetMS =
                        static_cast<int>((totalDifference / tapTimes.size()) * 1000);
                    // Convert to milliseconds
                    settingsMain.inputOffsetMS = settingsMain.avOffsetMS;
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
                    audioManager.playSample("click", 1);
                    lastClickTime += clickInterval;
                    // Increment by the interval to avoid missing clicks
                    std::cout << "Click" << std::endl;
                }

                if (IsKeyPressed(settingsMain.keybindOverdrive)) {
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
                    { static_cast<float>((GetScreenWidth() - u.hinpct(0.35f)) / 2),
                      static_cast<float>(GetScreenHeight() / 2) },
                    u.hinpct(0.05f),
                    0,
                    feedbackColor
                );
            }

            if (GuiButton(
                    { ((float)GetScreenWidth() / 2) - 350,
                      ((float)GetScreenHeight() - 60),
                      100,
                      60 },
                    "Cancel"
                )) {
                isCalibrating = false;
                settingsMain.avOffsetMS = settingsMain.prevAvOffsetMS;
                settingsMain.inputOffsetMS = settingsMain.prevInputOffsetMS;
                tapTimes.clear();

                settingsMain.saveSettings(directory / "settings.json");
                menu.SwitchScreen(SETTINGS);
            }

            if (GuiButton(
                    { ((float)GetScreenWidth() / 2) + 250,
                      ((float)GetScreenHeight() - 60),
                      100,
                      60 },
                    "Apply"
                )) {
                isCalibrating = false;
                settingsMain.prevAvOffsetMS = settingsMain.avOffsetMS;
                settingsMain.prevInputOffsetMS = settingsMain.inputOffsetMS;
                tapTimes.clear();

                settingsMain.saveSettings(directory / "settings.json");
                menu.SwitchScreen(SETTINGS);
            }

            break;
        }
        case SETTINGS: {
            if (menu.songsLoaded)
                menu.DrawAlbumArtBackground(songList.curSong->albumArtBlur);
            // if (settingsMain.controllerType == -1 && controllerID != -1) {
            //	std::string gamepadName = std::string(glfwGetGamepadName(controllerID));
            //	settingsMain.controllerType = keybinds.getControllerType(gamepadName);
            //}
            float TextPlacementTB = u.hpct(0.15f) - u.hinpct(0.11f);
            float TextPlacementLR = u.wpct(0.01f);
            DrawRectangle(
                u.LeftSide, 0, u.winpct(1.0f), GetScreenHeight(), Color { 0, 0, 0, 128 }
            );
            DrawLineEx(
                { u.LeftSide + u.winpct(0.0025f), 0 },
                { u.LeftSide + u.winpct(0.0025f), (float)GetScreenHeight() },
                u.winpct(0.005f),
                WHITE
            );
            DrawLineEx(
                { u.RightSide - u.winpct(0.0025f), 0 },
                { u.RightSide - u.winpct(0.0025f), (float)GetScreenHeight() },
                u.winpct(0.005f),
                WHITE
            );

            overshellRenderer.DrawTopOvershell(0.15f);
            menu.DrawVersion();
            menu.DrawBottomOvershell();
            DrawTextEx(
                assets.redHatDisplayBlack,
                "Options",
                { TextPlacementLR, TextPlacementTB },
                u.hinpct(0.10f),
                0,
                WHITE
            );

            float OvershellBottom = u.hpct(0.15f);
            if (GuiButton(
                    { ((float)GetScreenWidth() / 2) - 350,
                      ((float)GetScreenHeight() - 60),
                      100,
                      60 },
                    "Cancel"
                )
                && !(changingKey || changingOverdrive || changingPause)) {
                glfwSetGamepadStateCallback(origGamepadCallback);
                settingsMain.keybinds4K = settingsMain.prev4K;
                settingsMain.keybinds5K = settingsMain.prev5K;
                settingsMain.keybinds4KAlt = settingsMain.prev4KAlt;
                settingsMain.keybinds5KAlt = settingsMain.prev5KAlt;
                settingsMain.keybindOverdrive = settingsMain.prevOverdrive;
                settingsMain.keybindOverdriveAlt = settingsMain.prevOverdriveAlt;
                settingsMain.keybindPause = settingsMain.prevKeybindPause;

                settingsMain.controller4K = settingsMain.prevController4K;
                settingsMain.controller4KAxisDirection =
                    settingsMain.prevController4KAxisDirection;
                settingsMain.controller5K = settingsMain.prevController5K;
                settingsMain.controller5KAxisDirection =
                    settingsMain.prevController5KAxisDirection;
                settingsMain.controllerOverdrive = settingsMain.prevControllerOverdrive;
                settingsMain.controllerOverdriveAxisDirection =
                    settingsMain.prevControllerOverdriveAxisDirection;
                settingsMain.controllerType = settingsMain.prevControllerType;
                settingsMain.controllerPause = settingsMain.prevControllerPause;

                settingsMain.highwayLengthMult = settingsMain.prevHighwayLengthMult;
                settingsMain.trackSpeed = settingsMain.prevTrackSpeed;
                settingsMain.inputOffsetMS = settingsMain.prevInputOffsetMS;
                settingsMain.avOffsetMS = settingsMain.prevAvOffsetMS;
                settingsMain.missHighwayColor = settingsMain.prevMissHighwayColor;
                settingsMain.mirrorMode = settingsMain.prevMirrorMode;
                settingsMain.fullscreen = settingsMain.fullscreenPrev;

                settingsMain.MainVolume = settingsMain.prevMainVolume;
                settingsMain.PlayerVolume = settingsMain.prevPlayerVolume;
                settingsMain.BandVolume = settingsMain.prevBandVolume;
                settingsMain.SFXVolume = settingsMain.prevSFXVolume;
                settingsMain.MissVolume = settingsMain.prevMissVolume;
                settingsMain.MenuVolume = settingsMain.prevMenuVolume;
                enctime.SetOffset(settingsMain.avOffsetMS / 1000.0);
                menu.SwitchScreen(MENU);
            }
            if (GuiButton(
                    { ((float)GetScreenWidth() / 2) + 250,
                      ((float)GetScreenHeight() - 60),
                      100,
                      60 },
                    "Apply"
                )
                && !(changingKey || changingOverdrive || changingPause)) {
                glfwSetGamepadStateCallback(origGamepadCallback);
                if (settingsMain.fullscreen) {
                    FullscreenBorderless();
                    // SetWindowState(FLAG_WINDOW_UNDECORATED);
                    // SetWindowState(FLAG_MSAA_4X_HINT);
                    // int CurrentMonitor = GetCurrentMonitor();
                    // SetWindowPosition(0, 0);
                    // SetWindowSize(GetMonitorWidth(CurrentMonitor),
                    //			GetMonitorHeight(CurrentMonitor));
                } else {
                    Windowed();
                }
                settingsMain.prev4K = settingsMain.keybinds4K;
                settingsMain.prev5K = settingsMain.keybinds5K;
                settingsMain.prev4KAlt = settingsMain.keybinds4KAlt;
                settingsMain.prev5KAlt = settingsMain.keybinds5KAlt;
                settingsMain.prevOverdrive = settingsMain.keybindOverdrive;
                settingsMain.prevOverdriveAlt = settingsMain.keybindOverdriveAlt;
                settingsMain.prevKeybindPause = settingsMain.keybindPause;

                settingsMain.prevController4K = settingsMain.controller4K;
                settingsMain.prevController4KAxisDirection =
                    settingsMain.controller4KAxisDirection;
                settingsMain.prevController5K = settingsMain.controller5K;
                settingsMain.prevController5KAxisDirection =
                    settingsMain.controller5KAxisDirection;
                settingsMain.prevControllerOverdrive = settingsMain.controllerOverdrive;
                settingsMain.prevControllerPause = settingsMain.controllerPause;
                settingsMain.prevControllerOverdriveAxisDirection =
                    settingsMain.controllerOverdriveAxisDirection;
                settingsMain.prevControllerType = settingsMain.controllerType;

                settingsMain.prevHighwayLengthMult = settingsMain.highwayLengthMult;
                settingsMain.prevTrackSpeed = settingsMain.trackSpeed;
                settingsMain.prevInputOffsetMS = settingsMain.inputOffsetMS;
                settingsMain.prevAvOffsetMS = settingsMain.avOffsetMS;
                settingsMain.prevMissHighwayColor = settingsMain.missHighwayColor;
                settingsMain.prevMirrorMode = settingsMain.mirrorMode;
                settingsMain.fullscreenPrev = settingsMain.fullscreen;

                settingsMain.prevMainVolume = settingsMain.MainVolume;
                settingsMain.prevPlayerVolume = settingsMain.PlayerVolume;
                settingsMain.prevBandVolume = settingsMain.BandVolume;
                settingsMain.prevSFXVolume = settingsMain.SFXVolume;
                settingsMain.prevMissVolume = settingsMain.MissVolume;
                settingsMain.prevMenuVolume = settingsMain.MenuVolume;

                // player.InputOffset = settingsMain.inputOffsetMS / 1000.0f;
                // player.VideoOffset = settingsMain.avOffsetMS / 1000.0f;
                enctime.SetOffset(settingsMain.avOffsetMS / 1000.0);
                settingsMain.saveSettings(directory / "settings.json");

                menu.SwitchScreen(MENU);
            }
            static int selectedTab = 0;
            static int displayedTab = 0;

            static int selectedKbTab = 0;
            static int displayedKbTab = 0;

            GuiToggleGroup(
                { u.LeftSide + u.winpct(0.005f),
                  OvershellBottom,
                  (u.winpct(0.985f) / 5),
                  u.hinpct(0.05) },
                "Main;Highway;Volume;Keyboard Controls;Gamepad Controls",
                &selectedTab
            );
            if (!changingKey && !changingOverdrive && !changingPause) {
                displayedTab = selectedTab;
            } else {
                selectedTab = displayedTab;
            }
            if (!changingKey && !changingOverdrive && !changingPause) {
                displayedKbTab = selectedKbTab;
            } else {
                selectedKbTab = displayedKbTab;
            }
            float EntryFontSize = u.hinpct(0.03f);
            float EntryHeight = u.hinpct(0.05f);
            float EntryTop = OvershellBottom + u.hinpct(0.1f);
            float HeaderTextLeft = u.LeftSide + u.winpct(0.015f);
            float EntryTextLeft = u.LeftSide + u.winpct(0.025f);
            float EntryTextTop = EntryTop + u.hinpct(0.01f);
            float OptionLeft = u.LeftSide + u.winpct(0.005f) + u.winpct(0.989f) / 3;
            float OptionWidth = u.winpct(0.989f) / 3;
            float OptionRight = OptionLeft + OptionWidth;

            float underTabsHeight = OvershellBottom + u.hinpct(0.05f);

            GuiSetStyle(SLIDER, BASE_COLOR_NORMAL, 0x181827FF);
            GuiSetStyle(
                SLIDER,
                BASE_COLOR_PRESSED,
                ColorToInt(ColorBrightness(AccentColor, -0.25f))
            );
            GuiSetStyle(
                SLIDER,
                TEXT_COLOR_FOCUSED,
                ColorToInt(ColorBrightness(AccentColor, -0.5f))
            );
            GuiSetStyle(SLIDER, BORDER, 0xFFFFFFFF);
            GuiSetStyle(SLIDER, BORDER_COLOR_FOCUSED, 0xFFFFFFFF);
            GuiSetStyle(SLIDER, BORDER_WIDTH, 2);

            switch (displayedTab) {
            case MAIN: {
                // Main settings tab

                float trackSpeedFloat = settingsMain.trackSpeed;

                // header 1
                // calibration header
                int calibrationMenuOffset = 0;

                DrawRectangle(
                    u.wpct(0.005f),
                    underTabsHeight + (EntryHeight * calibrationMenuOffset),
                    OptionWidth * 2,
                    EntryHeight,
                    Color { 0, 0, 0, 128 }
                );
                DrawTextEx(
                    assets.rubikBoldItalic,
                    "Calibration",
                    { HeaderTextLeft,
                      OvershellBottom + u.hinpct(0.055f)
                          + (EntryHeight * calibrationMenuOffset) },
                    u.hinpct(0.04f),
                    0,
                    WHITE
                );

                // av offset

                settingsMain.avOffsetMS = sor.sliderEntry(
                    settingsMain.avOffsetMS,
                    -500.0f,
                    500.0f,
                    calibrationMenuOffset + 1,
                    "Audio/Visual Offset",
                    1
                );

                // input offset
                DrawRectangle(
                    u.wpct(0.005f),
                    underTabsHeight + (EntryHeight * (calibrationMenuOffset + 2)),
                    OptionWidth * 2,
                    EntryHeight,
                    Color { 0, 0, 0, 64 }
                );
                settingsMain.inputOffsetMS = sor.sliderEntry(
                    settingsMain.inputOffsetMS,
                    -500.0f,
                    500.0f,
                    calibrationMenuOffset + 2,
                    "Input Offset",
                    1
                );

                float calibrationTop =
                    EntryTop + (EntryHeight * (calibrationMenuOffset + 2));
                float calibrationTextTop =
                    EntryTextTop + (EntryHeight * (calibrationMenuOffset + 2));
                DrawTextEx(
                    assets.rubikBold,
                    "Automatic Calibration",
                    { EntryTextLeft, calibrationTextTop },
                    EntryFontSize,
                    0,
                    WHITE
                );
                if (GuiButton(
                        { OptionLeft, calibrationTop, OptionWidth, EntryHeight },
                        "Start Calibration"
                    )) {
                    menu.SwitchScreen(CALIBRATION);
                }

                int generalOffset = 4;
                // general header
                DrawRectangle(
                    u.wpct(0.005f),
                    underTabsHeight + (EntryHeight * generalOffset),
                    OptionWidth * 2,
                    EntryHeight,
                    Color { 0, 0, 0, 128 }
                );
                DrawTextEx(
                    assets.rubikBoldItalic,
                    "General",
                    { HeaderTextLeft,
                      OvershellBottom + u.hinpct(0.055f)
                          + (EntryHeight * generalOffset) },
                    u.hinpct(0.04f),
                    0,
                    WHITE
                );

                // fullscreen

                settingsMain.fullscreen = sor.toggleEntry(
                    settingsMain.fullscreen, generalOffset + 1, "Fullscreen"
                );

                DrawRectangle(
                    u.wpct(0.005f),
                    underTabsHeight + (EntryHeight * (generalOffset + 2)),
                    OptionWidth * 2,
                    EntryHeight,
                    Color { 0, 0, 0, 64 }
                );

                float scanTop = EntryTop + (EntryHeight * (generalOffset + 1));
                float scanTextTop = EntryTextTop + (EntryHeight * (generalOffset + 1));
                DrawTextEx(
                    assets.rubikBold,
                    "Scan Songs",
                    { EntryTextLeft, scanTextTop },
                    EntryFontSize,
                    0,
                    WHITE
                );
                if (GuiButton({ OptionLeft, scanTop, OptionWidth, EntryHeight }, "Scan")) {
                    menu.songsLoaded = false;
                    songList.ScanSongs(settingsMain.songPaths);
                }

                break;
            }
            case HIGHWAY: {
                DrawRectangle(
                    u.wpct(0.005f),
                    OvershellBottom + u.hinpct(0.05f),
                    OptionWidth * 2,
                    EntryHeight,
                    Color { 0, 0, 0, 128 }
                );
                DrawTextEx(
                    assets.rubikBoldItalic,
                    "Highway",
                    { HeaderTextLeft, OvershellBottom + u.hinpct(0.055f) },
                    u.hinpct(0.04f),
                    0,
                    WHITE
                );
                settingsMain.trackSpeed = sor.sliderEntry(
                    settingsMain.trackSpeed,
                    0,
                    settingsMain.trackSpeedOptions.size() - 1,
                    1,
                    "Track Speed",
                    1.0f
                );
                // highway length

                DrawRectangle(
                    u.wpct(0.005f),
                    underTabsHeight + (EntryHeight * 2),
                    OptionWidth * 2,
                    EntryHeight,
                    Color { 0, 0, 0, 64 }
                );
                settingsMain.highwayLengthMult = sor.sliderEntry(
                    settingsMain.highwayLengthMult,
                    0.25f,
                    2.5f,
                    2,
                    "Highway Length Multiplier",
                    0.25f
                );

                // miss color
                settingsMain.missHighwayDefault = sor.toggleEntry(
                    settingsMain.missHighwayDefault, 3, "Highway Miss Color"
                );

                // lefty flip
                DrawRectangle(
                    u.wpct(0.005f),
                    underTabsHeight + (EntryHeight * 4),
                    OptionWidth * 2,
                    EntryHeight,
                    Color { 0, 0, 0, 64 }
                );
                settingsMain.mirrorMode =
                    sor.toggleEntry(settingsMain.mirrorMode, 4, "Mirror/Lefty Mode");

                menu.hehe = sor.toggleEntry(menu.hehe, 5, "Super Cool Highway Colors");

                // gpr.bot = sor.toggleEntry(gpr.bot, 6, "Bot");

                // gpr.showHitwindow = sor.toggleEntry(
                // gpr.showHitwindow, 7, "Show Hitwindow");
                break;
            }
            case VOLUME: {
                // audio tab
                DrawRectangle(
                    u.wpct(0.005f),
                    OvershellBottom + u.hinpct(0.05f),
                    OptionWidth * 2,
                    EntryHeight,
                    Color { 0, 0, 0, 128 }
                );
                DrawTextEx(
                    assets.rubikBoldItalic,
                    "Volume",
                    { HeaderTextLeft, OvershellBottom + u.hinpct(0.055f) },
                    u.hinpct(0.04f),
                    0,
                    WHITE
                );

                settingsMain.MainVolume = sor.sliderEntry(
                    settingsMain.MainVolume, 0, 1, 1, "Main Volume", 0.05f
                );

                settingsMain.PlayerVolume = sor.sliderEntry(
                    settingsMain.PlayerVolume, 0, 1, 2, "Player Volume", 0.05f
                );

                settingsMain.BandVolume = sor.sliderEntry(
                    settingsMain.BandVolume, 0, 1, 3, "Band Volume", 0.05f
                );

                settingsMain.SFXVolume =
                    sor.sliderEntry(settingsMain.SFXVolume, 0, 1, 4, "SFX Volume", 0.05f);

                settingsMain.MissVolume = sor.sliderEntry(
                    settingsMain.MissVolume, 0, 1, 5, "Miss Volume", 0.05f
                );

                settingsMain.MenuVolume = sor.sliderEntry(
                    settingsMain.MenuVolume, 0, 1, 6, "Menu Music Volume", 0.05f
                );

                // player.selInstVolume = settingsMain.MainVolume *
                // settingsMain.PlayerVolume; player.otherInstVolume =
                // settingsMain.MainVolume * settingsMain.BandVolume; player.sfxVolume =
                // settingsMain.MainVolume * settingsMain.SFXVolume; player.missVolume =
                // settingsMain.MainVolume * settingsMain.MissVolume;

                break;
            }
            case KEYBOARD: {
                // Keyboard bindings tab
                GuiToggleGroup(
                    { u.LeftSide + u.winpct(0.005f),
                      OvershellBottom + u.hinpct(0.05f),
                      (u.winpct(0.987f) / 4),
                      u.hinpct(0.05) },
                    "Pad Binds;Classic Binds;Misc Gameplay;Menu Binds",
                    &selectedKbTab
                );
                GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
                switch (displayedKbTab) {
                case kbPAD: {
                    for (int i = 0; i < 5; i++) {
                        sor.keybindEntryText(i + 2, "Lane " + to_string(i + 1));
                        sor.keybind5kEntry(
                            settingsMain.keybinds5K[i],
                            i + 2,
                            "Lane " + to_string(i + 1),
                            keybinds,
                            i
                        );
                        sor.keybind5kAltEntry(
                            settingsMain.keybinds5KAlt[i],
                            i + 2,
                            "Lane " + to_string(i + 1),
                            keybinds,
                            i
                        );
                    }
                    for (int i = 0; i < 4; i++) {
                        sor.keybindEntryText(i + 8, "Lane " + to_string(i + 1));
                        sor.keybind4kEntry(
                            settingsMain.keybinds4K[i],
                            i + 8,
                            "Lane " + to_string(i + 1),
                            keybinds,
                            i
                        );
                        sor.keybind4kAltEntry(
                            settingsMain.keybinds4KAlt[i],
                            i + 8,
                            "Lane " + to_string(i + 1),
                            keybinds,
                            i
                        );
                    }
                    GuiSetStyle(DEFAULT, TEXT_SIZE, 28);
                    break;
                }
                case kbCLASSIC: {
                    DrawRectangle(
                        u.wpct(0.005f),
                        underTabsHeight + (EntryHeight * 2),
                        OptionWidth * 2,
                        EntryHeight,
                        ColorAlpha(GREEN, 0.1f)
                    );
                    DrawRectangle(
                        u.wpct(0.005f),
                        underTabsHeight + (EntryHeight * 3),
                        OptionWidth * 2,
                        EntryHeight,
                        ColorAlpha(RED, 0.1f)
                    );
                    DrawRectangle(
                        u.wpct(0.005f),
                        underTabsHeight + (EntryHeight * 4),
                        OptionWidth * 2,
                        EntryHeight,
                        ColorAlpha(YELLOW, 0.1f)
                    );
                    DrawRectangle(
                        u.wpct(0.005f),
                        underTabsHeight + (EntryHeight * 5),
                        OptionWidth * 2,
                        EntryHeight,
                        ColorAlpha(BLUE, 0.1f)
                    );
                    DrawRectangle(
                        u.wpct(0.005f),
                        underTabsHeight + (EntryHeight * 6),
                        OptionWidth * 2,
                        EntryHeight,
                        ColorAlpha(ORANGE, 0.1f)
                    );
                    for (int i = 0; i < 5; i++) {
                        sor.keybindEntryText(i + 2, "Lane " + to_string(i + 1));
                        sor.keybind5kEntry(
                            settingsMain.keybinds5K[i],
                            i + 2,
                            "Lane " + to_string(i + 1),
                            keybinds,
                            i
                        );
                        sor.keybind5kAltEntry(
                            settingsMain.keybinds5KAlt[i],
                            i + 2,
                            "Lane " + to_string(i + 1),
                            keybinds,
                            i
                        );
                    }
                    sor.keybindEntryText(8, "Strum Up");
                    sor.keybindStrumEntry(0, 8, settingsMain.keybindStrumUp, keybinds);

                    sor.keybindEntryText(9, "Strum Down");
                    sor.keybindStrumEntry(1, 9, settingsMain.keybindStrumDown, keybinds);

                    break;
                }
                case kbMISC: {
                    sor.keybindEntryText(2, "Overdrive");
                    sor.keybindOdAltEntry(
                        settingsMain.keybindOverdriveAlt, 2, "Overdrive Alt", keybinds
                    );
                    sor.keybindOdEntry(
                        settingsMain.keybindOverdrive, 2, "Overdrive", keybinds
                    );

                    sor.keybindEntryText(3, "Pause Song");
                    sor.keybindPauseEntry(settingsMain.keybindPause, 3, "Pause", keybinds);
                    break;
                }
                case kbMENUS: {
                    break;
                }
                }
                if (sor.changingKey) {
                    std::vector<int> &bindsToChange = sor.changingAlt
                        ? (sor.changing4k ? settingsMain.keybinds4KAlt
                                          : settingsMain.keybinds5KAlt)
                        : (sor.changing4k ? settingsMain.keybinds4K
                                          : settingsMain.keybinds5K);
                    DrawRectangle(
                        0, 0, GetScreenWidth(), GetScreenHeight(), { 0, 0, 0, 200 }
                    );
                    std::string keyString = (sor.changing4k ? "4k" : "5k");
                    std::string altString = (sor.changingAlt ? " alt" : "");
                    std::string changeString =
                        "Press a key for " + keyString + altString + " lane ";
                    DrawTextRubik(
                        changeString.c_str(),
                        ((float)GetScreenWidth()
                         - MeasureTextRubik(changeString.c_str(), 20))
                            / 2,
                        (float)GetScreenHeight() / 2 - 30,
                        20,
                        WHITE
                    );
                    int pressedKey = GetKeyPressed();
                    if (GuiButton(
                            { ((float)GetScreenWidth() / 2) - 130,
                              GetScreenHeight() - 60.0f,
                              120,
                              40 },
                            "Unbind Key"
                        )) {
                        pressedKey = -2;
                    }
                    if (GuiButton(
                            { ((float)GetScreenWidth() / 2) + 10,
                              GetScreenHeight() - 60.0f,
                              120,
                              40 },
                            "Cancel"
                        )) {
                        sor.selLane = 0;
                        sor.changingKey = false;
                    }
                    if (pressedKey != 0) {
                        bindsToChange[sor.selLane] = pressedKey;
                        sor.selLane = 0;
                        sor.changingKey = false;
                    }
                }
                if (sor.changingOverdrive) {
                    DrawRectangle(
                        0, 0, GetScreenWidth(), GetScreenHeight(), { 0, 0, 0, 200 }
                    );
                    std::string altString = (sor.changingAlt ? " alt" : "");
                    std::string changeString =
                        "Press a key for " + altString + " overdrive";
                    DrawTextRubik(
                        changeString.c_str(),
                        ((float)GetScreenWidth()
                         - MeasureTextRubik(changeString.c_str(), 20))
                            / 2,
                        (float)GetScreenHeight() / 2 - 30,
                        20,
                        WHITE
                    );
                    int pressedKey = GetKeyPressed();
                    if (GuiButton(
                            { ((float)GetScreenWidth() / 2) - 130,
                              GetScreenHeight() - 60.0f,
                              120,
                              40 },
                            "Unbind Key"
                        )) {
                        pressedKey = -2;
                    }
                    if (GuiButton(
                            { ((float)GetScreenWidth() / 2) + 10,
                              GetScreenHeight() - 60.0f,
                              120,
                              40 },
                            "Cancel"
                        )) {
                        sor.changingAlt = false;
                        sor.changingOverdrive = false;
                    }
                    if (pressedKey != 0) {
                        if (sor.changingAlt)
                            settingsMain.keybindOverdriveAlt = pressedKey;
                        else
                            settingsMain.keybindOverdrive = pressedKey;
                        sor.changingOverdrive = false;
                    }
                }
                if (sor.changingPause) {
                    DrawRectangle(
                        0, 0, GetScreenWidth(), GetScreenHeight(), { 0, 0, 0, 200 }
                    );
                    DrawTextRubik(
                        "Press a key for Pause",
                        ((float)GetScreenWidth()
                         - MeasureTextRubik("Press a key for Pause", 20))
                            / 2,
                        (float)GetScreenHeight() / 2 - 30,
                        20,
                        WHITE
                    );
                    int pressedKey = GetKeyPressed();
                    if (GuiButton(
                            { ((float)GetScreenWidth() / 2) - 130,
                              GetScreenHeight() - 60.0f,
                              120,
                              40 },
                            "Unbind Key"
                        )) {
                        pressedKey = -2;
                    }
                    if (GuiButton(
                            { ((float)GetScreenWidth() / 2) + 10,
                              GetScreenHeight() - 60.0f,
                              120,
                              40 },
                            "Cancel"
                        )) {
                        sor.changingAlt = false;
                        sor.changingPause = false;
                    }
                    if (pressedKey != 0) {
                        settingsMain.keybindPause = pressedKey;
                        sor.changingPause = false;
                    }
                }
                if (sor.changingStrumUp) {
                    DrawRectangle(
                        0, 0, GetScreenWidth(), GetScreenHeight(), { 0, 0, 0, 200 }
                    );
                    DrawTextRubik(
                        "Press a key for Strum Up",
                        ((float)GetScreenWidth()
                         - MeasureTextRubik("Press a key for Strum Up", 20))
                            / 2,
                        (float)GetScreenHeight() / 2 - 30,
                        20,
                        WHITE
                    );
                    int pressedKey = GetKeyPressed();
                    if (GuiButton(
                            { ((float)GetScreenWidth() / 2) - 130,
                              GetScreenHeight() - 60.0f,
                              120,
                              40 },
                            "Unbind Key"
                        )) {
                        pressedKey = -2;
                    }
                    if (GuiButton(
                            { ((float)GetScreenWidth() / 2) + 10,
                              GetScreenHeight() - 60.0f,
                              120,
                              40 },
                            "Cancel"
                        )) {
                        sor.changingAlt = false;
                        sor.changingStrumUp = false;
                    }
                    if (pressedKey != 0) {
                        settingsMain.keybindStrumUp = pressedKey;
                        sor.changingStrumUp = false;
                    }
                }
                if (sor.changingStrumDown) {
                    DrawRectangle(
                        0, 0, GetScreenWidth(), GetScreenHeight(), { 0, 0, 0, 200 }
                    );
                    DrawTextRubik(
                        "Press a key for Strum Down",
                        ((float)GetScreenWidth()
                         - MeasureTextRubik("Press a key for Strum Down", 20))
                            / 2,
                        (float)GetScreenHeight() / 2 - 30,
                        20,
                        WHITE
                    );
                    int pressedKey = GetKeyPressed();
                    if (GuiButton(
                            { ((float)GetScreenWidth() / 2) - 130,
                              GetScreenHeight() - 60.0f,
                              120,
                              40 },
                            "Unbind Key"
                        )) {
                        pressedKey = -2;
                    }
                    if (GuiButton(
                            { ((float)GetScreenWidth() / 2) + 10,
                              GetScreenHeight() - 60.0f,
                              120,
                              40 },
                            "Cancel"
                        )) {
                        sor.changingAlt = false;
                        sor.changingStrumDown = false;
                    }
                    if (pressedKey != 0) {
                        settingsMain.keybindStrumDown = pressedKey;
                        sor.changingStrumDown = false;
                    }
                }
                break;
            }
            case GAMEPAD: { /*
                 //Controller bindings tab
                 GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
                 for (int i = 0; i < 5; i++) {
                     float j = (float) i - 2.0f;
                     if (GuiButton({
                                     ((float) GetScreenWidth() / 2) - 40 + (
                                         80 * j),
                                     240, 80, 60
                                 },
                                 keybinds.getControllerStr(
                                     controllerID,
                                     settingsMain.controller5K[i],
                                     settingsMain.controllerType,
                                     settingsMain.controller5KAxisDirection[
                                         i]).c_str())) {
                         changing4k = false;
                         selLane = i;
                         changingKey = true;
                     }
                 }
                 for (int i = 0; i < 4; i++) {
                     float j = (float) i - 1.5f;
                     if (GuiButton({
                                     ((float) GetScreenWidth() / 2) - 40 + (
                                         80 * j),
                                     360, 80, 60
                                 },
                                 keybinds.getControllerStr(
                                     controllerID,
                                     settingsMain.controller4K[i],
                                     settingsMain.controllerType,
                                     settingsMain.controller4KAxisDirection[
                                         i]).c_str())) {
                         changing4k = true;
                         selLane = i;
                         changingKey = true;
                     }
                 }
                 if (GuiButton({((float) GetScreenWidth() / 2) - 40, 480, 80, 60},
                             keybinds.getControllerStr(
                                 controllerID, settingsMain.controllerOverdrive,
                                 settingsMain.controllerType,
                                 settingsMain.controllerOverdriveAxisDirection).
                             c_str())) {
                     changingKey = false;
                     changingOverdrive = true;
                 }
                 if (GuiButton({((float) GetScreenWidth() / 2) - 40, 560, 80, 60},
                             keybinds.getControllerStr(
                                 controllerID, settingsMain.controllerPause,
                                 settingsMain.controllerType,
                                 settingsMain.controllerPauseAxisDirection).
                             c_str())) {
                     changingKey = false;
                     changingOverdrive = true;
                 }
                 if (changingKey) {
                     std::vector<int> &bindsToChange = (changing4k
                                                             ? settingsMain.
                                                             controller4K
                                                             : settingsMain.
                                                             controller5K);
                     std::vector<int> &directionToChange = (changing4k
                                                                 ? settingsMain.
                                                                 controller4KAxisDirection
                                                                 : settingsMain.
                                                                 controller5KAxisDirection);
                     DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                                 {0, 0, 0, 200});
                     std::string keyString = (changing4k ? "4k" : "5k");
                     std::string changeString =
                             "Press a button/axis for controller " +
                             keyString + " lane " +
                             std::to_string(selLane + 1);
                     DrawTextRubik(changeString.c_str(),
                                 ((float) GetScreenWidth() - MeasureTextRubik(
                                     changeString.c_str(), 20)) / 2,
                                 GetScreenHeight() / 2 - 30, 20, WHITE);
                     if (GuiButton({
                                     ((float) GetScreenWidth() / 2) - 60,
                                     GetScreenHeight() - 60.0f, 120, 40
                                 },
                                 "Cancel")) {
                         changingKey = false;
                     }
                     if (pressedGamepadInput != -999) {
                         bindsToChange[selLane] = pressedGamepadInput;
                         if (pressedGamepadInput < 0) {
                             directionToChange[selLane] = axisDirection;
                         }
                         selLane = 0;
                         changingKey = false;
                         pressedGamepadInput = -999;
                     }
                 }
                 if (changingOverdrive) {
                     DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                                 {0, 0, 0, 200});
                     std::string changeString =
                             "Press a button/axis for controller overdrive";
                     DrawTextRubik(changeString.c_str(),
                                 ((float) GetScreenWidth() - MeasureTextRubik(
                                     changeString.c_str(), 20)) / 2,
                                 GetScreenHeight() / 2 - 30, 20, WHITE);
                     if (GuiButton({
                                     ((float) GetScreenWidth() / 2) - 60,
                                     GetScreenHeight() - 60.0f, 120, 40
                                 },
                                 "Cancel")) {
                         changingOverdrive = false;
                     }
                     if (pressedGamepadInput != -999) {
                         settingsMain.controllerOverdrive = pressedGamepadInput;
                         if (pressedGamepadInput < 0) {
                             settingsMain.controllerOverdriveAxisDirection =
                                     axisDirection;
                         }
                         changingOverdrive = false;
                         pressedGamepadInput = -999;
                     }
                 }
                 if (changingPause) {
                     DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                                 {0, 0, 0, 200});
                     std::string changeString =
                             "Press a button/axis for controller pause";
                     DrawTextRubik(changeString.c_str(),
                                 ((float) GetScreenWidth() - MeasureTextRubik(
                                     changeString.c_str(), 20)) / 2,
                                 GetScreenHeight() / 2 - 30, 20, WHITE);
                     if (GuiButton({
                                     ((float) GetScreenWidth() / 2) - 60,
                                     GetScreenHeight() - 60.0f, 120, 40
                                 },
                                 "Cancel")) {
                         changingPause = false;
                     }
                     if (pressedGamepadInput != -999) {
                         settingsMain.controllerPause = pressedGamepadInput;
                         if (pressedGamepadInput < 0) {
                             settingsMain.controllerPauseAxisDirection =
                                     axisDirection;
                         }
                         changingPause = false;
                         pressedGamepadInput = -999;
                     }
                 }
                 GuiSetStyle(DEFAULT, TEXT_SIZE, 28);
                 break;*/
            }
            }
            break;
        }
        case SONG_SELECT: {
            if (!menu.songsLoaded) {
                songList.LoadCache(settingsMain.songPaths);
                menu.songsLoaded = true;
            }
            streamsLoaded = false;
            midiLoaded = false;
            isPlaying = false;
            // gpr.songEnded = false;
            //  player.overdrive = false;
            // gpr.curNoteIdx = {0, 0, 0, 0, 0};
            // gpr.curODPhrase = 0;
            // gpr.curNoteInt = 0;
            // gpr.curSolo = 0;
            // gpr.curBeatLine = 0;
            // gpr.curBPM = 0;

            // if (selSong)
            // gpr.selectedSongInt = curPlayingSong;
            // else
            // gpr.selectedSongInt = menu.ChosenSongInt;

            SetTextureWrap(songList.curSong->albumArtBlur, TEXTURE_WRAP_REPEAT);
            SetTextureFilter(
                songList.curSong->albumArtBlur, TEXTURE_FILTER_ANISOTROPIC_16X
            );
            // -5 -4 -3 -2 -1 0 1 2 3 4 5 6
            Vector2 mouseWheel = GetMouseWheelMoveV();
            int lastIntChosen = (int)mouseWheel.y;
            // set to specified height
            if (songSelectOffset <= songList.listMenuEntries.size()
                && songSelectOffset >= 1 && songList.listMenuEntries.size() >= 10) {
                songSelectOffset -= (int)mouseWheel.y;
            }

            // prevent going past top
            if (songSelectOffset < 1)
                songSelectOffset = 1;

            // prevent going past bottom
            if (songSelectOffset >= songList.listMenuEntries.size() - 10)
                songSelectOffset = songList.listMenuEntries.size() - 10;

            // todo(3drosalia): clean this shit up after changing it
            if (!albumArtLoaded) {
                // gpr.selectedSongInt = menu.ChosenSongInt;
                songList.curSong->LoadAlbumArt();
                if (!selSong)
                    songSelectOffset = songList.curSong->songListPos - 5;
                albumArtLoaded = true;
            }
            Song SongToDisplayInfo;
            BeginShaderMode(assets.bgShader);
            // todo(3drosalia): this too
            SongToDisplayInfo = *songList.curSong;
            if (songList.curSong->ini)
                songList.curSong->LoadInfoINI(songList.curSong->songInfoPath);
            else
                songList.curSong->LoadInfo(songList.curSong->songInfoPath);
            menu.DrawAlbumArtBackground(songList.curSong->albumArtBlur);
            EndShaderMode();

            float TopOvershell = u.hpct(0.15f);
            DrawRectangle(
                0,
                0,
                u.RightSide - u.LeftSide,
                (float)GetScreenHeight(),
                GetColor(0x00000080)
            );
            // DrawLineEx({u.LeftSide + u.winpct(0.0025f), 0}, {
            // 				u.LeftSide + u.winpct(0.0025f), (float) GetScreenHeight()
            // 			}, u.winpct(0.005f), WHITE);
            BeginScissorMode(
                0, u.hpct(0.15f), u.RightSide - u.winpct(0.25f), u.hinpct(0.7f)
            );
            menu.DrawTopOvershell(0.208333f);
            EndScissorMode();
            overshellRenderer.DrawTopOvershell(0.15f);

            menu.DrawVersion();
            int AlbumX = u.RightSide - u.winpct(0.25f);
            int AlbumY = u.hpct(0.075f);
            int AlbumHeight = u.winpct(0.25f);
            int AlbumOuter = u.hinpct(0.01f);
            int AlbumInner = u.hinpct(0.005f);
            int BorderBetweenAlbumStuff = (u.RightSide - u.LeftSide) - u.winpct(0.25f);

            DrawTextEx(
                assets.josefinSansItalic,
                TextFormat("Sorted by: %s", sortTypes[(int)currentSortValue].c_str()),
                { u.LeftSide, u.hinpct(0.165f) },
                u.hinpct(0.03f),
                0,
                WHITE
            );
            DrawTextEx(
                assets.josefinSansItalic,
                TextFormat("Songs loaded: %01i", songList.songs.size()),
                { AlbumX - (AlbumOuter * 2)
                      - MeasureTextEx(
                            assets.josefinSansItalic,
                            TextFormat("Songs loaded: %01i", songList.songs.size()),
                            u.hinpct(0.03f),
                            0
                      )
                            .x,
                  u.hinpct(0.165f) },
                u.hinpct(0.03f),
                0,
                WHITE
            );

            float songEntryHeight = u.hinpct(0.058333f);
            DrawRectangle(
                0,
                u.hinpct(0.208333f),
                (u.RightSide - u.winpct(0.25f)),
                songEntryHeight,
                ColorBrightness(AccentColor, -0.75f)
            );

            for (int j = 0; j < 5; j++) {
                DrawRectangle(
                    0,
                    ((songEntryHeight * 2) * j) + u.hinpct(0.208333f) + songEntryHeight,
                    (u.RightSide - u.winpct(0.25f)),
                    songEntryHeight,
                    Color { 0, 0, 0, 64 }
                );
            }

            for (int i = songSelectOffset;
                 i < songList.listMenuEntries.size() && i < songSelectOffset + 10;
                 i++) {
                if (songList.listMenuEntries.size() == i)
                    break;
                if (songList.listMenuEntries[i].isHeader) {
                    float songXPos = u.LeftSide + u.winpct(0.005f) - 2;
                    float songYPos = std::floor(
                        (u.hpct(0.266666f))
                        + ((songEntryHeight) * ((i - songSelectOffset)))
                    );
                    DrawRectangle(
                        0,
                        songYPos,
                        (u.RightSide - u.winpct(0.25f)),
                        songEntryHeight,
                        ColorBrightness(AccentColor, -0.75f)
                    );

                    DrawTextEx(
                        assets.rubikBold,
                        songList.listMenuEntries[i].headerChar.c_str(),
                        { songXPos, songYPos + u.hinpct(0.0125f) },
                        u.hinpct(0.035f),
                        0,
                        WHITE
                    );
                } else if (!songList.listMenuEntries[i].hiddenEntry) {
                    bool isCurSong = i == songList.curSong->songListPos - 1;
                    Font &artistFont =
                        isCurSong ? assets.josefinSansItalic : assets.josefinSansItalic;
                    Song &songi = songList.songs[songList.listMenuEntries[i].songListID];
                    int songID = songList.listMenuEntries[i].songListID;

                    // float buttonX =
                    // ((float)GetScreenWidth()/2)-(((float)GetScreenWidth()*0.86f)/2);
                    // LerpState state = lerpCtrl.createLerp("SONGSELECT_LERP_" +
                    // std::to_string(i), EaseOutCirc, 0.4f);
                    float songXPos = u.LeftSide + u.winpct(0.005f) - 2;
                    float songYPos = std::floor(
                        (u.hpct(0.266666f))
                        + ((songEntryHeight) * ((i - songSelectOffset)))
                    );
                    GuiSetStyle(BUTTON, BORDER_WIDTH, 0);
                    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0);
                    if (isCurSong) {
                        GuiSetStyle(
                            BUTTON,
                            BASE_COLOR_NORMAL,
                            ColorToInt(ColorBrightness(AccentColor, -0.4))
                        );
                    }
                    BeginScissorMode(
                        0, u.hpct(0.15f), u.RightSide - u.winpct(0.25f), u.hinpct(0.7f)
                    );
                    if (GuiButton(
                            Rectangle { 0,
                                        songYPos,
                                        (u.RightSide - u.winpct(0.25f)),
                                        songEntryHeight },
                            ""
                        )) {
                        // curPlayingSong = songID;
                        selSong = true;
                        albumArtSelectedAndLoaded = false;
                        albumArtLoaded = false;
                        songList.curSong = &songList.songs[songID];
                    }
                    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
                    EndScissorMode();
                    // DrawTexturePro(song.albumArt, Rectangle{
                    // songXPos,0,(float)song.albumArt.width,(float)song.albumArt.height
                    // }, { songXPos+5,songYPos + 5,50,50 }, Vector2{ 0,0 }, 0.0f,
                    // RAYWHITE);
                    int songTitleWidth = (u.winpct(0.3f)) - 6;

                    int songArtistWidth = (u.winpct(0.5f)) - 6;

                    if (songi.titleTextWidth >= (float)songTitleWidth) {
                        if (curTime > songi.titleScrollTime
                            && curTime < songi.titleScrollTime + 3.0)
                            songi.titleXOffset = 0;

                        if (curTime > songi.titleScrollTime + 3.0) {
                            songi.titleXOffset -= 1;

                            if (songi.titleXOffset
                                < -(songi.titleTextWidth - (float)songTitleWidth)) {
                                songi.titleXOffset =
                                    -(songi.titleTextWidth - (float)songTitleWidth);
                                songi.titleScrollTime = curTime + 3.0;
                            }
                        }
                    }
                    auto LightText = Color { 203, 203, 203, 255 };
                    BeginScissorMode(
                        (int)songXPos + (isCurSong ? 5 : 20),
                        (int)songYPos,
                        songTitleWidth,
                        songEntryHeight
                    );
                    DrawTextEx(
                        assets.rubikBold,
                        songi.title.c_str(),
                        { songXPos + songi.titleXOffset + (isCurSong ? 10 : 20),
                          songYPos + u.hinpct(0.0125f) },
                        u.hinpct(0.035f),
                        0,
                        isCurSong ? WHITE : LightText
                    );
                    EndScissorMode();

                    if (songi.artistTextWidth > (float)songArtistWidth) {
                        if (curTime > songi.artistScrollTime
                            && curTime < songi.artistScrollTime + 3.0)
                            songi.artistXOffset = 0;

                        if (curTime > songi.artistScrollTime + 3.0) {
                            songi.artistXOffset -= 1;
                            if (songi.artistXOffset
                                < -(songi.artistTextWidth - (float)songArtistWidth)) {
                                songi.artistScrollTime = curTime + 3.0;
                            }
                        }
                    }

                    auto SelectedText = WHITE;
                    BeginScissorMode(
                        (int)songXPos + 30 + (int)songTitleWidth,
                        (int)songYPos,
                        songArtistWidth,
                        songEntryHeight
                    );
                    DrawTextEx(
                        artistFont,
                        songi.artist.c_str(),
                        { songXPos + 30 + (float)songTitleWidth + songi.artistXOffset,
                          songYPos + u.hinpct(0.02f) },
                        u.hinpct(0.025f),
                        0,
                        isCurSong ? WHITE : LightText
                    );
                    EndScissorMode();
                }
            }

            DrawRectangle(
                AlbumX - AlbumOuter,
                AlbumY + AlbumHeight,
                AlbumHeight + AlbumOuter,
                AlbumHeight + u.hinpct(0.01f),
                WHITE
            );
            DrawRectangle(
                AlbumX - AlbumInner,
                AlbumY + AlbumHeight,
                AlbumHeight,
                u.hinpct(0.075f) + AlbumHeight,
                GetColor(0x181827FF)
            );
            DrawRectangle(
                AlbumX - AlbumOuter,
                AlbumY - AlbumInner,
                AlbumHeight + AlbumOuter,
                AlbumHeight + AlbumOuter,
                WHITE
            );
            DrawRectangle(AlbumX - AlbumInner, AlbumY, AlbumHeight, AlbumHeight, BLACK);
            // TODO: replace this with actual sorting/category hiding
            if (songSelectOffset > 0) {
                std::string SongTitleForCharThingyThatsTemporary =
                    songList.listMenuEntries[songSelectOffset].headerChar;
                switch (currentSortValue) {
                case SortType::Title: {
                    if (songList.listMenuEntries[songSelectOffset].isHeader) {
                        SongTitleForCharThingyThatsTemporary =
                            songList
                                .songs[songList.listMenuEntries[songSelectOffset - 1]
                                           .songListID]
                                .title[0];
                    } else {
                        SongTitleForCharThingyThatsTemporary =
                            songList
                                .songs[songList.listMenuEntries[songSelectOffset]
                                           .songListID]
                                .title[0];
                    }
                    break;
                }
                case SortType::Artist: {
                    if (songList.listMenuEntries[songSelectOffset].isHeader) {
                        SongTitleForCharThingyThatsTemporary =
                            songList
                                .songs[songList.listMenuEntries[songSelectOffset - 1]
                                           .songListID]
                                .artist[0];
                    } else {
                        SongTitleForCharThingyThatsTemporary =
                            songList
                                .songs[songList.listMenuEntries[songSelectOffset]
                                           .songListID]
                                .artist[0];
                    }
                    break;
                }
                case SortType::Length: {
                    if (songList.listMenuEntries[songSelectOffset].isHeader) {
                        SongTitleForCharThingyThatsTemporary = std::to_string(
                            songList
                                .songs[songList.listMenuEntries[songSelectOffset - 1]
                                           .songListID]
                                .length
                        );
                    } else {
                        SongTitleForCharThingyThatsTemporary = std::to_string(
                            songList
                                .songs[songList.listMenuEntries[songSelectOffset]
                                           .songListID]
                                .length
                        );
                    }
                    break;
                }
                }

                DrawTextEx(
                    assets.rubikBold,
                    SongTitleForCharThingyThatsTemporary.c_str(),
                    { u.LeftSide + 5, u.hpct(0.218333f) },
                    u.hinpct(0.035f),
                    0,
                    WHITE
                );
            }

            // bottom
            DrawTexturePro(
                songList.curSong->albumArt,
                Rectangle { 0,
                            0,
                            (float)songList.curSong->albumArt.width,
                            (float)songList.curSong->albumArt.width },
                Rectangle { (float)AlbumX - AlbumInner,
                            (float)AlbumY,
                            (float)AlbumHeight,
                            (float)AlbumHeight },
                { 0, 0 },
                0,
                WHITE
            );
            // hehe

            float TextPlacementTB = u.hpct(0.05f);
            float TextPlacementLR = u.LeftSide;
            GameMenu::mhDrawText(
                assets.redHatDisplayBlack,
                "MUSIC LIBRARY",
                { TextPlacementLR, TextPlacementTB },
                u.hinpct(0.125f),
                WHITE,
                assets.sdfShader,
                LEFT
            );

            std::string AlbumArtText = SongToDisplayInfo.album.empty()
                ? "No Album Listed"
                : SongToDisplayInfo.album;

            float AlbumTextHeight =
                MeasureTextEx(assets.rubikBold, AlbumArtText.c_str(), u.hinpct(0.035f), 0)
                    .y;
            float AlbumTextWidth =
                MeasureTextEx(assets.rubikBold, AlbumArtText.c_str(), u.hinpct(0.035f), 0)
                    .x;
            float AlbumNameTextCenter = u.RightSide - u.winpct(0.125f) - AlbumInner;
            float AlbumTTop = AlbumY + AlbumHeight + u.hinpct(0.011f);
            float AlbumNameFontSize = AlbumTextWidth <= u.winpct(0.25f)
                ? u.hinpct(0.035f)
                : u.winpct(0.23f) / (AlbumTextWidth / AlbumTextHeight);
            float AlbumNameLeft = AlbumNameTextCenter
                - (MeasureTextEx(
                       assets.rubikBold, AlbumArtText.c_str(), AlbumNameFontSize, 0
                   )
                       .x
                   / 2);
            float AlbumNameTextTop = AlbumTextWidth <= u.winpct(0.25f)
                ? AlbumTTop
                : AlbumTTop + ((u.hinpct(0.035f) / 2) - (AlbumNameFontSize / 2));

            DrawTextEx(
                assets.rubikBold,
                AlbumArtText.c_str(),
                { AlbumNameLeft, AlbumNameTextTop },
                AlbumNameFontSize,
                0,
                WHITE
            );

            DrawLine(
                u.RightSide - AlbumHeight - AlbumOuter,
                AlbumY + AlbumHeight + AlbumOuter + (u.hinpct(0.04f)),
                u.RightSide,
                AlbumY + AlbumHeight + AlbumOuter + (u.hinpct(0.04f)),
                WHITE
            );

            float DiffTop = AlbumY + AlbumHeight + AlbumOuter + (u.hinpct(0.045f));

            float DiffHeight = u.hinpct(0.035f);
            float DiffTextSize = u.hinpct(0.03f);
            float DiffDotLeft =
                u.RightSide - MeasureTextEx(assets.rubikBold, "OOOOO  ", DiffHeight, 0).x;
            float ScrollbarLeft =
                u.RightSide - AlbumHeight - AlbumOuter - (AlbumInner * 2);
            float ScrollbarTop = u.hpct(0.208333f);
            float ScrollbarHeight = GetScreenHeight() - u.hpct(0.208333f) - u.hpct(0.15f);
            GuiSetStyle(SCROLLBAR, BACKGROUND_COLOR, 0x181827FF);
            GuiSetStyle(SCROLLBAR, SCROLL_SLIDER_SIZE, u.hinpct(0.03f));
            songSelectOffset = GuiScrollBar(
                { ScrollbarLeft, ScrollbarTop, (float)(AlbumInner * 2), ScrollbarHeight },
                songSelectOffset,
                1,
                songList.listMenuEntries.size() - 10
            );
            for (int i = 0; i < 7; i++) {
                float IconWidth = (float)AlbumHeight / 5.0f;
                float BoxTopPos = DiffTop + (float)(IconWidth * (i < 4 ? 0 : 1));
                float ResetToLeftPos = (float)(i > 3 ? i - 4 : i);
                int asdasd = (float)(i > 3 ? i - 4 : i);
                float IconLeftPos =
                    (float)(u.RightSide - AlbumHeight) + IconWidth * ResetToLeftPos;
                Rectangle Placement = { IconLeftPos, BoxTopPos, IconWidth, IconWidth };
                Color TintColor = WHITE;
                if (SongToDisplayInfo.parts[i]->diff == -1)
                    TintColor = DARKGRAY;
                DrawTexturePro(
                    assets.InstIcons[asdasd],
                    { 0,
                      0,
                      (float)assets.InstIcons[asdasd].width,
                      (float)assets.InstIcons[asdasd].height },
                    Placement,
                    { 0, 0 },
                    0,
                    TintColor
                );
                DrawTexturePro(
                    assets.BaseRingTexture,
                    { 0,
                      0,
                      (float)assets.BaseRingTexture.width,
                      (float)assets.BaseRingTexture.height },
                    Placement,
                    { 0, 0 },
                    0,
                    ColorBrightness(WHITE, 2)
                );
                if (SongToDisplayInfo.parts[i]->diff > 0)
                    DrawTexturePro(
                        assets.YargRings[SongToDisplayInfo.parts[i]->diff - 1],
                        { 0,
                          0,
                          (float)assets.YargRings[SongToDisplayInfo.parts[i]->diff - 1]
                              .width,
                          (float)assets.YargRings[SongToDisplayInfo.parts[i]->diff - 1]
                              .height },
                        Placement,
                        { 0, 0 },
                        0,
                        WHITE
                    );

                /*
                if (SongToDisplayInfo.parts[i]->diff != -1) {
                    std::string DiffDot;
                    bool red = false;
                    if (SongToDisplayInfo.parts[i]->diff == 6) red = true;
                    for (int g = 0; g < 7; g++) {
                        if (g < SongToDisplayInfo.parts[i]->diff - 1)
                            DiffDot += "O";
                        if (g > SongToDisplayInfo.parts[i]->diff - 1) {
                            DiffDot += " ";
                        }
                    }
                    DrawTextEx(assets.rubikBold, "OOOOO",
                                {DiffDotLeft, DiffTop + (DiffHeight * i)}, DiffHeight, 0,
                                DARKGRAY);
                    DrawTextEx(assets.rubikBold, DiffDot.c_str(),
                                {DiffDotLeft, DiffTop + (DiffHeight * i)}, DiffHeight, 0,
                                red ? RED : WHITE);
                } else {
                    DrawTextEx(assets.rubikBold, "N/A",
                                {DiffDotLeft, DiffTop + (DiffHeight * i)}, DiffHeight, 0,
                GRAY);
                }
                DrawTextEx(assets.rubik,
                            songPartsList[i].c_str(), {
                                u.RightSide - AlbumHeight + AlbumInner,
                                DiffTop + u.hinpct(0.0025f) + (DiffHeight * i)
                            }, DiffTextSize, 0, WHITE);
                            */
            }

            menu.DrawBottomOvershell();
            float BottomOvershell = (float)GetScreenHeight() - 120;

            GuiSetStyle(
                BUTTON, BASE_COLOR_NORMAL, ColorToInt(ColorBrightness(AccentColor, -0.25))
            );
            if (GuiButton(
                    Rectangle { u.LeftSide,
                                GetScreenHeight() - u.hpct(0.1475f),
                                u.winpct(0.2f),
                                u.hinpct(0.05f) },
                    "Play Song"
                )) {
                // curPlayingSong = menu.ChosenSongInt;
                if (!songList.curSong->ini) {
                    songList.curSong->LoadSong(songList.curSong->songInfoPath);
                } else {
                    songList.curSong->LoadSongIni(songList.curSong->songDir);
                }
                menu.SwitchScreen(READY_UP);
                albumArtLoaded = false;
            }
            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);

            if (GuiButton(
                    Rectangle { u.LeftSide + u.winpct(0.4f) - 2,
                                GetScreenHeight() - u.hpct(0.1475f),
                                u.winpct(0.2f),
                                u.hinpct(0.05f) },
                    "Sort"
                )) {
                currentSortValue = NextSortType(currentSortValue);
                songList.sortList(currentSortValue, songList.curSong->songListPos);
            }
            if (GuiButton(
                    Rectangle { u.LeftSide + u.winpct(0.2f) - 1,
                                GetScreenHeight() - u.hpct(0.1475f),
                                u.winpct(0.2f),
                                u.hinpct(0.05f) },
                    "Back"
                )) {
                for (Song &songi : songList.songs) {
                    songi.titleXOffset = 0;
                    songi.artistXOffset = 0;
                }
                albumArtLoaded = false;
                menu.albumArtLoaded = false;
                menu.songsLoaded = true;
                menu.songChosen = false;
                selSong = false;
                menu.SwitchScreen(MENU);
            }
            overshellRenderer.DrawBottomOvershell();
            break;
        }
        case READY_UP: {
            if (!albumArtLoaded) {
                // selectedSong = songList.songs[curPlayingSong];
                songList.curSong->LoadAlbumArt();
                albumArtLoaded = true;
            }
            SetTextureWrap(songList.curSong->albumArtBlur, TEXTURE_WRAP_REPEAT);
            SetTextureFilter(
                songList.curSong->albumArtBlur, TEXTURE_FILTER_ANISOTROPIC_16X
            );
            menu.DrawAlbumArtBackground(songList.curSong->albumArtBlur);

            float AlbumArtLeft = u.LeftSide;
            float AlbumArtTop = u.hpct(0.05f);
            float AlbumArtRight = u.winpct(0.15f);
            float AlbumArtBottom = u.winpct(0.15f);
            DrawRectangle(
                0, 0, (int)GetScreenWidth(), (int)GetScreenHeight(), GetColor(0x00000080)
            );

            overshellRenderer.DrawTopOvershell(0.2f);
            menu.DrawVersion();

            DrawRectangle(
                (int)u.LeftSide,
                (int)AlbumArtTop,
                (int)AlbumArtRight + 12,
                (int)AlbumArtBottom + 12,
                WHITE
            );
            DrawRectangle(
                (int)u.LeftSide + 6,
                (int)AlbumArtTop + 6,
                (int)AlbumArtRight,
                (int)AlbumArtBottom,
                BLACK
            );
            DrawTexturePro(
                songList.curSong->albumArt,
                Rectangle { 0,
                            0,
                            (float)songList.curSong->albumArt.width,
                            (float)songList.curSong->albumArt.width },
                Rectangle {
                    u.LeftSide + 6, AlbumArtTop + 6, AlbumArtRight, AlbumArtBottom },
                { 0, 0 },
                0,
                WHITE
            );

            float BottomOvershell = u.hpct(1) - u.hinpct(0.15f);
            float TextPlacementTB = AlbumArtTop;
            float TextPlacementLR = AlbumArtRight + AlbumArtLeft + 32;
            DrawTextEx(
                assets.rubikBoldItalic,
                songList.curSong->title.c_str(),
                { TextPlacementLR, TextPlacementTB - 5 },
                u.hinpct(LargeHeader),
                0,
                WHITE
            );
            DrawTextEx(
                assets.rubik,
                songList.curSong->artist.c_str(),
                { TextPlacementLR, TextPlacementTB + u.hinpct(LargeHeader) },
                u.hinpct(MediumHeader),
                0,
                LIGHTGRAY
            );
            // todo: allow this to be run per player
            // load midi
            menu.DrawBottomOvershell();
            for (int playerInt = 0; playerInt < 4; playerInt++) {
                if (playerManager.ActivePlayers[playerInt] == -1)
                    continue;
                Player *player = playerManager.GetActivePlayer(playerInt);
                if (!midiLoaded && !songList.curSong->midiParsed) {
                    smf::MidiFile midiFile;
                    midiFile.read(songList.curSong->midiPath.string());
                    for (int track = 0; track < midiFile.getTrackCount(); track++) {
                        SongParts songPart = GetSongPart(midiFile[track]);
                        IsPartValid(midiFile[track], songPart);
                    }

                    songList.curSong->midiParsed = true;
                    midiLoaded = true;

                    if (!player->ReadiedUpBefore
                        || !songList.curSong->parts[player->Instrument]->hasPart) {
                        player->instSelection = true;
                    } else if (songList.curSong->parts[player->Instrument]
                                   ->charts[player->Difficulty]
                                   .valid) {
                        player->diffSelection = true;
                    } else if (player->ReadiedUpBefore) {
                        player->ReadyUpMenu = true;
                    }
                }

                // load instrument select

                else if (midiLoaded && player->instSelection) {
                    if (GuiButton({ 0, 0, 60, 60 }, "<")) {
                        if (!player->ReadiedUpBefore
                            || !songList.curSong->parts[player->Instrument]->hasPart) {
                            player->instSelection = true;
                            player->diffSelection = false;
                            player->instSelected = false;
                            player->diffSelected = false;
                            midiLoaded = false;
                            albumArtSelectedAndLoaded = false;
                            albumArtLoaded = false;
                            menu.SwitchScreen(SONG_SELECT);
                        } else {
                            player->instSelection = false;
                            player->ReadyUpMenu = true;
                        }
                    }
                    // DrawTextRHDI(TextFormat("%s - %s",
                    // songList.curSong->title.c_str(),
                    // songList.curSong->artist.c_str()), 70,7, WHITE);

                    for (int i = 0; i < songList.curSong->parts.size(); i++) {
                        if (songList.curSong->parts[i]->hasPart) {
                            GuiSetStyle(
                                BUTTON,
                                BASE_COLOR_NORMAL,
                                i == player->Instrument && player->instSelected
                                    ? ColorToInt(
                                          ColorBrightness(player->AccentColor, -0.25)
                                      )
                                    : 0x181827FF
                            );
                            GuiSetStyle(
                                BUTTON,
                                TEXT_COLOR_NORMAL,
                                ColorToInt(Color { 255, 255, 255, 255 })
                            );
                            GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
                            if (GuiButton(
                                    { u.LeftSide + ((playerInt)*u.winpct(0.25f)),
                                      BottomOvershell - u.hinpct(0.05f)
                                          - (u.hinpct(0.05f) * (float)i),
                                      u.winpct(0.2f),
                                      u.hinpct(0.05f) },
                                    TextFormat("  %s", songPartsList[i].c_str())
                                )) {
                                player->instSelected = true;
                                player->Instrument = i;
                                int isBassOrVocal = 0;
                                if (i > 3)
                                    player->ClassicMode = true;
                                else
                                    player->ClassicMode = false;
                                if (player->Instrument == PAD_BASS
                                    || player->Instrument == PAD_VOCALS
                                    || player->Instrument == PLASTIC_BASS) {
                                    isBassOrVocal = 1;
                                }
                                SetShaderValue(
                                    assets.odMultShader,
                                    assets.isBassOrVocalLoc,
                                    &isBassOrVocal,
                                    SHADER_UNIFORM_INT
                                );
                            }
                            GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0xcbcbcbFF);
                            GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
                            DrawTextRubik(
                                (std::to_string(songList.curSong->parts[i]->diff + 1)
                                 + "/7")
                                    .c_str(),
                                (u.LeftSide + ((playerInt)*u.winpct(0.25f)))
                                    + u.winpct(0.165f),
                                BottomOvershell - u.hinpct(0.04f)
                                    - (u.hinpct(0.05f) * (float)i),
                                u.hinpct(0.03f),
                                WHITE
                            );
                        } else {
                            GuiButton(
                                { (u.LeftSide + (playerInt)*u.winpct(0.25f)),
                                  BottomOvershell - u.hinpct(0.05f)
                                      - (u.hinpct(0.05f) * (float)i),
                                  u.winpct(0.2f),
                                  u.hinpct(0.05f) },
                                ""
                            );
                            DrawRectangle(
                                (u.LeftSide + ((playerInt)*u.winpct(0.25f))) + 2,
                                BottomOvershell - u.hinpct(0.05f)
                                    - (u.hinpct(0.05f) * (float)i) + 2,
                                u.winpct(0.2f) - 4,
                                u.hinpct(0.05f) - 4,
                                Color { 0, 0, 0, 128 }
                            );
                        }
                        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
                        if (player->instSelected) {
                            GuiSetStyle(
                                BUTTON,
                                TEXT_COLOR_NORMAL,
                                ColorToInt(Color { 255, 255, 255, 255 })
                            );
                            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x1D754AFF);
                            GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, 0x2AA86BFF);
                            if (GuiButton(
                                    { (u.LeftSide + ((playerInt)*u.winpct(0.25f))),
                                      BottomOvershell,
                                      u.winpct(0.2f),
                                      u.hinpct(0.05f) },
                                    "Done"
                                )) {
                                if (!player->ReadiedUpBefore
                                    || songList.curSong->parts[player->Instrument]
                                           ->charts[player->Difficulty]
                                           .notes.empty()) {
                                    player->instSelection = false;
                                    player->diffSelection = true;
                                } else {
                                    player->instSelection = false;
                                    player->ReadyUpMenu = true;
                                }
                            }
                            GuiSetStyle(
                                BUTTON,
                                BASE_COLOR_FOCUSED,
                                ColorToInt(ColorBrightness(player->AccentColor, -0.5))
                            );
                            GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0xcbcbcbFF);
                            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
                        }
                    }
                }
                // load difficulty select
                if (midiLoaded && player->diffSelection) {
                    for (int a = 0; a < 4; a++) {
                        if (songList.curSong->parts[player->Instrument]->charts[a].valid) {
                            GuiSetStyle(
                                BUTTON,
                                BASE_COLOR_NORMAL,
                                a == player->Difficulty && player->diffSelected
                                    ? ColorToInt(
                                          ColorBrightness(player->AccentColor, -0.25)
                                      )
                                    : 0x181827FF
                            );
                            if (GuiButton(
                                    { (u.LeftSide + ((playerInt)*u.winpct(0.25f))),
                                      BottomOvershell - u.hinpct(0.05f)
                                          - (u.hinpct(0.05f) * (float)a),
                                      u.winpct(0.2f),
                                      u.hinpct(0.05f) },
                                    diffList[songList.curSong->parts[player->Instrument]->charts[a].diff].c_str()
                                )) {
                                player->Difficulty = songList.curSong->parts[player->Instrument]->charts[a].diff;
                                player->diffSelected = true;
                            }
                        } else {
                            GuiButton(
                                { (u.LeftSide + ((playerInt)*u.winpct(0.25f))),
                                  BottomOvershell - u.hinpct(0.05f)
                                      - (u.hinpct(0.05f) * (float)a),
                                  u.winpct(0.2f),
                                  u.hinpct(0.05f) },
                                ""
                            );
                            DrawRectangle(
                                (u.LeftSide + ((playerInt)*u.winpct(0.25f))) + 2,
                                BottomOvershell + 2 - u.hinpct(0.05f)
                                    - (u.hinpct(0.05f) * (float)a),
                                u.winpct(0.2f) - 4,
                                u.hinpct(0.05f) - 4,
                                Color { 0, 0, 0, 128 }
                            );
                        }
                        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
                        if (player->diffSelected) {
                            GuiSetStyle(
                                BUTTON,
                                TEXT_COLOR_NORMAL,
                                ColorToInt(Color { 255, 255, 255, 255 })
                            );
                            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x1D754AFF);
                            GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, 0x2AA86BFF);
                            if (GuiButton(
                                    { (u.LeftSide + ((playerInt)*u.winpct(0.25f))),
                                      BottomOvershell - u.hinpct(0.25f),
                                      u.winpct(0.2f),
                                      u.hinpct(0.05f) },
                                    "Done"
                                )) {
                                player->diffSelection = false;
                                player->ReadyUpMenu = true;
                                player->ReadiedUpBefore = true;
                            }
                            GuiSetStyle(
                                BUTTON,
                                BASE_COLOR_FOCUSED,
                                ColorToInt(ColorBrightness(player->AccentColor, -0.5))
                            );
                            GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0xcbcbcbFF);
                            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
                        }
                        if (GuiButton({ 0, 0, 60, 60 }, "<")) {
                            if (player->ReadiedUpBefore
                                || !songList.curSong->parts[player->Instrument]->hasPart) {
                                player->instSelection = true;
                                player->diffSelection = false;
                                player->instSelected = false;
                                player->diffSelected = false;
                            } else {
                                player->instSelection = false;
                                player->diffSelection = false;
                                player->instSelected = false;
                                player->diffSelected = false;
                                player->ReadyUpMenu = true;
                            }
                        }
                    }
                }
                if (midiLoaded && player->ReadyUpMenu) {
                    if (GuiButton(
                            { (u.LeftSide + ((playerInt)*u.winpct(0.25f))),
                              BottomOvershell - u.hinpct(0.05f),
                              u.winpct(0.2f),
                              u.hinpct(0.05f) },
                            ""
                        )) {
                        player->ReadyUpMenu = false;
                        player->diffSelection = true;
                    }
                    DrawTextRubik(
                        "  Difficulty",
                        (u.LeftSide + ((playerInt)*u.winpct(0.25f))),
                        BottomOvershell - u.hinpct(0.04f),
                        u.hinpct(0.03f),
                        WHITE
                    );
                    DrawTextEx(
                        assets.rubikBold,
                        diffList[player->Difficulty].c_str(),
                        { (u.LeftSide + ((playerInt)*u.winpct(0.25f))) + u.winpct(0.19f)
                              - MeasureTextEx(
                                    assets.rubikBold,
                                    diffList[player->Difficulty].c_str(),
                                    u.hinpct(0.03f),
                                    0
                              )
                                    .x,
                          BottomOvershell - u.hinpct(0.04f) },
                        u.hinpct(0.03f),
                        0,
                        WHITE
                    );
                    if (GuiButton(
                            { (u.LeftSide + ((playerInt)*u.winpct(0.25f))),
                              BottomOvershell - u.hinpct(0.10f),
                              u.winpct(0.2f),
                              u.hinpct(0.05f) },
                            ""
                        )) {
                        player->ReadyUpMenu = false;
                        player->instSelection = true;
                    }
                    DrawTextRubik(
                        "  Instrument",
                        (u.LeftSide + ((playerInt)*u.winpct(0.25f))),
                        BottomOvershell - u.hinpct(0.09f),
                        u.hinpct(0.03f),
                        WHITE
                    );
                    DrawTextEx(
                        assets.rubikBold,
                        songPartsList[player->Instrument].c_str(),
                        { (u.LeftSide + ((playerInt)*u.winpct(0.25f))) + u.winpct(0.19f)
                              - MeasureTextEx(
                                    assets.rubikBold,
                                    songPartsList[player->Instrument].c_str(),
                                    u.hinpct(0.03f),
                                    0
                              )
                                    .x,
                          BottomOvershell - u.hinpct(0.09f) },
                        u.hinpct(0.03f),
                        0,
                        WHITE
                    );
                    GuiSetStyle(
                        BUTTON,
                        TEXT_COLOR_NORMAL,
                        ColorToInt(Color { 255, 255, 255, 255 })
                    );
                    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x1D754AFF);
                    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, 0x2AA86BFF);
                    if (GuiButton(
                            { (u.LeftSide + ((playerInt)*u.winpct(0.25f))),
                              BottomOvershell,
                              u.winpct(0.2f),
                              u.hinpct(0.05f) },
                            "Ready Up!"
                        )) {
                        player->instSelection = true;
                        player->ReadyUpMenu = false;
                        player->stats->Difficulty = player->Difficulty;
                        player->stats->Instrument = player->Instrument;
                        // gpr.highwayInAnimation = false;
                        // gpr.songStartTime = GetTime();
                        menu.SwitchScreen(CHART_LOADING_SCREEN);
                        glfwSetKeyCallback(glfwGetCurrentContext(), keyCallback);
                        glfwSetGamepadStateCallback(gamepadStateCallback);
                    }
                    GuiSetStyle(
                        BUTTON,
                        BASE_COLOR_FOCUSED,
                        ColorToInt(ColorBrightness(player->AccentColor, -0.5))
                    );
                    GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0xcbcbcbFF);
                    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
                }

                if (GuiButton({ 0, 0, 60, 60 }, "<")) {
                    midiLoaded = false;
                    songList.curSong->midiParsed = false;
                    menu.SwitchScreen(SONG_SELECT);
                }
            }
            overshellRenderer.DrawBottomOvershell();
            break;
        }
        case GAMEPLAY: {
            // IMAGE BACKGROUNDS??????
            ClearBackground(BLACK);

            if (IsWindowResized() || notes_tex.texture.width != GetScreenWidth()
                || notes_tex.texture.height != GetScreenHeight()) {
                UnloadRenderTexture(notes_tex);
                notes_tex = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
                GenTextureMipmaps(&notes_tex.texture);
                SetTextureFilter(notes_tex.texture, TEXTURE_FILTER_ANISOTROPIC_4X);

                UnloadRenderTexture(hud_tex);
                hud_tex = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
                GenTextureMipmaps(&hud_tex.texture);
                SetTextureFilter(hud_tex.texture, TEXTURE_FILTER_ANISOTROPIC_4X);

                UnloadRenderTexture(highway_tex);
                highway_tex = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
                GenTextureMipmaps(&highway_tex.texture);
                SetTextureFilter(highway_tex.texture, TEXTURE_FILTER_ANISOTROPIC_4X);

                UnloadRenderTexture(highwayStatus_tex);
                highwayStatus_tex =
                    LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
                GenTextureMipmaps(&highwayStatus_tex.texture);
                SetTextureFilter(highwayStatus_tex.texture, TEXTURE_FILTER_ANISOTROPIC_4X);

                UnloadRenderTexture(smasher_tex);
                smasher_tex = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
                GenTextureMipmaps(&smasher_tex.texture);
                SetTextureFilter(smasher_tex.texture, TEXTURE_FILTER_ANISOTROPIC_4X);
            }

            float scorePos = u.RightSide - u.hinpct(0.01f);
            float scoreY = u.hpct(0.15f);
            float starY = scoreY + u.hinpct(0.065f);
            float comboY = starY + u.hinpct(0.055f);
            if (!songAlbumArtLoadedGameplay) {
                songList.curSong->LoadAlbumArt();
                songAlbumArtLoadedGameplay = true;
            }

            menu.DrawAlbumArtBackground(songList.curSong->albumArtBlur);
            DrawRectangle(
                0, 0, GetScreenWidth(), GetScreenHeight(), Color { 0, 0, 0, 128 }
            );
            // DrawTextureEx(assets.songBackground, {0,0},0,
            // (float)GetScreenHeight()/assets.songBackground.height,WHITE);
            for (int i = 0; i < playerManager.PlayersActive; i++) {
                if (i == 0)
                    playerManager.BandStats.BaseScore =
                        songList.curSong
                            ->parts[playerManager.GetActivePlayer(i)->Instrument]
                            ->charts[playerManager.GetActivePlayer(i)->Difficulty]
                            .baseScore;
                else
                    playerManager.BandStats.BaseScore +=
                        songList.curSong
                            ->parts[playerManager.GetActivePlayer(i)->Instrument]
                            ->charts[playerManager.GetActivePlayer(i)->Difficulty]
                            .baseScore;
            }

            int starsval = playerManager.BandStats.Stars();
            float starPercent = (float)playerManager.BandStats.Score
                / (float)playerManager.BandStats.BaseScore;
            for (int i = 0; i < 5; i++) {
                bool firstStar = (i == 0);
                float starX = scorePos - u.hinpct(0.26) + (i * u.hinpct(0.0525));
                float starWH = u.hinpct(0.05);
                Rectangle emptyStarWH = {
                    0, 0, (float)assets.emptyStar.width, (float)assets.emptyStar.height
                };
                Rectangle starRect = { starX, starY, starWH, starWH };
                DrawTexturePro(
                    assets.emptyStar, emptyStarWH, starRect, { 0, 0 }, 0, WHITE
                );
                float yMaskPos = Remap(
                    starPercent,
                    firstStar ? 0 : playerManager.BandStats.xStarThreshold[i - 1],
                    playerManager.BandStats.xStarThreshold[i],
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
            if (starPercent >= playerManager.BandStats.xStarThreshold[4]
                && playerManager.BandStats.EligibleForGoldStars) {
                float starWH = u.hinpct(0.05);
                Rectangle emptyStarWH = {
                    0, 0, (float)assets.goldStar.width, (float)assets.goldStar.height
                };
                float yMaskPos = Remap(
                    starPercent,
                    playerManager.BandStats.xStarThreshold[4],
                    playerManager.BandStats.xStarThreshold[5],
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
                        playerManager.BandStats.GoldStars ? assets.goldStar
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
            // int totalScore =
            // 		player.score + player.sustainScoreBuffer[0] +
            // player.sustainScoreBuffer[ 			1] + player.sustainScoreBuffer[2] +
            // player.sustainScoreBuffer[3]
            // 		+ player.sustainScoreBuffer[4];

            int Score = 0;
            int Combo = 0;
            /*
            for (int player = 0; player < playerManager.PlayersActive; player++) {
                Score += playerManager.GetActivePlayer(player)->stats->Score;
                Combo += playerManager.GetActivePlayer(player)->stats->Combo;
            }
            playerManager.BandStats.Score = Score;
            playerManager.BandStats.Combo = Combo;
*/
            if (playerManager.PlayersActive > 1) {
                playerManager.BandStats.Multiplayer = true;
                for (int player = 0; player < playerManager.PlayersActive; player++) {
                    playerManager.GetActivePlayer(player)->stats->Multiplayer = true;
                }
            } else {
                playerManager.BandStats.Multiplayer = false;
                for (int player = 0; player < playerManager.PlayersActive; player++) {
                    playerManager.GetActivePlayer(player)->stats->Multiplayer = false;
                }
            }
            Rectangle scoreboxSrc {0,0, float(assets.Scorebox.width), float(assets.Scorebox.height)};
            float WidthOfScorebox = u.hinpct(0.28);
            // float scoreY = u.hpct(0.15f);
            float ScoreboxX = u.RightSide;
            float ScoreboxY =  u.hpct(0.1425f);
            float HeightOfScorebox = WidthOfScorebox / 4;
            Rectangle scoreboxDraw {ScoreboxX, ScoreboxY, WidthOfScorebox, HeightOfScorebox};
            DrawTexturePro(assets.Scorebox, scoreboxSrc, scoreboxDraw, {WidthOfScorebox,0}, 0, WHITE);
            Rectangle TimerboxSrc {0,0, float(assets.Timerbox.width), float(assets.Timerbox.height)};
            float WidthOfTimerbox = u.hinpct(0.14);
            // float scoreY = u.hpct(0.15f);
            float TimerboxX = u.RightSide;
            float TimerboxY =  u.hpct(0.1425f);
            float HeightOfTimerbox = WidthOfTimerbox / 4;
            Rectangle TimerboxDraw {TimerboxX, TimerboxY, WidthOfTimerbox, HeightOfTimerbox};
            DrawTexturePro(assets.Timerbox, TimerboxSrc, TimerboxDraw, {WidthOfTimerbox,HeightOfTimerbox}, 0, WHITE);
            int played = enctime.GetSongTime();
            int length = enctime.GetSongLength();
            float Width = Remap(played, 0, length, 0, WidthOfTimerbox);
            BeginScissorMode(TimerboxX-WidthOfTimerbox, TimerboxY-HeightOfTimerbox, Width, HeightOfTimerbox+1);
            DrawTexturePro(assets.TimerboxOutline, TimerboxSrc, TimerboxDraw, {WidthOfTimerbox,HeightOfTimerbox}, 0, WHITE);
            EndScissorMode();
            GameMenu::mhDrawText(
                assets.redHatDisplayItalicLarge,
                scoreCommaFormatter(playerManager.BandStats.Score),
                {
                    u.RightSide - u.winpct(0.015f),
                    scoreY
                },
                u.hinpct(0.053),
                Color { 107, 161, 222, 255 },
                assets.sdfShader,
                RIGHT
            );
            float bandMult = u.RightSide - WidthOfScorebox;
            GameMenu::mhDrawText(
                assets.redHatDisplayItalicLarge,
                TextFormat(
                    "%01ix",
                    playerManager.BandStats
                        .OverdriveMultiplier[playerManager.BandStats.PlayersInOverdrive]
                ),
                { bandMult, scoreY },
                u.hinpct(0.05),
                RAYWHITE,
                assets.sdfShader,
                RIGHT
            );

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
                            { u.RightSide - u.winpct(0.013f),
                              scoreY - u.hinpct(SmallHeader) },
                            u.hinpct(SmallHeader*0.66),
                            WHITE,
                            assets.sdfShader,
                            RIGHT
                        );
            if (!streamsLoaded) {
                audioManager.loadStreams(songList.curSong->stemsPath);

                for (auto &stream : audioManager.loadedStreams) {
                    // if ((player.plastic ? player.Instrument - 4 : player.Instrument) ==
                    //	stream.Instrument)
                    //	audioManager.SetAudioStreamVolume(
                    //		stream.handle,
                    //		player.mute
                    //			? player.missVolume
                    //			: settingsMain.MainVolume * settingsMain.
                    //			PlayerVolume);
                    // else
                    audioManager.SetAudioStreamVolume(
                        stream.handle, settingsMain.MainVolume * settingsMain.BandVolume
                    );
                }
                streamsLoaded = true;

                // player.resetPlayerStats();
            } else {
                for (auto &stream : audioManager.loadedStreams) {
                    // if ((player.plastic ? player.Instrument - 4 : player.Instrument) ==
                    //	stream.Instrument)
                    //	audioManager.SetAudioStreamVolume(
                    //		stream.handle,
                    //		player.mute
                    //			? player.missVolume
                    //			: settingsMain.MainVolume * settingsMain.
                    //			PlayerVolume);
                    // else
                    audioManager.SetAudioStreamVolume(
                        stream.handle, settingsMain.MainVolume * settingsMain.BandVolume
                    );
                }
                float songPlayed = audioManager.GetMusicTimePlayed();
                double songEnd = floor(audioManager.GetMusicTimeLength())
                        <= (songList.curSong->end <= 0 ? 0 : songList.curSong->end)
                    ? floor(audioManager.GetMusicTimeLength())
                    : songList.curSong->end - 0.01;
                if (enctime.SongComplete()) {
                    gpr.LowerHighway();
                }
                if (enctime.SongComplete()) {
                    glfwSetKeyCallback(glfwGetCurrentContext(), origKeyCallback);
                    glfwSetGamepadStateCallback(origGamepadCallback);
                    songList.curSong->LoadAlbumArt();
                    midiLoaded = false;
                    isPlaying = false;
                    gpr.highwayInAnimation = false;
                    gpr.songPlaying = false;
                    gpr.highwayLevel = 0;
                    enctime.Stop();
                    if (streamsLoaded) {
                        audioManager.unloadStreams();
                        streamsLoaded = false;
                    }
                    menu.SwitchScreen(RESULTS);
                    Encore::EncoreLog(
                        LOG_INFO, TextFormat("Song ended at at %f", songPlayed)
                    );
                    break;
                }
            }
            double songEnd = floor(audioManager.GetMusicTimeLength())
                    <= (songList.curSong->end <= 0 ? 0 : songList.curSong->end)
                ? floor(audioManager.GetMusicTimeLength())
                : songList.curSong->end - 0.01;
            // menu.DrawFPS(u.LeftSide, 0);

            int songPlayed = audioManager.GetMusicTimePlayed();
            double songFloat = audioManager.GetMusicTimePlayed();
            // player.notes = (int) songList.curSong->parts[player.instrument]->charts[
            //	player.diff].notes.size();
            for (int pnum = 0; pnum < playerManager.PlayersActive; pnum++) {
                switch (playerManager.PlayersActive) {
                case (1): {
                    gpr.cameraSel = 0;
                    gpr.renderPos = 0;
                    break;
                }
                case (2): {
                    if (pnum == 0) {
                        gpr.cameraSel = 1;
                        gpr.renderPos = GetScreenWidth() / 8;
                    } else {
                        gpr.cameraSel = 0;
                        gpr.renderPos = -GetScreenWidth() / 8;
                    }
                    break;
                }
                case (3): {
                    if (pnum == 0) {
                        gpr.cameraSel = 2;
                        gpr.renderPos = GetScreenWidth() / 4;
                    } else if (pnum == 1) {
                        gpr.cameraSel = 0;
                        gpr.renderPos = 0;
                    } else if (pnum == 2) {
                        gpr.cameraSel = 1;
                        gpr.renderPos = -GetScreenWidth() / 4;
                    }
                    break;
                }
                case (4): {
                    if (pnum == 0) {
                        gpr.cameraSel = 2;
                        gpr.renderPos = GetScreenWidth() / 4;
                    } else if (pnum == 1) {
                        gpr.cameraSel = 3;
                        gpr.renderPos = GetScreenWidth() / 12;
                    } else if (pnum == 2) {
                        gpr.cameraSel = 0;
                        gpr.renderPos = -GetScreenWidth() / 12;
                    } else if (pnum == 3) {
                        gpr.cameraSel = 1;
                        gpr.renderPos = -GetScreenWidth() / 4;
                    }
                    break;
                }
                }
                gpr.RenderGameplay(
                    playerManager.GetActivePlayer(pnum),
                    enctime.GetSongTime(),
                    *songList.curSong,
                    highway_tex,
                    hud_tex,
                    notes_tex,
                    highwayStatus_tex,
                    smasher_tex
                );
                float CenterPosForText =
                    GetWorldToScreen(
                        { 0, 0, 0 },
                        gpr.cameraVectors[playerManager.PlayersActive - 1][gpr.cameraSel]
                    )
                        .x;
                float fontSize = u.hinpct(0.025);
                float textWidth = MeasureTextEx(
                                      assets.rubikBold,
                                      playerManager.GetActivePlayer(pnum)->Name.c_str(),
                                      fontSize,
                                      0
                )
                                      .x;
                DrawTextEx(
                    assets.rubikBold,
                    playerManager.GetActivePlayer(pnum)->Name.c_str(),
                    { (CenterPosForText - (textWidth / 2)) - (gpr.renderPos),
                      GetScreenHeight() - u.hinpct(0.03) },
                    fontSize,
                    0,
                    WHITE
                );

                // DrawTextEx(assets.rubik,
                // playerManager.GetActivePlayer(pnum)->Name.c_str(), {u.wpct((pnum *
                // 0.33)+(0.33/2)),u.hpct(0.9)}, u.hinpct(0.05), 0, WHITE);
            }
            // gpr.cameraSel = 2;
            // gpr.renderPos = -GetScreenWidth()/4;
            // gpr.RenderGameplay(playerManager.GetActivePlayer(1), songFloat,
            // songList.songs[curPlayingSong], highway_tex, hud_tex, notes_tex,
            // highwayStatus_tex, smasher_tex);

            // gpr.cameraSel = 0;
            // gpr.renderPos = 0;
            // gpr.RenderGameplay(playerManager.GetActivePlayer(2), songFloat,
            // songList.songs[curPlayingSong], highway_tex, 					hud_tex,
            // notes_tex,
            // highwayStatus_tex, smasher_tex);

            float SongNameWidth = MeasureTextEx(
                                      assets.rubikBoldItalic,
                                      songList.curSong->title.c_str(),
                                      u.hinpct(MediumHeader),
                                      0
            )
                                      .x;
            float SongArtistWidth = MeasureTextEx(
                                        assets.rubikBoldItalic,
                                        songList.curSong->artist.c_str(),
                                        u.hinpct(SmallHeader),
                                        0
            )
                                        .x;

            float SongExtrasWidth = MeasureTextEx(
                                        assets.rubikBoldItalic,
                                        songList.curSong->artist.c_str(),
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
            if (curTime > enctime.GetStartTime() + 7.5
                && curTime < enctime.GetStartTime() + 7.5 + SongNameDuration) {
                double timeSinceStart = GetTime() - (enctime.GetStartTime() + 7.5);
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
                    static_cast<float>(getEasingFunction(EaseInOutBack)(
                        timeSinceStart / SongNameDuration
                    )),
                    0,
                    1.0,
                    35,
                    -SongNameWidth
                );
            } else if (curTime > enctime.GetStartTime() + 7.5 + SongNameDuration)
                SongNameAlpha = 0;

            if (curTime > enctime.GetStartTime() + 7.75
                && curTime < enctime.GetStartTime() + 7.75 + SongNameDuration) {
                double timeSinceStart = GetTime() - (enctime.GetStartTime() + 7.75);
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
                    static_cast<float>(getEasingFunction(EaseInOutBack)(
                        timeSinceStart / SongNameDuration
                    )),
                    0,
                    1.0,
                    35,
                    -SongArtistWidth
                );
            }
            if (curTime > enctime.GetStartTime() + 8
                && curTime < enctime.GetStartTime() + 8 + SongNameDuration) {
                double timeSinceStart = GetTime() - (enctime.GetStartTime() + 8);
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
                    static_cast<float>(getEasingFunction(EaseInOutCirc)(
                        timeSinceStart / SongNameDuration
                    )),
                    0,
                    1.0,
                    SongNameBackgroundWidth,
                    0
                );

                SongExtrasPosition = Remap(
                    static_cast<float>(getEasingFunction(EaseInOutBack)(
                        timeSinceStart / SongNameDuration
                    )),
                    0,
                    1.0,
                    35,
                    -SongExtrasWidth
                );
            }
            if (curTime < enctime.GetStartTime() + 7.75 + SongNameDuration) {
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
                    songList.curSong->title.c_str(),
                    { SongNamePosition, u.hpct(0.2f) },
                    u.hinpct(MediumHeader),
                    0,
                    Color { 255, 255, 255, SongNameAlpha }
                );
                DrawTextEx(
                    assets.rubikItalic,
                    songList.curSong->artist.c_str(),
                    { SongArtistPosition, u.hpct(0.2f + MediumHeader) },
                    u.hinpct(SmallHeader),
                    0,
                    Color { 200, 200, 200, SongArtistAlpha }
                );
                DrawTextEx(
                    assets.rubikItalic,
                    TextFormat(
                        "%s, %01i",
                        songList.curSong->charters[0].c_str(),
                        songList.curSong->releaseYear
                    ),
                    { SongExtrasPosition, u.hpct(0.2f + MediumHeader + SmallHeader) },
                    u.hinpct(SmallHeader),
                    0,
                    Color { 200, 200, 200, SongExtrasAlpha }
                );
            }

            int songLength;
            if (songList.curSong->end == 0)
                songLength = static_cast<int>(audioManager.GetMusicTimeLength());
            else
                songLength = static_cast<int>(songList.curSong->end);


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

            float floatSongLength = audioManager.GetMusicTimePlayed();

            if (!playerManager.BandStats.Multiplayer) {
                Player *player = playerManager.GetActivePlayer(0);
                PlayerGameplayStats *stats = player->stats;
                if (player->stats->Paused) {
                    DrawRectangle(
                        0, 0, GetScreenWidth(), GetScreenHeight(), Color { 0, 0, 0, 80 }
                    );
                    overshellRenderer.DrawTopOvershell(0.2f);
                    GuiSetStyle(DEFAULT, TEXT_SIZE, (int)u.hinpct(0.08f));
                    GuiSetFont(assets.redHatDisplayBlack);
                    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
                    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0xaaaaaaFF);
                    GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, 0xFFFFFFFF);
                    GuiSetStyle(BUTTON, BORDER_WIDTH, 0);
                    GuiSetStyle(BUTTON, BACKGROUND_COLOR, 0);
                    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x00000000);
                    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, 0x00000000);
                    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, 0x00000000);
                    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, 0x00000000);
                    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, 0x00000000);
                    GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, 0x00000000);
                    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0x00000000);

                    if (GuiButton(
                            { u.wpct(0.02f),
                              u.hpct(0.3f),
                              u.winpct(0.2f),
                              u.hinpct(0.08f) },
                            "Resume"
                        )) {
                        audioManager.unpauseStreams();
                        enctime.Resume();
                        stats->Paused = false;
                    }

                    if (GuiButton(
                            { u.wpct(0.02f),
                              u.hpct(0.39f),
                              u.winpct(0.2f),
                              u.hinpct(0.08f) },
                            "Restart"
                        )) {
                        enctime.Reset();
                        songList.curSong->parts[player->Instrument]
                            ->charts[player->Difficulty]
                            .restartNotes();

                        stats->Overdrive = false;
                        stats->overdriveFill = 0.0f;
                        stats->overdriveActiveFill = 0.0f;
                        stats->overdriveActiveTime = 0.0;
                        stats->overdriveActivateTime = 0.0f;
                        gpr.highwayInAnimation = false;
                        gpr.highwayInEndAnim = false;
                        gpr.songPlaying = false;
                        gpr.Restart = true;
                        stats->curODPhrase = 0;
                        stats->curNoteInt = 0;
                        stats->curSolo = 0;
                        stats->curNoteIdx = { 0, 0, 0, 0, 0 };
                        stats->curBeatLine = 0;
                        player->ResetGameplayStats();
                        assets.expertHighway.materials[0]
                            .maps[MATERIAL_MAP_ALBEDO]
                            .texture = assets.highwayTexture;
                        assets.emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture =
                            assets.highwayTexture;
                        assets.multBar.materials[0].maps[MATERIAL_MAP_EMISSION].texture =
                            assets.odMultFill;
                        assets.multCtr3.materials[0].maps[MATERIAL_MAP_EMISSION].texture =
                            assets.odMultFill;
                        assets.multCtr5.materials[0].maps[MATERIAL_MAP_EMISSION].texture =
                            assets.odMultFill;
                        assets.expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color =
                            DARKGRAY;
                        assets.emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color =
                            DARKGRAY;
                        stats->Paused = false;
                    }
                    if (GuiButton(
                            { u.wpct(0.02f),
                              u.hpct(0.48f),
                              u.winpct(0.2f),
                              u.hinpct(0.08f) },
                            "Drop Out"
                        )) {
                        glfwSetKeyCallback(glfwGetCurrentContext(), origKeyCallback);
                        glfwSetGamepadStateCallback(origGamepadCallback);
                        // notes =
                        // songList.curSong->parts[instrument]->charts[diff].notes.size();
                        // notes = songList.curSong->parts[instrument]->charts[diff];
                        menu.SwitchScreen(RESULTS);
                        songList.curSong->LoadAlbumArt();
                        stats->Overdrive = false;
                        stats->overdriveFill = 0.0f;
                        stats->overdriveActiveFill = 0.0f;
                        stats->overdriveActiveTime = 0.0;
                        stats->overdriveActivateTime = 0.0f;
                        stats->curNoteInt = 0;
                        stats->curODPhrase = 0;
                        stats->curSolo = 0;
                        stats->Paused = false;
                        midiLoaded = false;
                        isPlaying = false;
                        enctime.Reset();
                        songList.curSong->parts[player->Instrument]
                            ->charts[player->Difficulty]
                            .resetNotes();
                        audioManager.unloadStreams();
                        player->stats->Quit = true;
                        gpr.highwayInAnimation = false;
                        gpr.highwayInEndAnim = false;
                        gpr.songPlaying = false;
                        songAlbumArtLoadedGameplay = false;
                        GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, 0xFFFFFFFF);
                        GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, 0xFFFFFFFF);
                        GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, 0xFFFFFFFF);
                        GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0x505050ff);
                        GuiSetStyle(BUTTON, BORDER_WIDTH, 2);
                        GuiSetFont(assets.rubik);
                        GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
                        break;
                    }
                    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, 0xFFFFFFFF);
                    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, 0xFFFFFFFF);
                    GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, 0xFFFFFFFF);
                    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0x505050ff);
                    GuiSetStyle(BUTTON, BORDER_WIDTH, 2);
                    GuiSetFont(assets.rubik);
                    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
                    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
                    GuiSetStyle(
                        BUTTON,
                        BASE_COLOR_FOCUSED,
                        ColorToInt(ColorBrightness(Color { 255, 0, 255, 255 }, -0.5))
                    );
                    GuiSetStyle(
                        BUTTON,
                        BASE_COLOR_PRESSED,
                        ColorToInt(ColorBrightness(Color { 255, 0, 255, 255 }, -0.3))
                    );

                    DrawTextEx(
                        assets.rubikBoldItalic,
                        "PAUSED",
                        { u.wpct(0.02f), u.hpct(0.05f) },
                        u.hinpct(0.1f),
                        0,
                        WHITE
                    );

                    float SongFontSize = u.hinpct(0.03f);

                    const char *instDiffText = TextFormat(
                        "%s %s",
                        diffList[player->Difficulty].c_str(),
                        songPartsList[player->Instrument].c_str()
                    );

                    float TitleHeight = MeasureTextEx(
                                            assets.rubikBoldItalic,
                                            songList.curSong->title.c_str(),
                                            SongFontSize,
                                            0
                    )
                                            .y;
                    float TitleWidth = MeasureTextEx(
                                           assets.rubikBoldItalic,
                                           songList.curSong->title.c_str(),
                                           SongFontSize,
                                           0
                    )
                                           .x;
                    float ArtistHeight = MeasureTextEx(
                                             assets.rubikItalic,
                                             songList.curSong->artist.c_str(),
                                             SongFontSize,
                                             0
                    )
                                             .y;
                    float ArtistWidth = MeasureTextEx(
                                            assets.rubikItalic,
                                            songList.curSong->artist.c_str(),
                                            SongFontSize,
                                            0
                    )
                                            .x;
                    float InstDiffHeight =
                        MeasureTextEx(assets.rubikBold, instDiffText, SongFontSize, 0).y;
                    float InstDiffWidth =
                        MeasureTextEx(assets.rubikBold, instDiffText, SongFontSize, 0).x;

                    Vector2 SongTitleBox = { u.RightSide - TitleWidth - u.winpct(0.01f),
                                             u.hpct(0.1f) - (ArtistHeight / 2)
                                                 - (TitleHeight * 1.1f) };
                    Vector2 SongArtistBox = { u.RightSide - ArtistWidth - u.winpct(0.01f),
                                              u.hpct(0.1f) - (ArtistHeight / 2) };
                    Vector2 SongInstDiffBox = {
                        u.RightSide - InstDiffWidth - u.winpct(0.01f),
                        u.hpct(0.1f) + (ArtistHeight / 2) + (InstDiffHeight * 0.1f)
                    };

                    DrawTextEx(
                        assets.rubikBoldItalic,
                        songList.curSong->title.c_str(),
                        SongTitleBox,
                        SongFontSize,
                        0,
                        WHITE
                    );
                    DrawTextEx(
                        assets.rubikItalic,
                        songList.curSong->artist.c_str(),
                        SongArtistBox,
                        SongFontSize,
                        0,
                        WHITE
                    );
                    DrawTextEx(
                        assets.rubikBold,
                        instDiffText,
                        SongInstDiffBox,
                        SongFontSize,
                        0,
                        WHITE
                    );
                }
            }

            menu.DrawFPS(u.LeftSide, u.hpct(0.0025f) + u.hinpct(0.025f));
            menu.DrawVersion();


            // if (!gpr.bot)
            //	DrawTextEx(assets.rubikBold, TextFormat("%s", player.FC ? "FC" : ""),
            //				{5, GetScreenHeight() - u.hinpct(0.05f)}, u.hinpct(0.04), 0,
            //				GOLD);
            // if (gpr.bot)
            //	DrawTextEx(assets.rubikBold, "BOT",
            //				{5, GetScreenHeight() - u.hinpct(0.05f)}, u.hinpct(0.04), 0,
            //				SKYBLUE);
            // if (!gpr.bot)
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
            /*
            std::string ScriptDisplayString = "";
            lua.script_file("scripts/testing.lua");
            ScriptDisplayString = lua["TextDisplay"];
            DrawTextEx(assets.rubikBold, ScriptDisplayString.c_str(),
                                            {5, GetScreenHeight() - u.hinpct(0.1f)},
            u.hinpct(0.04), 0, GOLD);

            DrawRectangle(u.wpct(0.5f) - (u.winpct(0.12f) / 2), u.hpct(0.02f) -
            u.winpct(0.01f), u.winpct(0.12f), u.winpct(0.065f),DARKGRAY);

            for (int fretBox = 0; fretBox < gpr.heldFrets.size(); fretBox++) {
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

                DrawRectangle(u.wpct(0.5f) - leftInputBoxSize + (fretBox *
            u.winpct(0.02f)), u.hpct(0.02f), u.winpct(0.02f), u.winpct(0.02f),
                            gpr.heldFrets[fretBox] ? fretColor : GRAY);
            }
            DrawRectangle(u.wpct(0.5f) - ((5 * u.winpct(0.02f)) / 2),
                        u.hpct(0.02f) + u.winpct(0.025f), u.winpct(0.1f), u.winpct(0.01f),
                        gpr.upStrum ? WHITE : GRAY);
            DrawRectangle(u.wpct(0.5f) - ((5 * u.winpct(0.02f)) / 2),
                        u.hpct(0.02f) + u.winpct(0.035f), u.winpct(0.1f), u.winpct(0.01f),
                        gpr.downStrum ? WHITE : GRAY);
            */
            DrawTextEx(
                assets.rubik,
                TextFormat("song time: %f", enctime.GetSongTime()),
                { 0, u.hpct(0.5f) },
                u.hinpct(SmallHeader),
                0,
                WHITE
            );
            DrawTextEx(
                assets.rubik,
                TextFormat("audio time: %f", audioManager.GetMusicTimePlayed()),
                { 0, u.hpct(0.5f + SmallHeader) },
                u.hinpct(SmallHeader),
                0,
                WHITE
            );
            break;
        }
        case RESULTS: {
            ActiveMenu->Draw();
            break;
        }
        case CHART_LOADING_SCREEN: {
            ClearBackground(BLACK);
            if (StartLoading) {
                songList.curSong->LoadAlbumArt();
                std::thread ChartLoader(LoadCharts);
                ChartLoader.detach();
                StartLoading = false;
            }
            menu.DrawAlbumArtBackground(songList.curSong->albumArtBlur);
            overshellRenderer.DrawTopOvershell(0.15f);
            DrawTextEx(
                assets.redHatDisplayBlack,
                "LOADING...  ",
                { u.LeftSide, u.hpct(0.05f) },
                u.hinpct(0.125f),
                0,
                WHITE
            );
            float AfterLoadingTextPos =
                MeasureTextEx(
                    assets.redHatDisplayBlack, "LOADING...  ", u.hinpct(0.125f), 0
                )
                    .x;

            std::string LoadingPhrase = "";

            switch (LoadingState) {
            case BEATLINES: {
                LoadingPhrase = "Setting metronome";
                break;
            }
            case NOTE_PARSING: {
                LoadingPhrase = "Loading notes";
                break;
            }
            case NOTE_SORTING: {
                LoadingPhrase = "Cleaning up notes";
                break;
            }
            case PLASTIC_CALC: {
                LoadingPhrase = "Fixing up Classic";
                break;
            }
            case NOTE_MODIFIERS: {
                LoadingPhrase = "HOPO on, HOPO off";
                break;
            }
            case OVERDRIVE: {
                LoadingPhrase = "Gettin' your double points on";
                break;
            }
            case SOLOS: {
                LoadingPhrase = "Shuffling through solos";
                break;
            }
            case BASE_SCORE: {
                LoadingPhrase = "Calculating stars";
                break;
            }
            case EXTRA_PROCESSING: {
                LoadingPhrase = "Finishing touch-ups";
                break;
            }
            case READY: {
                LoadingPhrase = "Ready!";
                break;
            }
            default: {
                LoadingPhrase = "";
                break;
            }
            }

            DrawTextEx(
                assets.rubikBold,
                LoadingPhrase.c_str(),
                { u.LeftSide + AfterLoadingTextPos + u.winpct(0.02f), u.hpct(0.09f) },
                u.hinpct(0.05f),
                0,
                LIGHTGRAY
            );
            menu.DrawBottomOvershell();

            if (FinishedLoading) {
                gpr.LoadGameplayAssets();
                FinishedLoading = false;
                StartLoading = true;
                menu.SwitchScreen(GAMEPLAY);
            }
            break;
        }
        case SOUND_TEST:
            ActiveMenu->Draw();
        case CACHE_LOADING_SCREEN:
            ActiveMenu->Draw();
        }
        EndDrawing();

        if (!removeFPSLimit || menu.currentScreen != GAMEPLAY) {
            currentTime = GetTime();
            updateDrawTime = currentTime - previousTime;
            int Target = targetFPS;
            if (menu.currentScreen != GAMEPLAY)
                Target = menuFPS;

            if (Target > 0) {
                waitTime = (1.0f / (float)Target) - updateDrawTime;
                if (waitTime > 0.0) {
                    WaitTime((float)waitTime);
                    currentTime = GetTime();
                    deltaTime = (float)(currentTime - previousTime);
                }
            } else
                deltaTime = (float)updateDrawTime;

            previousTime = currentTime;
        }
    }
    CloseWindow();
    return 0;
}

#define RAYGUI_IMPLEMENTATION

#if defined(WIN32) && defined(NDEBUG)
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif
#include "raylib.h"
#include <vector>
#include <iostream>
#include <filesystem>
#include "song/song.h"
#include "song/songlist.h"
#include "game/arguments.h"
#include "game/utility.h"
#include "game/player.h"
#include "game/lerp.h"
#include "game/keybinds.h"
#include "game/settings.h"
#include "raygui.h"
#include <random>
#include "GLFW/glfw3.h"
#include "game/menus/gameMenu.h"
#include "game/assets.h"
#include "raymath.h"
#include "game/menus/uiUnits.h"
#include "game/menus/settingsOptionRenderer.h"
#include "game/timingvalues.h"
#include "game/gameplay/gameplayRenderer.h"

#include <thread>
#include <atomic>

Menu &menu = Menu::getInstance();
Player player = Player::getInstance();
Settings& settingsMain = Settings::getInstance();
AudioManager &audioManager = AudioManager::getInstance();


vector<std::string> ArgumentList::arguments;



#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH
#endif

#ifndef ENCORE_VERSION
#define ENCORE_VERSION
#endif


bool midiLoaded = false;
bool isPlaying = false;
bool streamsLoaded = false;
bool albumArtSelectedAndLoaded = false;

bool ShowHighwaySettings = true;
bool ShowCalibrationSettings = true;
bool ShowGeneralSettings = true;

bool ReadyUpMenu = false;
bool diffSelected = false;
bool diffSelection = false;
bool instSelection = false;
bool instSelected = false;

int curNoteIndex = 0;
int curPlayingSong = 0;
int selLane = 0;
bool selSong = false;
int songSelectOffset = 0;
bool changingKey = false;
bool changing4k = false;
bool changingOverdrive = false;
bool changingAlt = false;
bool changingPause = false;
double startedPlayingSong = 0.0;
Vector2 viewScroll = { 0,0 };
Rectangle view = { 0 };

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

std::string trackSpeedButton;

std::string encoreVersion = ENCORE_VERSION;
std::string commitHash = GIT_COMMIT_HASH;
bool overdriveHeld = false;
bool overdriveAltHeld = false;
bool overdriveHitAvailable = false;
bool overdriveLiftAvailable = false;
std::vector<bool> overdriveLanesHit{ false,false,false,false,false };
double overdriveHitTime = 0.0;
std::vector<int> lastHitLifts{-1, -1, -1, -1, -1};

gameplayRenderer gpr;

SongList &songList = SongList::getInstance();
Assets &assets = Assets::getInstance();
double lastAxesTime = 0.0;
std::vector<float> axesValues{0.0f,0.0f,0.0f,0.0f,0.0f,0.0f};
std::vector<int> buttonValues{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
std::vector<float> axesValues2{ 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f };
int pressedGamepadInput = -999;
int axisDirection = -1;
int controllerID = -1;

int currentSortValue = 0;
std::vector <std::string> sortTypes{ "Title","Artist","Length"};
static void DrawTextRubik(const char* text, float posX, float posY, float fontSize, Color color) {
    DrawTextEx(assets.rubik, text, { posX,posY }, fontSize, 0, color);
}
static void DrawTextRHDI(const char* text, float posX, float posY, float fontSize, Color color) {
    DrawTextEx(assets.redHatDisplayItalic, text, { posX,posY }, fontSize, 0, color);
}
static float MeasureTextRubik(const char* text, float fontSize) {
    return MeasureTextEx(assets.rubik, text, fontSize, 0).x;
}
static float MeasureTextRHDI(const char* text, float fontSize) {
    return MeasureTextEx(assets.redHatDisplayItalic, text, fontSize, 0).x;
}

template<typename CharT>
struct Separators : public std::numpunct<CharT>
{
    [[nodiscard]] std::string do_grouping()
    const override
    {
        return "\003";
    }
};

std::string scoreCommaFormatter(int value) {
    std::stringstream ss;
    ss.imbue(std::locale(std::cout.getloc(), new Separators <char>()));
    ss << std::fixed << value;
    return ss.str();
}

double StrumNoFretTime = 0.0;

bool FAS = false;
int strummedNote = 0;
int FASNote = 0;

static void handleInputs(int lane, int action){
    if (player.paused) return;
    if (lane == -2) return;
    if (settingsMain.mirrorMode && lane != -1 && !player.plastic) {
        lane = (player.diff == 3 ? 4 : 3) - lane;
    }
    if (!streamsLoaded) {
        return;
    }
    Chart& curChart = songList.songs[curPlayingSong].parts[player.instrument]->charts[player.diff];
    float eventTime = audioManager.GetMusicTimePlayed(audioManager.loadedStreams[0].handle);
    if (player.instrument != 4){
        if (action == GLFW_PRESS && (lane == -1) && player.overdriveFill > 0 && !player.overdrive) {
            player.overdriveActiveTime = eventTime;
            player.overdriveActiveFill = player.overdriveFill;
            player.overdrive = true;
            overdriveHitAvailable = true;
            overdriveHitTime = eventTime;
        }

        if (!player.plastic) {
            if (lane == -1) {
                if ((action == GLFW_PRESS && !overdriveHitAvailable) ||
                    (action == GLFW_RELEASE && !overdriveLiftAvailable))
                    return;
                Note *curNote = &curChart.notes[0];
                for (auto &note: curChart.notes) {
                    if (note.isGood(eventTime, player.InputOffset) &&
                        !note.hit) {
                        curNote = &note;
                        break;
                    }
                }
                if (action == GLFW_PRESS && !overdriveHeld) {
                    overdriveHeld = true;
                } else if (action == GLFW_RELEASE && overdriveHeld) {
                    overdriveHeld = false;
                }
                if (action == GLFW_PRESS && overdriveHitAvailable) {
                    if (curNote->isGood(eventTime, player.InputOffset) &&
                        !curNote->hit) {
                        for (int newlane = 0; newlane < 5; newlane++) {
                            int chordLane = curChart.findNoteIdx(curNote->time, newlane);
                            if (chordLane != -1) {
                                Note &chordNote = curChart.notes[chordLane];
                                if (!chordNote.accounted) {
                                    chordNote.hit = true;
                                    overdriveLanesHit[newlane] = true;
                                    chordNote.hitTime = eventTime;

                                    if ((chordNote.len) > 0 && !chordNote.lift) {
                                        chordNote.held = true;
                                    }
                                    if (chordNote.isPerfect(eventTime, player.InputOffset)) {
                                        chordNote.perfect = true;

                                    }
                                    if (chordNote.perfect) player.lastNotePerfect = true;
                                    else player.lastNotePerfect = false;
                                    chordNote.HitOffset = chordNote.time - eventTime;
                                    player.HitNote(chordNote.perfect, player.instrument);
                                    chordNote.accounted = true;
                                }
                            }
                        }
                        overdriveHitAvailable = false;
                        overdriveLiftAvailable = true;
                    }
                } else if (action == GLFW_RELEASE && overdriveLiftAvailable) {
                    if (curNote->isGood(eventTime, player.InputOffset) &&
                        !curNote->hit) {
                        for (int newlane = 0; newlane < 5; newlane++) {
                            if (overdriveLanesHit[newlane]) {
                                int chordLane = curChart.findNoteIdx(curNote->time, newlane);
                                if (chordLane != -1) {
                                    Note &chordNote = curChart.notes[chordLane];
                                    if (chordNote.lift) {
                                        chordNote.hit = true;
                                        chordNote.HitOffset = chordNote.time - eventTime;
                                        overdriveLanesHit[newlane] = false;
                                        chordNote.hitTime = eventTime;

                                        if (chordNote.isPerfect(eventTime, player.InputOffset)) {
                                            chordNote.perfect = true;

                                        }
                                        chordNote.accounted = true;
                                        if (chordNote.perfect) player.lastNotePerfect = true;
                                        else player.lastNotePerfect = false;
                                    }

                                }
                            }
                        }
                        overdriveLiftAvailable = false;
                    }
                }
                if (action == GLFW_RELEASE && curNote->held && (curNote->len) > 0 && overdriveLiftAvailable) {
                    for (int newlane = 0; newlane < 5; newlane++) {
                        if (overdriveLanesHit[newlane]) {
                            int chordLane = curChart.findNoteIdx(curNote->time, newlane);
                            if (chordLane != -1) {
                                Note &chordNote = curChart.notes[chordLane];
                                if (chordNote.held && chordNote.len > 0) {
                                    if (!((player.diff == 3 && settingsMain.keybinds5K[chordNote.lane]) ||
                                          (player.diff != 3 && settingsMain.keybinds4K[chordNote.lane]))) {
                                        chordNote.held = false;
                                        player.score += player.sustainScoreBuffer[chordNote.lane];
                                        player.sustainScoreBuffer[chordNote.lane] = 0;
                                        player.mute = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else {
                for (int i = gpr.curNoteIdx[lane]; i < curChart.notes_perlane[lane].size(); i++) {
                    Note &curNote = curChart.notes[curChart.notes_perlane[lane][i]];



                    if (lane != curNote.lane) continue;
                    if (!player.plastic) {
                        if ((curNote.lift && action == GLFW_RELEASE) || action == GLFW_PRESS) {
                            if (curNote.isGood(eventTime, player.InputOffset) &&
                                !curNote.hit) {
                                if (curNote.lift && action == GLFW_RELEASE) {
                                    lastHitLifts[lane] = curChart.notes_perlane[lane][i];
                                }
                                curNote.hit = true;
                                curNote.HitOffset = curNote.time - eventTime;
                                curNote.hitTime = eventTime;
                                if ((curNote.len) > 0 && !curNote.lift) {
                                    curNote.held = true;
                                }
                                if (curNote.isPerfect(eventTime, player.InputOffset)) {
                                    curNote.perfect = true;
                                }
                                if (curNote.perfect) player.lastNotePerfect = true;
                                else player.lastNotePerfect = false;
                                player.HitNote(curNote.perfect, player.instrument);
                                curNote.accounted = true;
                                break;
                            }
                            if (curNote.miss) player.lastNotePerfect = false;
                        }
                        if ((!gpr.heldFrets[curNote.lane] && !gpr.heldFretsAlt[curNote.lane]) && curNote.held &&
                            (curNote.len) > 0) {
                            curNote.held = false;
                            player.score += player.sustainScoreBuffer[curNote.lane];
                            player.sustainScoreBuffer[curNote.lane] = 0;
                            player.mute = true;
                            // SetAudioStreamVolume(audioManager.loadedStreams[instrument].stream, missVolume);
                        }

                        if (action == GLFW_PRESS &&
                            eventTime > songList.songs[curPlayingSong].music_start &&
                            !curNote.hit &&
                            !curNote.accounted &&
                            ((curNote.time) - goodBackend) + player.InputOffset > eventTime &&
                            eventTime > overdriveHitTime + 0.05
                            && !gpr.overhitFrets[lane]) {
                            if (lastHitLifts[lane] != -1) {
                                if (eventTime > curChart.notes[lastHitLifts[lane]].time - 0.1 &&
                                    eventTime < curChart.notes[lastHitLifts[lane]].time + 0.1)
                                    continue;
                            }
                            player.OverHit();
                            if (!curChart.odPhrases.empty() && eventTime >= curChart.odPhrases[gpr.curODPhrase].start &&
                                eventTime < curChart.odPhrases[gpr.curODPhrase].end &&
                                !curChart.odPhrases[gpr.curODPhrase].missed)
                                curChart.odPhrases[gpr.curODPhrase].missed = true;
                            gpr.overhitFrets[lane] = true;
                        }
                    }
                }
            }
        }
        else {
            if (gpr.curNoteInt >= curChart.notes.size())
                gpr.curNoteInt = curChart.notes.size() - 1;
            player.notes = gpr.curNoteInt;
            Note &curNote = curChart.notes[gpr.curNoteInt];
            int pressedMask = 0b000000;

            for (int pressedButtons = 0; pressedButtons < gpr.heldFrets.size(); pressedButtons++) {
                if (gpr.heldFrets[pressedButtons] || gpr.heldFretsAlt[pressedButtons])
                    pressedMask += curChart.PlasticFrets[pressedButtons];
            }

            HeldMaskShow = pressedMask;
            Note &lastNote = curChart.notes[gpr.curNoteInt == 0 ? 0 : gpr.curNoteInt - 1];

            // if (!lastNote.accounted && gpr.curNoteInt != 0) return;

            bool firstNote = gpr.curNoteInt == 0;

            if (lane == 8008135 && action == GLFW_PRESS && !gpr.FAS
            // && (firstNote ? true : lastNote.time + 0.005 < eventTime)
                ) {
                StrumNoFretTime = eventTime;
                curNote.strumCount++;
                if (curNote.isGood(eventTime, player.InputOffset) && !curNote.hit && !curNote.hitWithFAS
                    // && (firstNote ? true : lastNote.accounted)
                    ) {
                    TraceLog(LOG_INFO, TextFormat("FAS Active at %f", eventTime));
                    gpr.FAS = true;
                    curNote.hitWithFAS = true;
                    // if (gpr.curNoteInt < curChart.notes.size() - 1) {
                    //     curChart.notes[gpr.curNoteInt + 1].hitWithFAS = false;
                    //    curChart.notes[gpr.curNoteInt + 1].strumCount = 0;
                    // }
                    strummedNote = gpr.curNoteInt;
                }
                if ((!curNote.isGood(eventTime, player.InputOffset))
                &&
                ((lastNote.phopo && lastNote.hit) ? (eventTime > lastNote.time + 0.05) : (true))) {
                    TraceLog(LOG_INFO, TextFormat("Overstrum at %f", eventTime));
                    gpr.overstrum = true;
                    gpr.FAS = false;
                    player.OverHit();
                    if (!curChart.odPhrases.empty() && !curChart.odPhrases[gpr.curODPhrase].missed &&
                        curNote.time >= curChart.odPhrases[gpr.curODPhrase].start &&
                        curNote.time < curChart.odPhrases[gpr.curODPhrase].end)
                        curChart.odPhrases[gpr.curODPhrase].missed = true;
                }
            } else if (lane == 8008135 && action == GLFW_RELEASE) {
                gpr.downStrum = false;
                gpr.upStrum = false;
                return;
            }
            if (StrumNoFretTime > eventTime + fretAfterStrumTime && gpr.FAS) {
                TraceLog(LOG_INFO, TextFormat("FAS Inactive at %f", eventTime));
                gpr.FAS = false;
            }


                bool chordMatch = (pressedMask == curNote.mask);
                bool singleMatch = (pressedMask >= curNote.mask && pressedMask < (curNote.mask * 2));
                bool noteMatch = (curNote.chord ? chordMatch : singleMatch);

                if (curNote.hitWithFAS) {
                    if (noteMatch && !curNote.hit) {
                        TraceLog(LOG_INFO, TextFormat("Note hit at %f as a STRUM", eventTime));
                        gpr.FAS = false;
                        curNote.hit = true;
                        curNote.HitOffset = curNote.time - eventTime;
                        curNote.hitTime = eventTime;
                        if (curNote.isPerfect(eventTime, player.InputOffset)) {
                            curNote.perfect = true;
                        }
                        player.HitPlasticNote(curNote, player.instrument);
                        curNote.accounted = true;
                        if ((curNote.len) > 0) {
                            curNote.held = true;
                        }
                        gpr.curNoteInt++;
                        return;
                    }
                }


                if (noteMatch && curNote.phopo && (player.combo > 0 || gpr.curNoteInt == 0)) {
                    if (curNote.isGood(eventTime, player.InputOffset) && !curNote.hit && !curNote.accounted) {
                        TraceLog(LOG_INFO, TextFormat("Note hit at %f as a HOPO", eventTime));
                        curNote.hit = true;
                        curNote.HitOffset = curNote.time - eventTime;
                        curNote.hitTime = eventTime;

                        if ((curNote.len) > 0) {
                            curNote.held = true;
                        }
                        if (curNote.isPerfect(eventTime, player.InputOffset)) {
                            curNote.perfect = true;
                        }
                        player.HitNote(curNote.perfect, player.instrument);
                        curNote.accounted = true;
                        gpr.curNoteInt++;
                        return;
                    }
                }
            
        }
    }
}

// what to check when a key changes states (what was the change? was it pressed? or released? what time? what window? were any modifiers pressed?)
static void keyCallback(GLFWwindow* wind, int key, int scancode, int action, int mods) {
	if (!streamsLoaded) {
		return;
	}
	if (action < 2) {  // if the key action is NOT repeat (release is 0, press is 1)
		int lane = -2;
        if (key == settingsMain.keybindPause && action==GLFW_PRESS) {
            player.paused = !player.paused;
            if(player.paused)
                audioManager.pauseStreams();
            else {
                audioManager.playStreams();
                for (int i = 0; i < (player.diff == 3 ? 5 : 4); i++) {
                    handleInputs(i, -1);
                }
            }

        }
        else if (key == settingsMain.keybindOverdrive || key == settingsMain.keybindOverdriveAlt) {
			handleInputs(-1, action);
		}
        else {
            if (player.instrument != 4) {
                if (player.diff == 3) {
                    for (int i = 0; i < 5; i++) {
                        if (key == settingsMain.keybinds5K[i] && !gpr.heldFretsAlt[i]) {
                            if (action == GLFW_PRESS) {
                                gpr.heldFrets[i] = true;
                            } else if (action == GLFW_RELEASE) {
                                gpr.heldFrets[i] = false;
                                gpr.overhitFrets[i] = false;
                            }
                            lane = i;
                        } else if (key == settingsMain.keybinds5KAlt[i] && !gpr.heldFrets[i]) {
                            if (action == GLFW_PRESS) {
                                gpr.heldFretsAlt[i] = true;
                            } else if (action == GLFW_RELEASE) {
                                gpr.heldFretsAlt[i] = false;
                                gpr.overhitFrets[i] = false;
                            }
                            lane = i;
                        }
                    }
                } else {
                    for (int i = 0; i < 4; i++) {
                        if (key == settingsMain.keybinds4K[i] && !gpr.heldFretsAlt[i]) {
                            if (action == GLFW_PRESS) {
                                gpr.heldFrets[i] = true;
                            } else if (action == GLFW_RELEASE) {
                                gpr.heldFrets[i] = false;
                                gpr.overhitFrets[i] = false;
                            }
                            lane = i;
                        } else if (key == settingsMain.keybinds4KAlt[i] && !gpr.heldFrets[i]) {
                            if (action == GLFW_PRESS) {
                                gpr.heldFretsAlt[i] = true;
                            } else if (action == GLFW_RELEASE) {
                                gpr.heldFretsAlt[i] = false;
                                gpr.overhitFrets[i] = false;
                            }
                            lane = i;
                        }
                    }
                }
                if (player.plastic) {
                    if (key == settingsMain.keybindStrumUp) {
                        if (action == GLFW_PRESS) {
                            lane = 8008135;
                            gpr.upStrum = true;
                        }
                        else if (action == GLFW_RELEASE) {
                            gpr.upStrum = false;
                            gpr.overstrum = false;
                        }
                    }
                    if (key == settingsMain.keybindStrumDown) {
                        if (action == GLFW_PRESS) {
                            lane = 8008135;
                            gpr.downStrum = true;
                        }
                        else if (action == GLFW_RELEASE) {
                            gpr.downStrum = false;
                            gpr.overstrum = false;
                        }
                    }
                }
               
                if (lane != -1 && lane != -2) {
                    handleInputs(lane, action);
                }
            }
		}
	}
}

static void gamepadStateCallback(int jid, GLFWgamepadstate state) {
	if (!streamsLoaded) {
		return;
	}
	double eventTime = audioManager.GetMusicTimePlayed(audioManager.loadedStreams[0].handle);
    if (settingsMain.controllerPause >= 0) {
        if (state.buttons[settingsMain.controllerPause] != buttonValues[settingsMain.controllerPause]) {
            buttonValues[settingsMain.controllerPause] = state.buttons[settingsMain.controllerPause];
            if (state.buttons[settingsMain.controllerPause] == 1) {
                player.paused = !player.paused;
                if (player.paused)
                    audioManager.pauseStreams();
                else {
                    audioManager.playStreams();
                    for (int i = 0; i < (player.diff == 3 ? 5 : 4); i++) {
                        handleInputs(i, -1);
                    }
                }
            }
        }
    }
    else {
        if (state.axes[-(settingsMain.controllerPause + 1)] != axesValues[-(settingsMain.controllerPause + 1)]) {
            axesValues[-(settingsMain.controllerPause + 1)] = state.axes[-(settingsMain.controllerPause + 1)];
            if (state.axes[-(settingsMain.controllerPause + 1)] == 1.0f * (float)settingsMain.controllerPauseAxisDirection) {


            }
        }
    }
	if (settingsMain.controllerOverdrive >= 0) {
		if (state.buttons[settingsMain.controllerOverdrive] != buttonValues[settingsMain.controllerOverdrive]) {
			buttonValues[settingsMain.controllerOverdrive] = state.buttons[settingsMain.controllerOverdrive];
			handleInputs(-1, state.buttons[settingsMain.controllerOverdrive]);
		}
	}
	else {
		if (state.axes[-(settingsMain.controllerOverdrive+1)] != axesValues[-(settingsMain.controllerOverdrive + 1)]) {
			axesValues[-(settingsMain.controllerOverdrive + 1)] = state.axes[-(settingsMain.controllerOverdrive + 1)];
			if (state.axes[-(settingsMain.controllerOverdrive + 1)] == 1.0f*(float)settingsMain.controllerOverdriveAxisDirection) {
				handleInputs(-1, GLFW_PRESS);
			}
			else {
				handleInputs(-1, GLFW_RELEASE);
			}
		}
	}
	if (player.diff == 3 || player.plastic) {
        int lane = -2;
        int action = -2;
		for (int i = 0; i < 5; i++) {
			if (settingsMain.controller5K[i] >= 0) {
				if (state.buttons[settingsMain.controller5K[i]] != buttonValues[settingsMain.controller5K[i]]) {
					if (state.buttons[settingsMain.controller5K[i]] == 1)
                        gpr.heldFrets[i] = true;
					else {
                        gpr.heldFrets[i] = false;
                        gpr.overhitFrets[i] = false;
					}
					handleInputs(i, state.buttons[settingsMain.controller5K[i]]);
					buttonValues[settingsMain.controller5K[i]] = state.buttons[settingsMain.controller5K[i]];
				    lane = i;
                }

			}
			else {
				if (state.axes[-(settingsMain.controller5K[i] + 1)] != axesValues[-(settingsMain.controller5K[i] + 1)]) {
					if (state.axes[-(settingsMain.controller5K[i] + 1)] == 1.0f * (float)settingsMain.controller5KAxisDirection[i]) {
                        gpr.heldFrets[i] = true;
						handleInputs(i, GLFW_PRESS);
					}
					else {
                        gpr.heldFrets[i] = false;
                        gpr.overhitFrets[i] = false;
						handleInputs(i, GLFW_RELEASE);
					}
					axesValues[-(settingsMain.controller5K[i] + 1)] = state.axes[-(settingsMain.controller5K[i] + 1)];
                    lane = i;
                }
			}
		}
        if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_PRESS) {
            gpr.upStrum = true;
            gpr.overstrum = false;
            handleInputs(8008135, GLFW_PRESS);
        }
        else if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_RELEASE) {
            gpr.upStrum = false;
            handleInputs(8008135, GLFW_RELEASE);
        }
        if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] == GLFW_PRESS) {
            gpr.downStrum = true;
            gpr.overstrum = false;
            handleInputs(8008135, GLFW_PRESS);
        } else if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] == GLFW_RELEASE){
            gpr.downStrum = false;
            handleInputs(8008135, GLFW_RELEASE);
        }
	}
	else{
		for (int i = 0; i < 4; i++) {
			if (settingsMain.controller4K[i] >= 0) {
				if (state.buttons[settingsMain.controller4K[i]] != buttonValues[settingsMain.controller4K[i]]) {
					if (state.buttons[settingsMain.controller4K[i]] == 1)
                        gpr.heldFrets[i] = true;
					else {
                        gpr.heldFrets[i] = false;
                        gpr.overhitFrets[i] = false;
					}
					handleInputs(i, state.buttons[settingsMain.controller4K[i]]);
					buttonValues[settingsMain.controller4K[i]] = state.buttons[settingsMain.controller4K[i]];
				}
			}
			else {
				if (state.axes[-(settingsMain.controller4K[i] + 1)] != axesValues[-(settingsMain.controller4K[i] + 1)]) {
					if (state.axes[-(settingsMain.controller4K[i] + 1)] == 1.0f * (float)settingsMain.controller4KAxisDirection[i]) {
                        gpr.heldFrets[i] = true;
						handleInputs(i, GLFW_PRESS);
					}
					else {
                        gpr.heldFrets[i] = false;
                        gpr.overhitFrets[i] = false;
						handleInputs(i, GLFW_RELEASE);
					}
					axesValues[-(settingsMain.controller4K[i] + 1)] = state.axes[-(settingsMain.controller4K[i] + 1)];
				}
			}
		}
	}

}

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
				}
				else {
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
			}
			else {
				axesValues[i] = 0.0f;
			}
		}
	}
	else {
		for (int i = 0; i < 15; i++) {
			buttonValues[i] = state.buttons[i];
		}
		for (int i = 0; i < 6; i++) {
			axesValues[i] = state.axes[i];
		}
		pressedGamepadInput = -999;
	}
}

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

Song song;

bool firstInit = true;
int loadedAssets;
bool albumArtLoaded = false;

settingsOptionRenderer sor;

Song selectedSong;

bool ReloadGameplayTexture = true;
bool songAlbumArtLoadedGameplay = false;



int main(int argc, char* argv[])
{
    Units u = Units::getInstance();
    commitHash.erase(7);
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	// SetConfigFlags(FLAG_VSYNC_HINT);
	
	//SetTraceLogLevel(LOG_NONE);

	// 800 , 600
	InitWindow(1, 1, "Encore");
    SetWindowState(FLAG_MSAA_4X_HINT);


    bool windowToggle = true;
    ArgumentList::InitArguments(argc, argv);

    std::string FPSCapStringVal = ArgumentList::GetArgValue("fpscap");
    int targetFPSArg = 0;



    if (!FPSCapStringVal.empty())
    {
#ifdef NDEBUG
        str2int(&targetFPSArg, FPSCapStringVal.c_str());
#else
        assert(str2int(&targetFPSArg, FPSCapStringVal.c_str()) == STR2INT_SUCCESS);
#endif
        TraceLog(LOG_INFO, "Argument overridden target FPS: %d", targetFPSArg);
    }


    //https://www.raylib.com/examples/core/loader.html?name=core_custom_frame_control

    double previousTime = GetTime();
    double currentTime = 0.0;
    double updateDrawTime = 0.0;
    double waitTime = 0.0;
    float deltaTime = 0.0f;

    float timeCounter = 0.0f;

    std::filesystem::path executablePath(GetApplicationDirectory());

    std::filesystem::path directory = executablePath.parent_path();

    if (std::filesystem::exists(directory / "keybinds.json")) {
        settingsMain.migrateSettings(directory / "keybinds.json", directory / "settings.json");
    }
    settingsMain.loadSettings(directory / "settings.json");
    player.InputOffset = settingsMain.inputOffsetMS / 1000.0f;
    player.VideoOffset = settingsMain.avOffsetMS / 1000.0f;
    int targetFPS = 120; // targetFPSArg == 0 ? GetMonitorRefreshRate(GetCurrentMonitor()) : targetFPSArg;
    if (!settingsMain.fullscreen) {
        if (IsWindowState(FLAG_WINDOW_UNDECORATED)) {
            ClearWindowState(FLAG_WINDOW_UNDECORATED);
            SetWindowState(FLAG_MSAA_4X_HINT);
        }
        SetWindowSize(GetMonitorWidth(GetCurrentMonitor()) * 0.75f, GetMonitorHeight(GetCurrentMonitor()) * 0.75f);
        SetWindowPosition((GetMonitorWidth(GetCurrentMonitor()) * 0.5f) - (GetMonitorWidth(GetCurrentMonitor()) * 0.375f),
                          (GetMonitorHeight(GetCurrentMonitor()) * 0.5f) -
                          (GetMonitorHeight(GetCurrentMonitor()) * 0.375f));
    } else {
        SetWindowState(FLAG_WINDOW_UNDECORATED + FLAG_MSAA_4X_HINT);
        int CurrentMonitor = GetCurrentMonitor();
        SetWindowPosition(0, 0);
        SetWindowSize(GetMonitorWidth(CurrentMonitor), GetMonitorHeight(CurrentMonitor));
    }
    std::vector<std::string> songPartsList{ "Drums","Bass","Guitar","Vocals","Classic Drums", "Classic Bass", "Classic Lead"};
    std::vector<std::string> diffList{ "Easy","Medium","Hard","Expert" };
    TraceLog(LOG_INFO, "Target FPS: %d", targetFPS);

    audioManager.Init();
    SetExitKey(0);
    audioManager.loadSample("Assets/combobreak.mp3", "miss");

    // Y UP!!!! REMEMBER!!!!!!
    //							  x,    y,     z
    //                         0.0f, 5.0f, -3.5f
    //								 6.5f
    gpr.camera.position = Vector3{ 0.0f, 7.0f, -10.0f };
    // 0.0f, 0.0f, 6.5f
    gpr.camera.target = Vector3{ 0.0f, 0.0f, 13.0f };

    gpr.camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    gpr.camera.fovy = 35.0f;
    gpr.camera.projection = CAMERA_PERSPECTIVE;






    trackSpeedButton = "Track Speed " + truncateFloatString(settingsMain.trackSpeedOptions[settingsMain.trackSpeed]) + "x";



    ChangeDirectory(GetApplicationDirectory());

    GLFWkeyfun origKeyCallback = glfwSetKeyCallback(glfwGetCurrentContext(), keyCallback);
    GLFWgamepadstatefun origGamepadCallback = glfwSetGamepadStateCallback(gamepadStateCallback);
    glfwSetKeyCallback(glfwGetCurrentContext(), origKeyCallback);
    glfwSetGamepadStateCallback(origGamepadCallback);
    // GuiLoadStyle((directory / "Assets/ui/encore.rgs").string().c_str());

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt(ColorBrightness(player.accentColor, -0.5)));
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, ColorToInt(ColorBrightness(player.accentColor, -0.3)));
    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, 0xFFFFFFFF);
    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, 0xFFFFFFFF);
    GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, 0xFFFFFFFF);
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0x505050ff);
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0xcbcbcbFF);
    GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, 0xFFFFFFFF);
    GuiSetStyle(BUTTON, TEXT_COLOR_PRESSED, 0xFFFFFFFF);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 28);


    GuiSetStyle(TOGGLE, BASE_COLOR_NORMAL, 0x181827FF);
    GuiSetStyle(TOGGLE, BASE_COLOR_FOCUSED, ColorToInt(ColorBrightness(player.accentColor, -0.5)));
    GuiSetStyle(TOGGLE, BASE_COLOR_PRESSED, ColorToInt(ColorBrightness(player.accentColor, -0.3)));
    GuiSetStyle(TOGGLE, BORDER_COLOR_NORMAL, 0xFFFFFFFF);
    GuiSetStyle(TOGGLE, BORDER_COLOR_FOCUSED, 0xFFFFFFFF);
    GuiSetStyle(TOGGLE, BORDER_COLOR_PRESSED, 0xFFFFFFFF);
    GuiSetStyle(TOGGLE, TEXT_COLOR_FOCUSED, 0xFFFFFFFF);
    GuiSetStyle(TOGGLE, TEXT_COLOR_PRESSED, 0xFFFFFFFF);


    gpr.sustainPlane = GenMeshPlane(0.8f,1.0f,1,1);
    bool wideSoloPlane = player.diff == 3;
    gpr.soloPlane = GenMeshPlane(wideSoloPlane ? 6 : 5,1.0f,1,1);

    assets.FirstAssets();
    SetWindowIcon(assets.icon);
    GuiSetFont(assets.rubik);
    assets.LoadAssets();
    RenderTexture2D notes_tex = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    RenderTexture2D hud_tex = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    RenderTexture2D highway_tex = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    RenderTexture2D highwayStatus_tex = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    RenderTexture2D smasher_tex = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    while (!WindowShouldClose())

    {
        u.calcUnits();
        GuiSetStyle(DEFAULT, TEXT_SIZE, (int)u.hinpct(0.03f));
        GuiSetStyle(DEFAULT, TEXT_SPACING, 0);
        double curTime = GetTime();
        float bgTime = curTime / 5.0f;

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



        float diffDistance = player.diff == 3 ? 2.0f : 1.5f;
        float lineDistance = player.diff == 3 ? 1.5f : 1.0f;
        BeginDrawing();

        // menu.loadTitleScreen();

        ClearBackground(DARKGRAY);

        
        SetShaderValue(assets.bgShader, assets.bgTimeLoc, &bgTime, SHADER_UNIFORM_FLOAT);
        

        switch (menu.currentScreen) {
            case MENU: {
                if (!menu.songsLoaded) {
                    if (std::filesystem::exists("songCache.encr")) {
                        songList = songList.LoadCache(settingsMain.songPaths);
                        menu.songsLoaded = true;
                    }
                }

                menu.loadMenu(gamepadStateCallbackSetControls);
                break;
            }
            case CALIBRATION: {

                static bool sampleLoaded = false;
                if (!sampleLoaded) {
                    audioManager.loadSample("Assets/kick.wav", "click");
                    sampleLoaded = true;
                }

                if (GuiButton({ (float)GetScreenWidth() / 2 - 250, (float)GetScreenHeight() - 120, 200, 60 }, "Start Calibration")) {
                    isCalibrating = true;
                    calibrationStartTime = GetTime();
                    lastClickTime = calibrationStartTime;
                    tapTimes.clear();
                }
                if (GuiButton({ (float)GetScreenWidth() / 2 + 50, (float)GetScreenHeight() - 120, 200, 60 }, "Stop Calibration")) {
                    isCalibrating = false;

                    if (tapTimes.size() > 1) {
                        double totalDifference = 0.0;
                        for (double tapTime : tapTimes) {
                            double expectedClickTime = round((tapTime - calibrationStartTime) / clickInterval) * clickInterval + calibrationStartTime;
                            totalDifference += (tapTime - expectedClickTime);
                        }
                        settingsMain.avOffsetMS = static_cast<int>((totalDifference / tapTimes.size()) * 1000);  // Convert to milliseconds
                        settingsMain.inputOffsetMS = settingsMain.avOffsetMS;
                        std::cout << static_cast<int>((totalDifference / tapTimes.size()) * 1000) << "ms of latency detected" << std::endl;
                    }
                    std::cout << "Stopped Calibration" << std::endl;
                    tapTimes.clear();
                }

                if (isCalibrating) {
                    double currentTime = GetTime();
                    double elapsedTime = currentTime - lastClickTime;

                    if (elapsedTime >= clickInterval) {
                        audioManager.playSample("click", 1);
                        lastClickTime += clickInterval;  // Increment by the interval to avoid missing clicks
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
                    Color feedbackColor = {0, 255, 0, static_cast<unsigned char>(inputFeedbackAlpha * 255)};
                    DrawTextEx(assets.rubikBold, "Input Registered", {static_cast<float>((GetScreenWidth() - u.hinpct(0.35f)) / 2), static_cast<float>(GetScreenHeight() / 2)}, u.hinpct(0.05f), 0, feedbackColor);
                }

                if (GuiButton({ ((float)GetScreenWidth() / 2) - 350,((float)GetScreenHeight() - 60),100,60 }, "Cancel")) {
                    isCalibrating = false;
                    settingsMain.avOffsetMS = settingsMain.prevAvOffsetMS;
                    settingsMain.inputOffsetMS = settingsMain.prevInputOffsetMS;
                    tapTimes.clear();

                    settingsMain.saveSettings(directory / "settings.json");
                    menu.SwitchScreen(SETTINGS);
                }

                if (GuiButton({ ((float)GetScreenWidth() / 2) + 250,((float)GetScreenHeight() - 60),100,60 }, "Apply")) {
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
                    menu.DrawAlbumArtBackground(menu.ChosenSong.albumArtBlur);
                if (settingsMain.controllerType == -1 && controllerID != -1) {
                    std::string gamepadName = std::string(glfwGetGamepadName(controllerID));
                    settingsMain.controllerType = keybinds.getControllerType(gamepadName);
                }
                float TextPlacementTB = u.hpct(0.15f) - u.hinpct(0.11f);
                float TextPlacementLR = u.wpct(0.01f);
                DrawRectangle(u.LeftSide, 0, u.winpct(1.0f), GetScreenHeight(), Color{0, 0, 0, 128});
                DrawLineEx({u.LeftSide + u.winpct(0.0025f), 0},
                           {u.LeftSide + u.winpct(0.0025f), (float) GetScreenHeight()}, u.winpct(0.005f), WHITE);
                DrawLineEx({u.RightSide - u.winpct(0.0025f), 0},
                           {u.RightSide - u.winpct(0.0025f), (float) GetScreenHeight()}, u.winpct(0.005f), WHITE);

                menu.DrawTopOvershell(0.15f);
                menu.DrawVersion();
                menu.DrawBottomOvershell();
                menu.DrawBottomBottomOvershell();
                DrawTextEx(assets.redHatDisplayBlack, "Options", {TextPlacementLR, TextPlacementTB}, u.hinpct(0.10f), 0,
                           WHITE);

                float OvershellBottom = u.hpct(0.15f);
                if (GuiButton({((float) GetScreenWidth() / 2) - 350, ((float) GetScreenHeight() - 60), 100, 60},
                              "Cancel") && !(changingKey || changingOverdrive || changingPause)) {
                    glfwSetGamepadStateCallback(origGamepadCallback);
                    settingsMain.keybinds4K = settingsMain.prev4K;
                    settingsMain.keybinds5K = settingsMain.prev5K;
                    settingsMain.keybinds4KAlt = settingsMain.prev4KAlt;
                    settingsMain.keybinds5KAlt = settingsMain.prev5KAlt;
                    settingsMain.keybindOverdrive = settingsMain.prevOverdrive;
                    settingsMain.keybindOverdriveAlt = settingsMain.prevOverdriveAlt;
                    settingsMain.keybindPause = settingsMain.prevKeybindPause;

                    settingsMain.controller4K = settingsMain.prevController4K;
                    settingsMain.controller4KAxisDirection = settingsMain.prevController4KAxisDirection;
                    settingsMain.controller5K = settingsMain.prevController5K;
                    settingsMain.controller5KAxisDirection = settingsMain.prevController5KAxisDirection;
                    settingsMain.controllerOverdrive = settingsMain.prevControllerOverdrive;
                    settingsMain.controllerOverdriveAxisDirection = settingsMain.prevControllerOverdriveAxisDirection;
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

                    menu.SwitchScreen(MENU);
                }
                if (GuiButton({((float) GetScreenWidth() / 2) + 250, ((float) GetScreenHeight() - 60), 100, 60},
                              "Apply") && !(changingKey || changingOverdrive || changingPause)) {
                    glfwSetGamepadStateCallback(origGamepadCallback);
                    if (settingsMain.fullscreen) {
                        SetWindowState(FLAG_WINDOW_UNDECORATED);
                        SetWindowState(FLAG_MSAA_4X_HINT);
                        int CurrentMonitor = GetCurrentMonitor();
                        SetWindowPosition(0, 0);
                        SetWindowSize(GetMonitorWidth(CurrentMonitor), GetMonitorHeight(CurrentMonitor));
                    } else {
                        if (IsWindowState(FLAG_WINDOW_UNDECORATED)) {
                            ClearWindowState(FLAG_WINDOW_UNDECORATED);
                            SetWindowState(FLAG_MSAA_4X_HINT);
                        }
                        if (settingsMain.fullscreen != settingsMain.fullscreenPrev) {
                            SetWindowSize(GetMonitorWidth(GetCurrentMonitor()) * 0.75,
                                          GetMonitorHeight(GetCurrentMonitor()) * 0.75);
                            SetWindowPosition((GetMonitorWidth(GetCurrentMonitor()) * 0.5) -
                                              (GetMonitorWidth(GetCurrentMonitor()) * 0.375),
                                              (GetMonitorHeight(GetCurrentMonitor()) * 0.5) -
                                              (GetMonitorHeight(GetCurrentMonitor()) * 0.375));
                        }
                    }
                    settingsMain.prev4K = settingsMain.keybinds4K;
                    settingsMain.prev5K = settingsMain.keybinds5K;
                    settingsMain.prev4KAlt = settingsMain.keybinds4KAlt;
                    settingsMain.prev5KAlt = settingsMain.keybinds5KAlt;
                    settingsMain.prevOverdrive = settingsMain.keybindOverdrive;
                    settingsMain.prevOverdriveAlt = settingsMain.keybindOverdriveAlt;
                    settingsMain.prevKeybindPause = settingsMain.keybindPause;

                    settingsMain.prevController4K = settingsMain.controller4K;
                    settingsMain.prevController4KAxisDirection = settingsMain.controller4KAxisDirection;
                    settingsMain.prevController5K = settingsMain.controller5K;
                    settingsMain.prevController5KAxisDirection = settingsMain.controller5KAxisDirection;
                    settingsMain.prevControllerOverdrive = settingsMain.controllerOverdrive;
                    settingsMain.prevControllerPause = settingsMain.controllerPause;
                    settingsMain.prevControllerOverdriveAxisDirection = settingsMain.controllerOverdriveAxisDirection;
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

                    player.InputOffset = settingsMain.inputOffsetMS / 1000.0f;
                    player.VideoOffset = settingsMain.avOffsetMS / 1000.0f;

                    settingsMain.saveSettings(directory / "settings.json");


                    menu.SwitchScreen(MENU);
                }
                static int selectedTab = 0;
                static int displayedTab = 0;

                static int selectedKbTab = 0;
                static int displayedKbTab = 0;

                GuiToggleGroup({u.LeftSide + u.winpct(0.005f), OvershellBottom, (u.winpct(0.985f) / 5), u.hinpct(0.05)},
                               "Main;Highway;Volume;Keyboard Controls;Gamepad Controls", &selectedTab);
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
                GuiSetStyle(SLIDER, BASE_COLOR_PRESSED, ColorToInt(ColorBrightness(player.accentColor, -0.25f)));
                GuiSetStyle(SLIDER, TEXT_COLOR_FOCUSED, ColorToInt(ColorBrightness(player.accentColor, -0.5f)));
                GuiSetStyle(SLIDER, BORDER, 0xFFFFFFFF);
                GuiSetStyle(SLIDER, BORDER_COLOR_FOCUSED, 0xFFFFFFFF);
                GuiSetStyle(SLIDER, BORDER_WIDTH, 2);

                switch (displayedTab) {
                    case MAIN: { // Main settings tab


                        float trackSpeedFloat = settingsMain.trackSpeed;



                        // header 1
                        // calibration header
                        int calibrationMenuOffset = 0;

                        DrawRectangle(u.wpct(0.005f), underTabsHeight + (EntryHeight * calibrationMenuOffset),
                                      OptionWidth * 2,
                                      EntryHeight, Color{0, 0, 0, 128});
                        DrawTextEx(assets.rubikBoldItalic, "Calibration",
                                   {HeaderTextLeft,
                                    OvershellBottom + u.hinpct(0.055f) + (EntryHeight * calibrationMenuOffset)},
                                   u.hinpct(0.04f), 0, WHITE);

                            // av offset

                            settingsMain.avOffsetMS = sor.sliderEntry(settingsMain.avOffsetMS, -500.0f, 500.0f,
                                                                      calibrationMenuOffset + 1, "Audio/Visual Offset",
                                                                      1);

                            // input offset
                            DrawRectangle(u.wpct(0.005f), underTabsHeight + (EntryHeight * (calibrationMenuOffset + 2)),
                                          OptionWidth * 2, EntryHeight, Color{0, 0, 0, 64});
                            settingsMain.inputOffsetMS = sor.sliderEntry(settingsMain.inputOffsetMS, -500.0f, 500.0f,
                                                                         calibrationMenuOffset + 2, "Input Offset", 1);

                            float calibrationTop = EntryTop + (EntryHeight * (calibrationMenuOffset + 2));
                            float calibrationTextTop = EntryTextTop + (EntryHeight * (calibrationMenuOffset + 2));
                            DrawTextEx(assets.rubikBold, "Automatic Calibration", {EntryTextLeft, calibrationTextTop},
                                       EntryFontSize, 0, WHITE);
                            if (GuiButton({OptionLeft, calibrationTop, OptionWidth, EntryHeight},
                                          "Start Calibration")) {
                                menu.SwitchScreen(CALIBRATION);
                            }


                        int generalOffset = 4;
                        // general header
                        DrawRectangle(u.wpct(0.005f), underTabsHeight + (EntryHeight * generalOffset), OptionWidth * 2,
                                      EntryHeight, Color{0, 0, 0, 128});
                        DrawTextEx(assets.rubikBoldItalic, "General",
                                   {HeaderTextLeft, OvershellBottom + u.hinpct(0.055f) + (EntryHeight * generalOffset)},
                                   u.hinpct(0.04f), 0, WHITE);

                        // fullscreen

                            settingsMain.fullscreen = sor.toggleEntry(settingsMain.fullscreen, generalOffset + 1,
                                                                      "Fullscreen");

                            DrawRectangle(u.wpct(0.005f), underTabsHeight + (EntryHeight * (generalOffset + 2)),
                                          OptionWidth * 2, EntryHeight, Color{0, 0, 0, 64});

                            float scanTop = EntryTop + (EntryHeight * (generalOffset + 1));
                            float scanTextTop = EntryTextTop + (EntryHeight * (generalOffset + 1));
                            DrawTextEx(assets.rubikBold, "Scan Songs", {EntryTextLeft, scanTextTop},
                                       EntryFontSize, 0, WHITE);
                            if (GuiButton({OptionLeft, scanTop, OptionWidth, EntryHeight}, "Scan")) {
                                menu.songsLoaded = false;
                                songList.ScanSongs(settingsMain.songPaths);
                            }



                        break;
                    }
                    case HIGHWAY: {
                        DrawRectangle(u.wpct(0.005f), OvershellBottom + u.hinpct(0.05f), OptionWidth * 2, EntryHeight,
                                      Color{0, 0, 0, 128});
                        DrawTextEx(assets.rubikBoldItalic, "Highway",
                                   {HeaderTextLeft, OvershellBottom + u.hinpct(0.055f)},
                                   u.hinpct(0.04f), 0, WHITE);
                        settingsMain.trackSpeed = sor.sliderEntry(settingsMain.trackSpeed, 0,
                                                                  settingsMain.trackSpeedOptions.size() - 1, 1,
                                                                  "Track Speed", 1.0f);
                        // highway length

                        DrawRectangle(u.wpct(0.005f), underTabsHeight + (EntryHeight * 2), OptionWidth * 2,
                                      EntryHeight,
                                      Color{0, 0, 0, 64});
                        settingsMain.highwayLengthMult = sor.sliderEntry(settingsMain.highwayLengthMult, 0.25f,
                                                                         2.5f, 2,
                                                                         "Highway Length Multiplier", 0.25f);

                        // miss color
                        settingsMain.missHighwayDefault = sor.toggleEntry(settingsMain.missHighwayDefault, 3,
                                                                          "Highway Miss Color");

                        // lefty flip
                        DrawRectangle(u.wpct(0.005f), underTabsHeight + (EntryHeight * 4), OptionWidth * 2,
                                      EntryHeight,
                                      Color{0, 0, 0, 64});
                        settingsMain.mirrorMode = sor.toggleEntry(settingsMain.mirrorMode, 4, "Mirror/Lefty Mode");

                        menu.hehe = sor.toggleEntry(menu.hehe, 5, "Super Cool Highway Colors");
                        break;
                    }
                    case VOLUME: { // audio tab
                        DrawRectangle(u.wpct(0.005f), OvershellBottom + u.hinpct(0.05f), OptionWidth * 2, EntryHeight,
                                      Color{0, 0, 0, 128});
                        DrawTextEx(assets.rubikBoldItalic, "Volume", {HeaderTextLeft, OvershellBottom + u.hinpct(0.055f)},
                                   u.hinpct(0.04f), 0, WHITE);

                        settingsMain.MainVolume = sor.sliderEntry(settingsMain.MainVolume, 0,
                                                                  1, 1,
                                                                  "Main Volume", 0.05f);


                        settingsMain.PlayerVolume = sor.sliderEntry(settingsMain.PlayerVolume, 0, 1, 2,
                                                                    "Player Volume", 0.05f);

                        settingsMain.BandVolume = sor.sliderEntry(settingsMain.BandVolume, 0,
                                                                  1, 3,
                                                                  "Band Volume", 0.05f);

                        settingsMain.SFXVolume = sor.sliderEntry(settingsMain.SFXVolume, 0, 1, 4,
                                                                 "SFX Volume", 0.05f);

                        settingsMain.MissVolume = sor.sliderEntry(settingsMain.MissVolume, 0, 1, 5,
                                                                 "Miss Volume", 0.05f);

                        settingsMain.MenuVolume = sor.sliderEntry(settingsMain.MenuVolume, 0, 1, 6,
                                                                 "Menu Music Volume", 0.05f);

                        player.selInstVolume = settingsMain.MainVolume * settingsMain.PlayerVolume;
                        player.otherInstVolume = settingsMain.MainVolume * settingsMain.BandVolume;
                        player.sfxVolume = settingsMain.MainVolume * settingsMain.SFXVolume;
                        player.missVolume = settingsMain.MainVolume * settingsMain.MissVolume;

                        break;
                    }
                    case KEYBOARD: { //Keyboard bindings tab
                        GuiToggleGroup({u.LeftSide + u.winpct(0.005f), OvershellBottom + u.hinpct(0.05f),
                                        (u.winpct(0.987f) / 4), u.hinpct(0.05)},
                                       "Pad Binds;Classic Binds;Misc Gameplay;Menu Binds", &selectedKbTab);
                        GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
                        switch (displayedKbTab) {
                            case kbPAD: {
                                for (int i = 0; i < 5; i++) {
                                    sor.keybindEntryText(i + 2, "Lane " + to_string(i + 1));
                                    sor.keybind5kEntry(settingsMain.keybinds5K[i], i + 2, "Lane " + to_string(i + 1),
                                                     keybinds, i);
                                    sor.keybind5kAltEntry(settingsMain.keybinds5KAlt[i], i + 2,
                                                        "Lane " + to_string(i + 1),
                                                        keybinds,
                                                        i);
                                }
                                for (int i = 0; i < 4; i++) {
                                    sor.keybindEntryText(i + 8, "Lane " + to_string(i + 1));
                                    sor.keybind4kEntry(settingsMain.keybinds4K[i], i + 8, "Lane " + to_string(i + 1),
                                                     keybinds,
                                                     i);
                                    sor.keybind4kAltEntry(settingsMain.keybinds4KAlt[i], i + 8,
                                                        "Lane " + to_string(i + 1),
                                                        keybinds,
                                                        i);
                                }
                                GuiSetStyle(DEFAULT, TEXT_SIZE, 28);
                                break;
                            }
                            case kbCLASSIC: {
                                DrawRectangle(u.wpct(0.005f), underTabsHeight + (EntryHeight * 2),
                                              OptionWidth * 2,
                                              EntryHeight, ColorAlpha(GREEN, 0.1f));
                                DrawRectangle(u.wpct(0.005f), underTabsHeight + (EntryHeight * 3),
                                              OptionWidth * 2,
                                              EntryHeight, ColorAlpha(RED, 0.1f));
                                DrawRectangle(u.wpct(0.005f), underTabsHeight + (EntryHeight * 4),
                                              OptionWidth * 2,
                                              EntryHeight, ColorAlpha(YELLOW, 0.1f));
                                DrawRectangle(u.wpct(0.005f), underTabsHeight + (EntryHeight * 5),
                                              OptionWidth * 2,
                                              EntryHeight, ColorAlpha(BLUE, 0.1f));
                                DrawRectangle(u.wpct(0.005f), underTabsHeight + (EntryHeight * 6),
                                              OptionWidth * 2,
                                              EntryHeight, ColorAlpha(ORANGE, 0.1f));
                                for (int i = 0; i < 5; i++) {
                                    sor.keybindEntryText(i + 2, "Lane " + to_string(i + 1));
                                    sor.keybind5kEntry(settingsMain.keybinds5K[i], i + 2, "Lane " + to_string(i + 1),
                                                       keybinds, i);
                                    sor.keybind5kAltEntry(settingsMain.keybinds5KAlt[i], i + 2,
                                                          "Lane " + to_string(i + 1),
                                                          keybinds,
                                                          i);
                                }
                                sor.keybindEntryText(8, "Strum Up");
                                sor.keybindStrumEntry(0, 8, settingsMain.keybindStrumUp, keybinds);

                                sor.keybindEntryText(9, "Strum Down");
                                sor.keybindStrumEntry(1, 9, settingsMain.keybindStrumDown, keybinds);

                                break;
                            }
                            case kbMISC: {
                                sor.keybindEntryText(2, "Overdrive");
                                sor.keybindOdAltEntry(settingsMain.keybindOverdriveAlt, 2, "Overdrive Alt", keybinds);
                                sor.keybindOdEntry(settingsMain.keybindOverdrive, 2, "Overdrive", keybinds);

                                sor.keybindEntryText(3, "Pause Song");
                                sor.keybindPauseEntry(settingsMain.keybindPause, 3, "Pause", keybinds);
                                break;
                            }
                            case kbMENUS: {
                                break;
                            }

                        }
                        if (sor.changingKey) {
                            std::vector<int> &bindsToChange = sor.changingAlt ? (sor.changing4k
                                                                                 ? settingsMain.keybinds4KAlt
                                                                                 : settingsMain.keybinds5KAlt)
                                                                              : (sor.changing4k
                                                                                 ? settingsMain.keybinds4K
                                                                                 : settingsMain.keybinds5K);
                            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0, 0, 0, 200});
                            std::string keyString = (sor.changing4k ? "4k" : "5k");
                            std::string altString = (sor.changingAlt ? " alt" : "");
                            std::string changeString = "Press a key for " + keyString + altString + " lane ";
                            DrawTextRubik(changeString.c_str(),
                                          ((float) GetScreenWidth() -
                                           MeasureTextRubik(changeString.c_str(), 20)) / 2,
                                          (float) GetScreenHeight() / 2 - 30, 20, WHITE);
                            int pressedKey = GetKeyPressed();
                            if (GuiButton(
                                    {((float) GetScreenWidth() / 2) - 130, GetScreenHeight() - 60.0f, 120, 40},
                                    "Unbind Key")) {
                                pressedKey = -2;
                            }
                            if (GuiButton(
                                    {((float) GetScreenWidth() / 2) + 10, GetScreenHeight() - 60.0f, 120, 40},
                                    "Cancel")) {
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
                            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0, 0, 0, 200});
                            std::string altString = (sor.changingAlt ? " alt" : "");
                            std::string changeString = "Press a key for " + altString + " overdrive";
                            DrawTextRubik(changeString.c_str(),
                                          ((float) GetScreenWidth() -
                                           MeasureTextRubik(changeString.c_str(), 20)) / 2,
                                          (float) GetScreenHeight() / 2 - 30, 20, WHITE);
                            int pressedKey = GetKeyPressed();
                            if (GuiButton(
                                    {((float) GetScreenWidth() / 2) - 130, GetScreenHeight() - 60.0f, 120, 40},
                                    "Unbind Key")) {
                                pressedKey = -2;
                            }
                            if (GuiButton(
                                    {((float) GetScreenWidth() / 2) + 10, GetScreenHeight() - 60.0f, 120, 40},
                                    "Cancel")) {
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
                            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0, 0, 0, 200});
                            DrawTextRubik("Press a key for Pause",
                                          ((float) GetScreenWidth() -
                                           MeasureTextRubik("Press a key for Pause", 20)) /
                                          2,
                                          (float) GetScreenHeight() / 2 - 30, 20, WHITE);
                            int pressedKey = GetKeyPressed();
                            if (GuiButton(
                                    {((float) GetScreenWidth() / 2) - 130, GetScreenHeight() - 60.0f, 120, 40},
                                    "Unbind Key")) {
                                pressedKey = -2;
                            }
                            if (GuiButton(
                                    {((float) GetScreenWidth() / 2) + 10, GetScreenHeight() - 60.0f, 120, 40},
                                    "Cancel")) {
                                sor.changingAlt = false;
                                sor.changingPause = false;
                            }
                            if (pressedKey != 0) {
                                settingsMain.keybindPause = pressedKey;
                                sor.changingPause = false;
                            }
                        }
                        if (sor.changingStrumUp) {
                            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0, 0, 0, 200});
                            DrawTextRubik("Press a key for Strum Up",
                                          ((float) GetScreenWidth() -
                                           MeasureTextRubik("Press a key for Strum Up", 20)) /
                                          2,
                                          (float) GetScreenHeight() / 2 - 30, 20, WHITE);
                            int pressedKey = GetKeyPressed();
                            if (GuiButton(
                                    {((float) GetScreenWidth() / 2) - 130, GetScreenHeight() - 60.0f, 120, 40},
                                    "Unbind Key")) {
                                pressedKey = -2;
                            }
                            if (GuiButton(
                                    {((float) GetScreenWidth() / 2) + 10, GetScreenHeight() - 60.0f, 120, 40},
                                    "Cancel")) {
                                sor.changingAlt = false;
                                sor.changingStrumUp = false;
                            }
                            if (pressedKey != 0) {
                                settingsMain.keybindStrumUp = pressedKey;
                                sor.changingStrumUp = false;
                            }
                        }
                        if (sor.changingStrumDown) {
                            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0, 0, 0, 200});
                            DrawTextRubik("Press a key for Strum Down",
                                          ((float) GetScreenWidth() -
                                           MeasureTextRubik("Press a key for Strum Down", 20)) /
                                          2,
                                          (float) GetScreenHeight() / 2 - 30, 20, WHITE);
                            int pressedKey = GetKeyPressed();
                            if (GuiButton(
                                    {((float) GetScreenWidth() / 2) - 130, GetScreenHeight() - 60.0f, 120, 40},
                                    "Unbind Key")) {
                                pressedKey = -2;
                            }
                            if (GuiButton(
                                    {((float) GetScreenWidth() / 2) + 10, GetScreenHeight() - 60.0f, 120, 40},
                                    "Cancel")) {
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
                    case GAMEPAD: { //Controller bindings tab
                        GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
                        for (int i = 0; i < 5; i++) {
                            float j = (float) i - 2.0f;
                            if (GuiButton({((float) GetScreenWidth() / 2) - 40 + (80 * j), 240, 80, 60},
                                          keybinds.getControllerStr(controllerID, settingsMain.controller5K[i],
                                                                    settingsMain.controllerType,
                                                                    settingsMain.controller5KAxisDirection[i]).c_str())) {
                                changing4k = false;
                                selLane = i;
                                changingKey = true;
                            }
                        }
                        for (int i = 0; i < 4; i++) {
                            float j = (float) i - 1.5f;
                            if (GuiButton({((float) GetScreenWidth() / 2) - 40 + (80 * j), 360, 80, 60},
                                          keybinds.getControllerStr(controllerID, settingsMain.controller4K[i],
                                                                    settingsMain.controllerType,
                                                                    settingsMain.controller4KAxisDirection[i]).c_str())) {
                                changing4k = true;
                                selLane = i;
                                changingKey = true;
                            }
                        }
                        if (GuiButton({((float) GetScreenWidth() / 2) - 40, 480, 80, 60},
                                      keybinds.getControllerStr(controllerID, settingsMain.controllerOverdrive,
                                                                settingsMain.controllerType,
                                                                settingsMain.controllerOverdriveAxisDirection).c_str())) {
                            changingKey = false;
                            changingOverdrive = true;
                        }
                        if (GuiButton({((float) GetScreenWidth() / 2) - 40, 560, 80, 60},
                                      keybinds.getControllerStr(controllerID, settingsMain.controllerPause,
                                                                settingsMain.controllerType,
                                                                settingsMain.controllerPauseAxisDirection).c_str())) {
                            changingKey = false;
                            changingOverdrive = true;
                        }
                        if (changingKey) {
                            std::vector<int> &bindsToChange = (changing4k ? settingsMain.controller4K
                                                                          : settingsMain.controller5K);
                            std::vector<int> &directionToChange = (changing4k ? settingsMain.controller4KAxisDirection
                                                                              : settingsMain.controller5KAxisDirection);
                            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0, 0, 0, 200});
                            std::string keyString = (changing4k ? "4k" : "5k");
                            std::string changeString = "Press a button/axis for controller " + keyString + " lane " +
                                                       std::to_string(selLane + 1);
                            DrawTextRubik(changeString.c_str(),
                                          ((float) GetScreenWidth() - MeasureTextRubik(changeString.c_str(), 20)) / 2,
                                          GetScreenHeight() / 2 - 30, 20, WHITE);
                            if (GuiButton({((float) GetScreenWidth() / 2) - 60, GetScreenHeight() - 60.0f, 120, 40},
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
                            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0, 0, 0, 200});
                            std::string changeString = "Press a button/axis for controller overdrive";
                            DrawTextRubik(changeString.c_str(),
                                          ((float) GetScreenWidth() - MeasureTextRubik(changeString.c_str(), 20)) / 2,
                                          GetScreenHeight() / 2 - 30, 20, WHITE);
                            if (GuiButton({((float) GetScreenWidth() / 2) - 60, GetScreenHeight() - 60.0f, 120, 40},
                                          "Cancel")) {
                                changingOverdrive = false;
                            }
                            if (pressedGamepadInput != -999) {
                                settingsMain.controllerOverdrive = pressedGamepadInput;
                                if (pressedGamepadInput < 0) {
                                    settingsMain.controllerOverdriveAxisDirection = axisDirection;
                                }
                                changingOverdrive = false;
                                pressedGamepadInput = -999;
                            }
                        }
                        if (changingPause) {
                            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0, 0, 0, 200});
                            std::string changeString = "Press a button/axis for controller pause";
                            DrawTextRubik(changeString.c_str(),
                                          ((float) GetScreenWidth() - MeasureTextRubik(changeString.c_str(), 20)) / 2,
                                          GetScreenHeight() / 2 - 30, 20, WHITE);
                            if (GuiButton({((float) GetScreenWidth() / 2) - 60, GetScreenHeight() - 60.0f, 120, 40},
                                          "Cancel")) {
                                changingPause = false;
                            }
                            if (pressedGamepadInput != -999) {
                                settingsMain.controllerPause = pressedGamepadInput;
                                if (pressedGamepadInput < 0) {
                                    settingsMain.controllerPauseAxisDirection = axisDirection;
                                }
                                changingPause = false;
                                pressedGamepadInput = -999;
                            }
                        }
                        GuiSetStyle(DEFAULT, TEXT_SIZE, 28);
                        break;
                    }
                }
                break;
            }
            case SONG_SELECT: {
                if (!menu.songsLoaded) {

                    songList = songList.LoadCache(settingsMain.songPaths);
                    menu.songsLoaded = true;

                }
                u.calcUnits();
                streamsLoaded = false;
                midiLoaded = false;
                isPlaying = false;
                gpr.songEnded = false;
                player.overdrive = false;
                gpr.curNoteIdx = { 0,0,0,0,0 };
                gpr.curODPhrase = 0;
                gpr.curNoteInt = 0;
                gpr.curSolo = 0;
                gpr.curBeatLine = 0;
                gpr.curBPM = 0;

                if (selSong)
                    gpr.selectedSongInt = curPlayingSong;
                else
                    gpr.selectedSongInt = menu.ChosenSongInt;



                SetTextureWrap(selectedSong.albumArtBlur, TEXTURE_WRAP_REPEAT);
                SetTextureFilter(selectedSong.albumArtBlur, TEXTURE_FILTER_ANISOTROPIC_16X);

                Vector2 mouseWheel = GetMouseWheelMoveV();
                int lastIntChosen = (int)mouseWheel.y;
                // set to specified height
                if (songSelectOffset <= songList.songs.size() && songSelectOffset >= 0) {
                    songSelectOffset -= (int)mouseWheel.y;
                }

                // prevent going past top
                if (songSelectOffset < 0)
                    songSelectOffset = 0;

                // prevent going past bottom
                if (songSelectOffset >= songList.songs.size() - 12)
                    songSelectOffset = (int)songList.songs.size() - 12;

                if (!albumArtLoaded) {
                    selectedSong = menu.ChosenSong;
                    gpr.selectedSongInt = menu.ChosenSongInt;
                    selectedSong.LoadAlbumArt(selectedSong.albumArtPath);
                    if (!selSong)
                        songSelectOffset = menu.ChosenSongInt - 5;
                    albumArtLoaded = true;
                } else {
                    menu.ChosenSong = selectedSong;
                }

                BeginShaderMode(assets.bgShader);
                if (selSong){

                    menu.DrawAlbumArtBackground(selectedSong.albumArtBlur);
                }
                else {

                    Song art = menu.ChosenSong;
                    menu.DrawAlbumArtBackground(art.albumArtBlur);
                }
                EndShaderMode();

                float TopOvershell = u.hpct(0.15f);
                DrawRectangle((int)u.LeftSide,0, u.RightSide - u.LeftSide, (float)GetScreenHeight(), Color(0,0,0,128));
                DrawLineEx({u.LeftSide + u.winpct(0.0025f),0},{u.LeftSide + u.winpct(0.0025f),(float)GetScreenHeight()}, u.winpct(0.005f), WHITE);

                menu.DrawTopOvershell(0.15f);


                menu.DrawVersion();
                int AlbumX = u.RightSide - u.winpct(0.25f);
                int AlbumY = u.hpct(0.075f);
                int AlbumHeight = u.winpct(0.25f);
                int AlbumOuter = u.hinpct(0.01f);
                int AlbumInner = u.hinpct(0.005f);
                int BorderBetweenAlbumStuff = (u.RightSide - u.LeftSide) - u.winpct(0.25f);


                DrawTextEx(assets.josefinSansItalic, TextFormat("Sorted by: %s", sortTypes[currentSortValue].c_str()), {AlbumX - (AlbumOuter * 2) - MeasureTextEx(assets.josefinSansItalic, TextFormat("Sorted by: %s", sortTypes[currentSortValue].c_str()), u.hinpct(0.025f), 0).x, AlbumY + u.hinpct(0.0075)}, u.hinpct(0.025f), 0, WHITE);
                DrawTextEx(assets.josefinSansItalic, TextFormat("Songs loaded: %01i", songList.songs.size()), {AlbumX - (AlbumOuter * 2) - MeasureTextEx(assets.josefinSansItalic, TextFormat("Songs loaded: %01i", songList.songs.size()), u.hinpct(0.025f), 0).x, AlbumY +u.hinpct(0.04f)}, u.hinpct(0.025f), 0, WHITE);


                float songEntryHeight = u.hinpct(0.06f);
                for (int i = songSelectOffset; i < songList.songs.size() && i < songSelectOffset+12; i++) {
                    if (songList.songs.size() == i)
                        break;
                    Font& songFont = i == menu.ChosenSongInt ? assets.rubikBoldItalic : assets.rubikBold;
                    Font& artistFont = i == menu.ChosenSongInt ? assets.josefinSansItalic : assets.josefinSansItalic;
                    Song& songi = songList.songs[i];
                    // float buttonX = ((float)GetScreenWidth()/2)-(((float)GetScreenWidth()*0.86f)/2);
                    //LerpState state = lerpCtrl.createLerp("SONGSELECT_LERP_" + std::to_string(i), EaseOutCirc, 0.4f);
                    float songXPos = u.LeftSide+u.winpct(0.005f)-2;
                    float songYPos = (u.hpct(0.15f)-2) + ((songEntryHeight-1) * (i - songSelectOffset));

                    if (i == menu.ChosenSongInt) {
                        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(ColorBrightness(player.accentColor, -0.4)));
                    }
                    BeginScissorMode(u.LeftSide, u.hpct(0.15f), BorderBetweenAlbumStuff,u.hinpct(0.7f));
                    if (GuiButton(Rectangle{ songXPos, songYPos,(u.winpct(0.75f)), songEntryHeight }, "")) {
                        curPlayingSong = i;
                        selSong = true;
                        albumArtSelectedAndLoaded = false;
                        albumArtLoaded = false;
                        menu.ChosenSong = songList.songs[i];
                        menu.ChosenSongInt = i;
                    }
                    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
                    EndScissorMode();
                    // DrawTexturePro(song.albumArt, Rectangle{ songXPos,0,(float)song.albumArt.width,(float)song.albumArt.height }, { songXPos+5,songYPos + 5,50,50 }, Vector2{ 0,0 }, 0.0f, RAYWHITE);
                    int songTitleWidth = (u.winpct(0.3f))-6;

                    int songArtistWidth = (u.winpct(0.5f))-6;

                    if (songi.titleTextWidth >= (float)songTitleWidth) {
                        if (curTime > songi.titleScrollTime && curTime < songi.titleScrollTime + 3.0)
                            songi.titleXOffset = 0;

                        if (curTime > songi.titleScrollTime + 3.0) {
                            songi.titleXOffset -= 1;

                            if (songi.titleXOffset < -(songi.titleTextWidth - (float)songTitleWidth)) {
                                songi.titleXOffset = -(songi.titleTextWidth - (float)songTitleWidth);
                                songi.titleScrollTime = curTime + 3.0;
                            }
                        }
                    }
                    auto LightText = Color{203, 203, 203, 255};
                    BeginScissorMode((int)songXPos + 20, (int)songYPos, songTitleWidth, songEntryHeight);
                    DrawTextEx(songFont,songi.title.c_str(), {songXPos + 20 + songi.titleXOffset + (i == menu.ChosenSongInt ? u.winpct(0.005f) : 0), songYPos + u.hinpct(0.0125f)}, u.hinpct(0.035f),0, i == menu.ChosenSongInt ? WHITE : LightText);
                    EndScissorMode();

                    if (songi.artistTextWidth > (float)songArtistWidth) {
                        if (curTime > songi.artistScrollTime && curTime < songi.artistScrollTime + 3.0)
                            songi.artistXOffset = 0;

                        if (curTime > songi.artistScrollTime + 3.0) {
                            songi.artistXOffset -= 1;
                            if (songi.artistXOffset < -(songi.artistTextWidth - (float)songArtistWidth)) {
                                songi.artistScrollTime = curTime + 3.0;
                            }
                        }
                    }

                    auto SelectedText = WHITE;
                    BeginScissorMode((int)songXPos + 30 + (int)songTitleWidth, (int)songYPos, songArtistWidth, songEntryHeight);
                    DrawTextEx(artistFont, songi.artist.c_str(), {songXPos + 30 + (float)songTitleWidth + songi.artistXOffset, songYPos + u.hinpct(0.02f)}, u.hinpct(0.025f), 0, i == menu.ChosenSongInt ? WHITE : LightText);
                    EndScissorMode();
                }

                DrawRectangle(AlbumX-AlbumOuter,AlbumY+AlbumHeight,AlbumHeight+AlbumOuter,AlbumHeight+u.hinpct(0.01f), WHITE);
                DrawRectangle(AlbumX-AlbumInner,AlbumY+AlbumHeight,AlbumHeight,u.hinpct(0.075f)+AlbumHeight, GetColor(0x181827FF));
                DrawRectangle(AlbumX-AlbumOuter,AlbumY-AlbumInner,AlbumHeight+AlbumOuter,AlbumHeight+AlbumOuter, WHITE);
                DrawRectangle(AlbumX-AlbumInner,AlbumY,AlbumHeight,AlbumHeight, BLACK);


                // bottom
                if (selSong) {
                    DrawTexturePro(selectedSong.albumArt, Rectangle{0, 0, (float) selectedSong.albumArt.width,
                                                                    (float) selectedSong.albumArt.width},
                                   Rectangle{(float)AlbumX-AlbumInner, (float)AlbumY, (float)AlbumHeight, (float)AlbumHeight}, {0, 0}, 0,
                                   WHITE);
                } else {
                    Song art = menu.ChosenSong;
                    DrawTexturePro(art.albumArt, Rectangle{0, 0, (float) art.albumArt.width,
                                                           (float) art.albumArt.width},
                                   Rectangle{(float)AlbumX-AlbumInner, (float)AlbumY, (float)AlbumHeight, (float)AlbumHeight}, {0, 0}, 0,
                                   WHITE);
                }
                // hehe

                float TextPlacementTB = u.TopSide + u.hinpct(0.15f) - u.hinpct(0.11f);
                float TextPlacementLR = u.LeftSide + u.winpct(0.01f);
                DrawTextEx(assets.redHatDisplayBlack, "Song Select", {TextPlacementLR, TextPlacementTB}, u.hinpct(0.10f),0, WHITE);

                menu.DrawBottomOvershell();
                float BottomOvershell = (float)GetScreenHeight() - 120;

                GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(ColorBrightness(player.accentColor, -0.25)));
                if (GuiButton(Rectangle{u.LeftSide, GetScreenHeight() - u.hpct(0.1475f), u.winpct(0.2f), u.hinpct(0.05f)}, "Play Song")) {
                        curPlayingSong = menu.ChosenSongInt;
                        songList.songs[curPlayingSong].LoadSong(songList.songs[curPlayingSong].songInfoPath);
                        menu.SwitchScreen(READY_UP);
                        albumArtLoaded = false;
                }
                GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);

                if (GuiButton(Rectangle{u.LeftSide + u.winpct(0.4f)-2, GetScreenHeight() - u.hpct(0.1475f), u.winpct(0.2f), u.hinpct(0.05f)}, "Sort")) {
                    currentSortValue++;
                    if (currentSortValue == 3) currentSortValue = 0;
                    if (selSong)
                        songList.sortList(currentSortValue, curPlayingSong);
                    else
                        songList.sortList(currentSortValue, menu.ChosenSongInt);
                }
                if (GuiButton(Rectangle{u.LeftSide + u.winpct(0.2f)-1, GetScreenHeight() - u.hpct(0.1475f), u.winpct(0.2f), u.hinpct(0.05f)}, "Back")) {
                    for (Song& songi : songList.songs) {
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
                menu.DrawBottomBottomOvershell();
                break;
            }
            case READY_UP: {

                if (!albumArtLoaded) {
                    selectedSong = songList.songs[curPlayingSong];
                    selectedSong.LoadAlbumArt(selectedSong.albumArtPath);
                    albumArtLoaded = true;
                }
                SetTextureWrap(selectedSong.albumArtBlur, TEXTURE_WRAP_REPEAT);
                SetTextureFilter(selectedSong.albumArtBlur, TEXTURE_FILTER_ANISOTROPIC_16X);
                menu.DrawAlbumArtBackground(selectedSong.albumArtBlur);

                float AlbumArtLeft = u.LeftSide;
                float AlbumArtTop = u.hpct(0.05f);
                float AlbumArtRight = u.winpct(0.15f);
                float AlbumArtBottom = u.winpct(0.15f);
                DrawRectangle(0,0, (int)GetScreenWidth(), (int)GetScreenHeight(), Color(0,0,0,128));

                menu.DrawTopOvershell(0.2f);
                menu.DrawVersion();

                DrawRectangle((int)u.LeftSide,(int)AlbumArtTop,(int)AlbumArtRight+12, (int)AlbumArtBottom+12, WHITE);
                DrawRectangle((int)u.LeftSide + 6,(int)AlbumArtTop+6,(int)AlbumArtRight, (int)AlbumArtBottom,BLACK);
                DrawTexturePro(selectedSong.albumArt, Rectangle{0,0,(float)selectedSong.albumArt.width,(float)selectedSong.albumArt.width}, Rectangle {u.LeftSide + 6, AlbumArtTop+6,AlbumArtRight,AlbumArtBottom}, {0,0}, 0, WHITE);
                


                float BottomOvershell = u.hpct(1)-u.hinpct(0.15f);
                float TextPlacementTB = AlbumArtTop;
                float TextPlacementLR = AlbumArtRight + AlbumArtLeft+ 32;
                DrawTextEx(assets.redHatDisplayBlack, songList.songs[curPlayingSong].title.c_str(), {TextPlacementLR, TextPlacementTB-5}, u.hinpct(0.1f), 0, WHITE);
                DrawTextEx(assets.rubikBoldItalic, selectedSong.artist.c_str(), {TextPlacementLR, TextPlacementTB+u.hinpct(0.09f)}, u.hinpct(0.05f), 0, LIGHTGRAY);
                // todo: allow this to be run per player
                // load midi
                menu.DrawBottomOvershell();
                menu.DrawBottomBottomOvershell();
                if (!midiLoaded) {
                    if (!songList.songs[curPlayingSong].midiParsed) {
                        smf::MidiFile midiFile;
                        midiFile.read(songList.songs[curPlayingSong].midiPath.string());
                        songList.songs[curPlayingSong].getTiming(midiFile, 0, midiFile[0]);
                        for (int i = 0; i < midiFile.getTrackCount(); i++)
                        {
                            std::string trackName;
                            for (int j = 0; j < midiFile[i].getSize(); j++) {
                                if (midiFile[i][j].isMeta()) {
                                    if ((int)midiFile[i][j][1] == 3) {
                                        for (int k = 3; k < midiFile[i][j].getSize(); k++) {
                                            trackName += midiFile[i][j][k];
                                        }
                                        SongParts songPart = song.partFromString(trackName);
                                        if (trackName == "BEAT")
                                            songList.songs[curPlayingSong].parseBeatLines(midiFile, i, midiFile[i]);
                                        else if (trackName == "EVENTS") {
                                            songList.songs[curPlayingSong].getStartEnd(midiFile, i, midiFile[i]);
                                        }
                                        else {
                                            if (songPart != SongParts::Invalid && songPart != SongParts::PlasticDrums) {
                                                for (int diff = 0; diff < 4; diff++) {
                                                    Chart newChart;
                                                    std::cout << trackName << " " << diff << endl;

                                                    if (songPart == SongParts::PlasticBass || songPart == SongParts::PlasticGuitar) {
                                                        newChart.plastic = true;
                                                        newChart.parsePlasticNotes(midiFile, i, midiFile[i], diff,
                                                                                   (int) songPart);
                                                    }
                                                    else {
                                                        newChart.plastic = false;
                                                        newChart.parseNotes(midiFile, i, midiFile[i], diff,
                                                                            (int) songPart);
                                                    }



                                                    if (!newChart.plastic) {
                                                        int noteIdx = 0;
                                                        for (Note &note: newChart.notes) {
                                                            newChart.notes_perlane[note.lane].push_back(noteIdx);
                                                            noteIdx++;
                                                        }
                                                    }
                                                    if (newChart.notes.size() > 0) {
                                                        songList.songs[curPlayingSong].parts[(int) songPart]->hasPart = true;
                                                    }
                                                    songList.songs[curPlayingSong].parts[(int)songPart]->charts.push_back(newChart);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        songList.songs[curPlayingSong].midiParsed = true;
                    }
                    midiLoaded = true;
                    if (player.firstReadyUp || !songList.songs[curPlayingSong].parts[player.instrument]->hasPart) {
                        instSelection = true;
                    }
                    else if (songList.songs[curPlayingSong].parts[player.instrument]->charts[player.diff].notes.size() < 1) {
                        diffSelection = true;
                    }
                    else if (!player.firstReadyUp) {
                        ReadyUpMenu = true;
                    }
                }
                // load instrument select
               
                else if (midiLoaded && instSelection) {
                    if (GuiButton({ 0,0,60,60 }, "<")) {
                        if (player.firstReadyUp || !songList.songs[curPlayingSong].parts[player.instrument]->hasPart) {
                            instSelection = false;
                            diffSelection = false;
                            instSelected = false;
                            diffSelected = false;
                            midiLoaded = false;
                            albumArtSelectedAndLoaded = false;
                            albumArtLoaded = false;
                            menu.SwitchScreen(SONG_SELECT);
                        }
                        else {
                            instSelection = false;
                            ReadyUpMenu = true;
                        }
                        
                    }
                    // DrawTextRHDI(TextFormat("%s - %s", songList.songs[curPlayingSong].title.c_str(), songList.songs[curPlayingSong].artist.c_str()), 70,7, WHITE);
                    for (int i = 0; i < 7; i++) {
                        if (songList.songs[curPlayingSong].parts[i]->hasPart) {
                            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, i == player.instrument && instSelected ? ColorToInt(ColorBrightness(player.accentColor, -0.25)) : 0x181827FF);
                            
                            GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt(Color{ 255,255,255,255 }));
                            GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
                            if (GuiButton({ u.LeftSide,BottomOvershell - u.hinpct(0.05f) - (u.hinpct(0.05f) * (float)i),u.winpct(0.2f),u.hinpct(0.05f) }, TextFormat("  %s", songPartsList[i].c_str()))) {
                                instSelected = true;
                                player.instrument = i;
                                int isBassOrVocal = 0;
                                if (i>3) player.plastic = true;
                                else player.plastic = false;
                                
                                if (player.instrument == 1 || player.instrument == 3 || player.instrument == 5) {
                                    isBassOrVocal = 1;
                                }
                                SetShaderValue(assets.odMultShader, assets.isBassOrVocalLoc, &isBassOrVocal, SHADER_UNIFORM_INT);
                            }
                            GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0xcbcbcbFF);
                            GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
                            DrawTextRubik((std::to_string(songList.songs[curPlayingSong].parts[i]->diff + 1) + "/7").c_str(), u.LeftSide + u.winpct(0.165f), BottomOvershell - u.hinpct(0.04f) - (u.hinpct(0.05f) * (float)i), u.hinpct(0.03f), WHITE);
                        } else {

                            GuiButton({ u.LeftSide,BottomOvershell - 60 - (60 * (float)i),u.winpct(0.2f),60 }, "");
                            DrawRectangle( u.LeftSide+2,BottomOvershell+2 - 60 - (60 * (float)i), u.winpct(0.2f) -4,60-4, Color{0,0,0,128});
                        }
                        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
                        if (instSelected) {

                            GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt(Color{ 255,255,255,255 }));
                            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x1D754AFF);
                            GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, 0x2AA86BFF);
                            if (GuiButton({ u.LeftSide, BottomOvershell, u.winpct(0.2f), u.hinpct(0.05f) }, "Done")) {
                                if (player.firstReadyUp) {
                                    instSelection = false;
                                    diffSelection = true;
                                }
                                else if (songList.songs[curPlayingSong].parts[player.instrument]->charts[player.diff].notes.size() < 1) {
                                    instSelection = false;
                                    diffSelection = true;
                                }
                                else {
                                    instSelection = false;
                                    ReadyUpMenu = true;
                                }
                            }
                            GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt(ColorBrightness(player.accentColor, -0.5)));
                            GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0xcbcbcbFF);
                            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
                        }
                    }
                }
                // load difficulty select
                if (midiLoaded && diffSelection) {
                    
                    for (int a = 0; a < 4; a++) {
                        if (songList.songs[curPlayingSong].parts[player.instrument]->charts[a].notes.size() > 0) {
                            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, a == player.diff && diffSelected ? ColorToInt(ColorBrightness(player.accentColor, -0.25)) : 0x181827FF);
                            if (GuiButton({ u.LeftSide,BottomOvershell - u.hinpct(0.05f) - (u.hinpct(0.05f) * (float)a),u.winpct(0.2f),u.hinpct(0.05f) }, diffList[a].c_str())) {
                                player.diff = a;
                                diffSelected = true;
                                isPlaying = true;
                                startedPlayingSong = GetTime();
                                
                            }
                        } else {
                            GuiButton({ u.LeftSide,BottomOvershell - u.hinpct(0.05f) - (u.hinpct(0.05f) * (float)a),u.winpct(0.2f),u.hinpct(0.05f) }, "");
                            DrawRectangle( u.LeftSide+2,BottomOvershell+2 - u.hinpct(0.05f) - (u.hinpct(0.05f) * (float)a), u.winpct(0.2f) - 4, u.hinpct(0.05f) - 4, Color{0,0,0,128});
                        }
                        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
                        if (diffSelected) {
                            GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt(Color{ 255,255,255,255 }));
                            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x1D754AFF);
                            GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, 0x2AA86BFF);
                            if (GuiButton({ u.LeftSide, BottomOvershell- u.hinpct(0.25f), u.winpct(0.2f), u.hinpct(0.05f)}, "Done")) {
                                diffSelection = false;
                                ReadyUpMenu = true;
                                player.firstReadyUp = false;
                            }
                            GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt(ColorBrightness(player.accentColor, -0.5)));
                            GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0xcbcbcbFF);
                            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
                        }
                        if (GuiButton({ 0,0,60,60 }, "<")) {
                            if (player.firstReadyUp || !songList.songs[curPlayingSong].parts[player.instrument]->hasPart) {
                                instSelection = true;
                                diffSelection = false;
                                instSelected = false;
                                diffSelected = false;
                            }
                            else {
                                instSelection = false;
                                diffSelection = false;
                                instSelected = false;
                                diffSelected = false;
                                ReadyUpMenu = true;
                            }
                        }
                    }
                }
                
                if (midiLoaded && ReadyUpMenu) {
                    
                    if (GuiButton({ u.LeftSide,BottomOvershell - u.hinpct(0.05f) ,u.winpct(0.2f),u.hinpct(0.05f) }, "")) {
                            ReadyUpMenu = false;
                            diffSelection = true;
                    }
                    DrawTextRubik("  Difficulty", u.LeftSide, BottomOvershell - u.hinpct(0.04f), u.hinpct(0.03f), WHITE);
                    DrawTextEx(assets.rubikBold, diffList[player.diff].c_str(), {u.LeftSide + u.winpct(0.19f) - MeasureTextEx(assets.rubikBold, diffList[player.diff].c_str(),u.hinpct(0.03f),0).x, BottomOvershell - u.hinpct(0.04f) }, u.hinpct(0.03f), 0, WHITE);

                    if (GuiButton({ u.LeftSide,BottomOvershell - u.hinpct(0.10f) ,u.winpct(0.2f),u.hinpct(0.05f) }, "")) {
                        ReadyUpMenu = false;
                        instSelection = true;
                    }
                    DrawTextRubik("  Instrument", u.LeftSide, BottomOvershell - u.hinpct(0.09f), u.hinpct(0.03f), WHITE);
                    DrawTextEx(assets.rubikBold, songPartsList[player.instrument].c_str(), { u.LeftSide + u.winpct(0.19f) - MeasureTextEx(assets.rubikBold, songPartsList[player.instrument].c_str(),u.hinpct(0.03f),0).x, BottomOvershell - u.hinpct(0.09f) }, u.hinpct(0.03f), 0, WHITE);

                    GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt(Color{ 255,255,255,255 }));
                    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x1D754AFF);
                    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, 0x2AA86BFF);
                    if (GuiButton({ u.LeftSide, BottomOvershell, u.winpct(0.2f), u.hinpct(0.05f)}, "Ready Up!")) {
                        ReadyUpMenu = false;
                        menu.SwitchScreen(GAMEPLAY);
                        glfwSetKeyCallback(glfwGetCurrentContext(), keyCallback);
                        glfwSetGamepadStateCallback(gamepadStateCallback);
                    }
                    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt(ColorBrightness(player.accentColor, -0.5)));
                    GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0xcbcbcbFF);
                    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
                        
                        if (GuiButton({ 0,0,60,60 }, "<")) {
                            midiLoaded = false;
                            instSelection = false;
                            diffSelection = false;
                            instSelected = false;
                            diffSelected = false;
                            ReadyUpMenu = false;
                            menu.SwitchScreen(SONG_SELECT);
                        }
                    
                }
                break;
            }
            case GAMEPLAY: {
                // IMAGE BACKGROUNDS??????
                ClearBackground(BLACK);
                player.songToBeJudged = songList.songs[curPlayingSong];

                if (IsWindowResized() || notes_tex.texture.width != GetScreenWidth() || notes_tex.texture.height != GetScreenHeight()) {
                    UnloadRenderTexture(notes_tex);
                    RenderTexture2D notes_tex = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
                    GenTextureMipmaps(&notes_tex.texture);
                    SetTextureFilter(notes_tex.texture, TEXTURE_FILTER_ANISOTROPIC_4X);

                    UnloadRenderTexture(hud_tex);
                    RenderTexture2D hud_tex = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
                    GenTextureMipmaps(&hud_tex.texture);
                    SetTextureFilter(hud_tex.texture, TEXTURE_FILTER_ANISOTROPIC_4X);

                    UnloadRenderTexture(highway_tex);
                    RenderTexture2D highway_tex = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
                    GenTextureMipmaps(&highway_tex.texture);
                    SetTextureFilter(highway_tex.texture, TEXTURE_FILTER_ANISOTROPIC_4X);

                    UnloadRenderTexture(highwayStatus_tex);
                    RenderTexture2D highwayStatus_tex = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
                    GenTextureMipmaps(&highwayStatus_tex.texture);
                    SetTextureFilter(highwayStatus_tex.texture, TEXTURE_FILTER_ANISOTROPIC_4X);

                    UnloadRenderTexture(smasher_tex);
                    RenderTexture2D smasher_tex = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
                    GenTextureMipmaps(&smasher_tex.texture);
                    SetTextureFilter(smasher_tex.texture, TEXTURE_FILTER_ANISOTROPIC_4X);
                }

                float scorePos = u.RightSide;
                float scoreY = u.hpct(0.15f);
                float starY = scoreY + u.hinpct(0.05f);
                float comboY = starY + u.hinpct(0.055f);
                if (!songAlbumArtLoadedGameplay) {
                    menu.ChosenSong.LoadAlbumArt(menu.ChosenSong.albumArtPath);
                    songAlbumArtLoadedGameplay = true;
                }

                menu.DrawAlbumArtBackground(menu.ChosenSong.albumArtBlur);
                DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(), Color{0,0,0,128});
                // DrawTextureEx(assets.songBackground, {0,0},0, (float)GetScreenHeight()/assets.songBackground.height,WHITE);

                int starsval = player.stars(songList.songs[curPlayingSong].parts[player.instrument]->charts[player.diff].baseScore,player.diff);
                float starPercent = (float)player.score/(float)player.songToBeJudged.parts[player.instrument]->charts[player.diff].baseScore;
                for (int i = 0; i < 5; i++) {
                    bool firstStar = (i == 0);
                    float starX = scorePos - u.hinpct(0.26) + (i*u.hinpct(0.05));
                    float starWH = u.hinpct(0.05);
                    Rectangle emptyStarWH = {0,0,(float)assets.emptyStar.width, (float)assets.emptyStar.height};
                    Rectangle starRect = {starX, starY, starWH,starWH};

                    DrawTexturePro(assets.emptyStar, emptyStarWH, starRect, {0,0},0, player.accentColor);
                    float yMaskPos = Remap(starPercent, firstStar ? 0 : player.xStarThreshold[i-1], player.xStarThreshold[i], 0, u.hinpct(0.05));
                    BeginScissorMode(starX, (starY+starWH)-yMaskPos, starWH, yMaskPos);
                    DrawTexturePro(assets.star, emptyStarWH, starRect, {0,0},0,i != starsval ? player.accentColor : Color{192,192,192,128});
                    EndScissorMode();
                }

                if (starPercent >= player.xStarThreshold[4]) {

                    float starWH = u.hinpct(0.05);
                    Rectangle emptyStarWH = {0,0,(float)assets.goldStar.width, (float)assets.goldStar.height};


                    float yMaskPos = Remap(starPercent, player.xStarThreshold[4], player.xStarThreshold[5], 0, u.hinpct(0.05));
                    BeginScissorMode(scorePos - (starWH*6), (starY+starWH)-yMaskPos, scorePos, yMaskPos);
                    for (int i = 0; i < 5; i++) {
                        float starX = scorePos - u.hinpct(0.26) + (i*u.hinpct(0.05));
                        Rectangle starRect = {starX, starY, starWH,starWH};
                        DrawTexturePro(player.goldStars ? assets.goldStar : assets.goldStarUnfilled, emptyStarWH, starRect,
                                       {0, 0}, 0, WHITE);
                    }
                    EndScissorMode();
                }



                int totalScore = player.score + player.sustainScoreBuffer[0] + player.sustainScoreBuffer[1] + player.sustainScoreBuffer[2] + player.sustainScoreBuffer[3] + player.sustainScoreBuffer[4];

                DrawTextRHDI(scoreCommaFormatter(totalScore).c_str(), u.RightSide - u.winpct(0.01f) - MeasureTextRHDI(scoreCommaFormatter(totalScore).c_str(), u.hinpct(0.05f)), scoreY, u.hinpct(0.05f), Color{107, 161, 222,255});
                DrawTextRHDI(scoreCommaFormatter(player.combo).c_str(), u.RightSide - u.winpct(0.01f) - MeasureTextRHDI(scoreCommaFormatter(player.combo).c_str(), u.hinpct(0.05f)), comboY, u.hinpct(0.05f), player.FC ? GOLD : (player.combo <= 3) ? RED : WHITE);
                

                if (player.extraGameplayStats) {
                    DrawTextRubik(TextFormat("Perfect Hit: %01i", player.perfectHit), 5, GetScreenHeight() - 280, 24,
                                  (player.perfectHit > 0) ? GOLD : WHITE);
                    DrawTextRubik(TextFormat("Max Combo: %01i", player.maxCombo), 5, GetScreenHeight() - 130, 24, WHITE);
                    DrawTextRubik(TextFormat("Multiplier: %01i", player.multiplier(player.instrument)), 5, GetScreenHeight() - 100, 24,
                                  (player.multiplier(player.instrument) >= 4) ? SKYBLUE : WHITE);
                    DrawTextRubik(TextFormat("Notes Hit: %01i", player.notesHit), 5, GetScreenHeight() - 250, 24, player.FC ? GOLD : WHITE);
                    DrawTextRubik(TextFormat("Notes Missed: %01i", player.notesMissed), 5, GetScreenHeight() - 220, 24,
                                  ((player.combo == 0) && (!player.FC)) ? RED : WHITE);
                    DrawTextRubik(TextFormat("Strikes: %01i", player.playerOverhits), 5, GetScreenHeight() - 40, 24, player.FC ? GOLD : WHITE);
                }

                if (!streamsLoaded && !player.quit) {
                    audioManager.loadStreams(songList.songs[curPlayingSong].stemsPath);
                    streamsLoaded = true;
                    for (auto& stream : audioManager.loadedStreams) {
                        if ((player.plastic ? player.instrument -4 : player.instrument) == stream.instrument)
                            audioManager.SetAudioStreamVolume(stream.handle, player.mute ? player.missVolume : settingsMain.MainVolume * settingsMain.PlayerVolume);
                        else
                            audioManager.SetAudioStreamVolume(stream.handle, settingsMain.MainVolume * settingsMain.BandVolume);

                    }
                    audioManager.BeginPlayback(audioManager.loadedStreams[0].handle);
                    player.resetPlayerStats();
                }
                else {
                    for (auto& stream : audioManager.loadedStreams) {
                        if ((player.plastic ? player.instrument -4 : player.instrument)  == stream.instrument)
                            audioManager.SetAudioStreamVolume(stream.handle, player.mute ? player.missVolume : settingsMain.MainVolume * settingsMain.PlayerVolume);
                        else
                            audioManager.SetAudioStreamVolume(stream.handle, settingsMain.MainVolume * settingsMain.BandVolume);

                    }
                    float songPlayed = audioManager.GetMusicTimePlayed(audioManager.loadedStreams[0].handle);
                    double songEnd = songList.songs[curPlayingSong].end == 0 ? audioManager.GetMusicTimeLength(audioManager.loadedStreams[0].handle) : songList.songs[curPlayingSong].end;
                    if (songEnd < songPlayed) {
                        glfwSetKeyCallback(glfwGetCurrentContext(), origKeyCallback);
                        glfwSetGamepadStateCallback(origGamepadCallback);
                        // notes = (int)songList.songs[curPlayingSong].parts[instrument]->charts[diff].notes.size();
                        player.overdrive = false;
                        player.overdriveFill = 0.0f;
                        player.overdriveActiveFill = 0.0f;
                        player.overdriveActiveTime = 0.0;
                        gpr.curODPhrase = 0;
                        gpr.curNoteInt = 0;
                        gpr.curSolo = 0;
                        menu.ChosenSong.LoadAlbumArt(menu.ChosenSong.albumArtPath);
                        midiLoaded = false;
                        isPlaying = false;
                        gpr.songEnded = true;
                        songList.songs[curPlayingSong].parts[player.instrument]->charts[player.diff].resetNotes();

                        assets.expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = assets.highwayTexture;
                        assets.emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = assets.highwayTexture;
                        assets.multBar.materials[0].maps[MATERIAL_MAP_EMISSION].texture = assets.odMultFill;
                        assets.multCtr3.materials[0].maps[MATERIAL_MAP_EMISSION].texture = assets.odMultFill;
                        assets.multCtr5.materials[0].maps[MATERIAL_MAP_EMISSION].texture = assets.odMultFill;
                        assets.expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = player.accentColor;
                        assets.emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = player.accentColor;
                        menu.SwitchScreen(RESULTS);
                        TraceLog(LOG_INFO, TextFormat("Song ended at at %f", songPlayed));
                    }

                }

                int songPlayed = audioManager.GetMusicTimePlayed(audioManager.loadedStreams[0].handle);
                double songFloat = audioManager.GetMusicTimePlayed(audioManager.loadedStreams[0].handle);
                player.notes = (int)songList.songs[curPlayingSong].parts[player.instrument]->charts[player.diff].notes.size();

                gpr.RenderGameplay(player, songFloat, songList.songs[curPlayingSong], highway_tex, hud_tex, notes_tex, highwayStatus_tex, smasher_tex);



                if (curTime < startedPlayingSong + 7.5) {
                    DrawTextEx(assets.rubikBoldItalic, songList.songs[curPlayingSong].title.c_str(), {25, (float)((GetScreenHeight()/3)*2) - u.hpct(0.08f)}, u.hpct(0.04f), 0, WHITE);
                    DrawTextEx(assets.rubikItalic, songList.songs[curPlayingSong].artist.c_str(), {35, (float)((GetScreenHeight()/3)*2) - u.hpct(0.04f)}, u.hpct(0.04f), 0, LIGHTGRAY);
                }

                int songLength = songList.songs[curPlayingSong].end == 0 ? audioManager.GetMusicTimeLength(audioManager.loadedStreams[0].handle) : songList.songs[curPlayingSong].end;
                int playedMinutes = songPlayed/60;
                int playedSeconds = songPlayed % 60;
                int songMinutes = songLength/60;
                int songSeconds = songLength % 60;

                GuiSetStyle(PROGRESSBAR, BORDER_WIDTH, 0);
                GuiSetStyle(PROGRESSBAR, BASE_COLOR_NORMAL, ColorToInt(player.FC ? GOLD : player.accentColor));
                GuiSetStyle(PROGRESSBAR, BASE_COLOR_FOCUSED, ColorToInt(player.FC ? GOLD : player.accentColor));
                GuiSetStyle(PROGRESSBAR, BASE_COLOR_DISABLED, ColorToInt(player.FC ? GOLD : player.accentColor));
                GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED, ColorToInt(player.FC ? GOLD : player.accentColor));
                GuiSetStyle(DEFAULT, TEXT_SIZE, (int)u.hinpct(0.03f));
                GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
                GuiSetFont(assets.rubik);

                float floatSongLength = audioManager.GetMusicTimePlayed(audioManager.loadedStreams[0].handle);

                const char* textTime = TextFormat("%i:%02i / %i:%02i ", playedMinutes,playedSeconds,songMinutes,songSeconds);
                float textLength = MeasureTextEx(assets.rubik, textTime, u.hinpct(0.04f), 0).x;

                if (player.paused) {

                    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{ 0,0,0,80 });
                    menu.DrawTopOvershell(0.2f);
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
                    
                    if (GuiButton({ u.wpct(0.02f), u.hpct(0.3f), u.winpct(0.2f), u.hinpct(0.08f) }, "Resume")) {

                        audioManager.unpauseStreams();
                        player.paused = false;
                    }
                    
                    if (GuiButton({ u.wpct(0.02f), u.hpct(0.39f), u.winpct(0.2f), u.hinpct(0.08f) },
                        "Restart")) {
                        for (Note& note : songList.songs[curPlayingSong].parts[player.instrument]->charts[player.diff].notes) {
                            note.accounted = false;
                            note.hit = false;
                            note.miss = false;
                            note.held = false;
                            note.heldTime = 0;
                            note.hitTime = 0;
                            note.perfect = false;
                            note.countedForODPhrase = false;
                            note.hitWithFAS = false;
                            note.strumCount = 0;
                        }
                        for (odPhrase& phrase : songList.songs[curPlayingSong].parts[player.instrument]->charts[player.diff].odPhrases) {
                            phrase.missed = false;
                            phrase.notesHit = 0;
                            phrase.added = false;
                        }
                        for (solo& solo : songList.songs[curPlayingSong].parts[player.instrument]->charts[player.diff].Solos) {
                            solo.notesHit = 0;
                        }
                        player.overdrive = false;
                        player.overdriveFill = 0.0f;
                        player.overdriveActiveFill = 0.0f;
                        player.overdriveActiveTime = 0.0;
                        gpr.curODPhrase = 0;
                        gpr.curNoteInt = 0;
                        gpr.curSolo = 0;
                        gpr.curNoteIdx = { 0,0,0,0,0 };
                        gpr.curBeatLine = 0;
                        player.resetPlayerStats();
                        assets.expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = assets.highwayTexture;
                        assets.emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = assets.highwayTexture;
                        assets.multBar.materials[0].maps[MATERIAL_MAP_EMISSION].texture = assets.odMultFill;
                        assets.multCtr3.materials[0].maps[MATERIAL_MAP_EMISSION].texture = assets.odMultFill;
                        assets.multCtr5.materials[0].maps[MATERIAL_MAP_EMISSION].texture = assets.odMultFill;
                        assets.expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = DARKGRAY;
                        assets.emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = DARKGRAY;
                        for (auto& stream : audioManager.loadedStreams) {
                            audioManager.restartStreams();
                            player.paused = false;
                        }

                        startedPlayingSong = GetTime();
                    }
                    if (GuiButton({ u.wpct(0.02f), u.hpct(0.48f), u.winpct(0.2f), u.hinpct(0.08f) }, "Drop Out")) {
                        glfwSetKeyCallback(glfwGetCurrentContext(), origKeyCallback);
                        glfwSetGamepadStateCallback(origGamepadCallback);
                        // notes = songList.songs[curPlayingSong].parts[instrument]->charts[diff].notes.size();
                        // notes = songList.songs[curPlayingSong].parts[instrument]->charts[diff];
                        menu.SwitchScreen(RESULTS);
                        menu.ChosenSong.LoadAlbumArt(menu.ChosenSong.albumArtPath);
                        player.overdrive = false;
                        player.overdriveFill = 0.0f;
                        player.overdriveActiveFill = 0.0f;
                        player.overdriveActiveTime = 0.0;
                        gpr.curNoteInt = 0;
                        gpr.curODPhrase = 0;
                        gpr.curSolo = 0;
                        player.paused = false;
                        assets.expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = assets.highwayTexture;
                        assets.emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = assets.highwayTexture;
                        assets.multBar.materials[0].maps[MATERIAL_MAP_EMISSION].texture = assets.odMultFill;
                        assets.multCtr3.materials[0].maps[MATERIAL_MAP_EMISSION].texture = assets.odMultFill;
                        assets.multCtr5.materials[0].maps[MATERIAL_MAP_EMISSION].texture = assets.odMultFill;
                        assets.expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = DARKGRAY;
                        assets.emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = DARKGRAY;
                        midiLoaded = false;
                        isPlaying = false;
                        gpr.songEnded = true;
                        songList.songs[curPlayingSong].parts[player.instrument]->charts[player.diff].resetNotes();
                        player.quit = true;
                        songAlbumArtLoadedGameplay = false;
                    }

                    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
                    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt(ColorBrightness(Color{ 255,0,255,255 }, -0.5)));
                    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, ColorToInt(ColorBrightness(Color{ 255,0,255,255 }, -0.3)));
                    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, 0xFFFFFFFF);
                    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, 0xFFFFFFFF);
                    GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, 0xFFFFFFFF);
                    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0x505050ff);
                    GuiSetStyle(BUTTON, BORDER_WIDTH, 2);
                    GuiSetFont(assets.rubik);
                    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
                    
                    DrawTextEx(assets.rubikBoldItalic, "PAUSED", {u.wpct(0.02f), u.hpct(0.05f)}, u.hinpct(0.1f), 0, WHITE);

                    float SongFontSize = u.hinpct(0.03f);
                    
                    const char* instDiffText = TextFormat("%s %s", diffList[player.diff].c_str(), songPartsList[player.instrument].c_str());

                    float TitleHeight = MeasureTextEx(assets.rubikBoldItalic, menu.ChosenSong.title.c_str(), SongFontSize, 0).y;
                    float TitleWidth = MeasureTextEx(assets.rubikBoldItalic, menu.ChosenSong.title.c_str(), SongFontSize, 0).x;
                    float ArtistHeight = MeasureTextEx(assets.rubikItalic, menu.ChosenSong.artist.c_str(), SongFontSize, 0).y;
                    float ArtistWidth = MeasureTextEx(assets.rubikItalic, menu.ChosenSong.artist.c_str(), SongFontSize, 0).x;
                    float InstDiffHeight = MeasureTextEx(assets.rubikBold, instDiffText, SongFontSize, 0).y;
                    float InstDiffWidth = MeasureTextEx(assets.rubikBold, instDiffText, SongFontSize, 0).x;

                    Vector2 SongTitleBox = { u.RightSide - TitleWidth - u.winpct(0.01f),  u.hpct(0.1f) - (ArtistHeight/2) - (TitleHeight * 1.1f)};
                    Vector2 SongArtistBox = { u.RightSide - ArtistWidth - u.winpct(0.01f),  u.hpct(0.1f) - (ArtistHeight / 2) };
                    Vector2 SongInstDiffBox = { u.RightSide - InstDiffWidth - u.winpct(0.01f),  u.hpct(0.1f) + (ArtistHeight / 2) + (InstDiffHeight*0.1f)};

                    DrawTextEx(assets.rubikBoldItalic, menu.ChosenSong.title.c_str(), SongTitleBox, SongFontSize, 0, WHITE);
                    DrawTextEx(assets.rubikItalic, menu.ChosenSong.artist.c_str(), SongArtistBox, SongFontSize, 0, WHITE);
                    DrawTextEx(assets.rubikBold, instDiffText, SongInstDiffBox, SongFontSize, 0, WHITE);
                }


                menu.DrawFPS(u.LeftSide,u.hpct(0.0025f) + u.hinpct(0.025f));
                menu.DrawVersion();

                DrawTextEx(assets.rubik, textTime, { GetScreenWidth() - textLength,GetScreenHeight() - u.hinpct(0.05f) }, u.hinpct(0.04f), 0, WHITE);
                DrawTextEx(assets.rubikBold, TextFormat("%s", player.FC ? "FC" : ""), { 5, GetScreenHeight() - u.hinpct(0.05f) }, u.hinpct(0.04), 0, GOLD);
                GuiProgressBar(Rectangle{ 0,(float)GetScreenHeight() - u.hinpct(0.005f),(float)GetScreenWidth(),u.hinpct(0.01f) }, "", "", & floatSongLength, 0, (float)songLength);
                DrawText(to_string(HeldMaskShow).c_str(), 0, 200, 30, WHITE);
                DrawText(to_string(gpr.curNoteInt).c_str(), 0, 240, 30, WHITE);

                DrawRectangle(u.wpct(0.5f)-(u.winpct(0.12f)/2),u.hpct(0.02f) - u.winpct(0.01f), u.winpct(0.12f),u.winpct(0.065f),DARKGRAY);

                for (int fretBox = 0; fretBox < gpr.heldFrets.size(); fretBox++) {
                    float leftInputBoxSize = (5 * u.winpct(0.02f))/2;

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

                    DrawRectangle(u.wpct(0.5f)-leftInputBoxSize+(fretBox * u.winpct(0.02f)),u.hpct(0.02f), u.winpct(0.02f),u.winpct(0.02f),gpr.heldFrets[fretBox]?fretColor:GRAY);

                }
                DrawRectangle(u.wpct(0.5f)-((5 * u.winpct(0.02f))/2),u.hpct(0.02f) + u.winpct(0.025f), u.winpct(0.1f),u.winpct(0.01f),gpr.upStrum?WHITE:GRAY);
                DrawRectangle(u.wpct(0.5f)-((5 * u.winpct(0.02f))/2),u.hpct(0.02f) + u.winpct(0.035f), u.winpct(0.1f),u.winpct(0.01f),gpr.downStrum?WHITE:GRAY);

                break;

            }
            case RESULTS: {
                
                if (streamsLoaded) {
                    audioManager.unloadStreams();
                    streamsLoaded = false;
                }


                menu.DrawAlbumArtBackground(menu.ChosenSong.albumArtBlur);
                menu.showResults(player);
                if (GuiButton({ 0,0,60,60 }, "<")) {
                    player.quit = false;
                    menu.SwitchScreen(SONG_SELECT);
                }
                break;
            }
        }
        EndDrawing();

        //SwapScreenBuffer();

        currentTime = GetTime();
        updateDrawTime = currentTime - previousTime;

        if (targetFPS > 0)
        {
            waitTime = (1.0f / (float)targetFPS) - updateDrawTime;
            if (waitTime > 0.0)
            {
                WaitTime((float)waitTime);
                currentTime = GetTime();
                deltaTime = (float)(currentTime - previousTime);
            }
        }
        else deltaTime = (float)updateDrawTime;

        previousTime = currentTime;
    }
    CloseWindow();
    return 0;
}

//
// Created by marie on 02/05/2024.
//

#include "rhythmLogic.h"
#include "raylib.h"
#include "player.cpp"
#include "song/songlist.h"
#include "settings.h"
#include "song/song.h"
#include "assets.h"

SongList songList;
Settings settings;


void RhythmLogic::handleInputs(int lane, int action) {
        if (lane == -2) return;
        if (settings.mirrorMode && lane != -1) {
            lane = (diff == 3 ? 4 : 3) - lane;
        }
        if (!streamsLoaded) {
            return;
        }
        Chart &curChart = songList.songs[curPlayingSong].parts[instrument]->charts[diff];
        float eventTime = GetMusicTimePlayed(loadedStreams[0].first);
        if (action == GLFW_PRESS && (lane == -1) && overdriveFill > 0 && !overdrive) {
            overdriveActiveTime = eventTime;
            overdriveActiveFill = overdriveFill;
            overdrive = true;
            overdriveHitAvailable = true;
            overdriveHitTime = eventTime;
        }
        if (lane == -1) {
            if ((action == GLFW_PRESS && !overdriveHitAvailable) ||
                (action == GLFW_RELEASE && !overdriveLiftAvailable))
                return;
            Note *curNote = &curChart.notes[0];
            for (auto &note: curChart.notes) {
                if (note.time - (goodBackend + InputOffset) < eventTime &&
                    note.time + (goodFrontend + InputOffset) > eventTime &&
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
                if (curNote->time - (goodBackend + InputOffset) < eventTime &&
                    curNote->time + (goodFrontend + InputOffset) > eventTime &&
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
                                if ((chordNote.time) - perfectBackend + InputOffset < eventTime &&
                                    chordNote.time + perfectFrontend + InputOffset > eventTime) {
                                    chordNote.perfect = true;

                                }
                                if (chordNote.perfect) lastNotePerfect = true;
                                else lastNotePerfect = false;
                                player::HitNote(chordNote.perfect, instrument);
                                chordNote.accounted = true;
                            }
                        }
                    }
                    overdriveHitAvailable = false;
                    overdriveLiftAvailable = true;
                }
            } else if (action == GLFW_RELEASE && overdriveLiftAvailable) {
                if ((curNote->time) - (goodBackend * liftTimingMult) + InputOffset < eventTime &&
                    (curNote->time) + (goodFrontend * liftTimingMult) + InputOffset > eventTime &&
                    !curNote->hit) {
                    for (int newlane = 0; newlane < 5; newlane++) {
                        if (overdriveLanesHit[newlane]) {
                            int chordLane = curChart.findNoteIdx(curNote->time, newlane);
                            if (chordLane != -1) {
                                Note &chordNote = curChart.notes[chordLane];
                                if (chordNote.lift) {
                                    chordNote.hit = true;
                                    overdriveLanesHit[newlane] = false;
                                    chordNote.hitTime = eventTime;

                                    if ((chordNote.time) - perfectBackend + InputOffset < eventTime &&
                                        chordNote.time + perfectFrontend + InputOffset > eventTime) {
                                        chordNote.perfect = true;

                                    }
                                    if (chordNote.perfect) lastNotePerfect = true;
                                    else lastNotePerfect = false;
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
                                if (!((diff == 3 && settings.keybinds5K[chordNote.lane]) ||
                                      (diff != 3 && settings.keybinds4K[chordNote.lane]))) {
                                    chordNote.held = false;
                                    score += sustainScoreBuffer[chordNote.lane];
                                    sustainScoreBuffer[chordNote.lane] = 0;
                                    mute = true;
                                }
                            }
                        }
                    }
                }
            }
        } else {
            for (int i = curNoteIdx[lane]; i < curChart.notes_perlane[lane].size(); i++) {
                Note &curNote = curChart.notes[curChart.notes_perlane[lane][i]];

                if (lane != curNote.lane) continue;
                if ((curNote.lift && action == GLFW_RELEASE) || action == GLFW_PRESS) {
                    if ((curNote.time) - (action == GLFW_RELEASE ? goodBackend * liftTimingMult : goodBackend) +
                        InputOffset < eventTime &&
                        (curNote.time) +
                        ((action == GLFW_RELEASE ? goodFrontend * liftTimingMult : goodFrontend) + InputOffset) >
                        eventTime &&
                        !curNote.hit) {
                        if (curNote.lift && action == GLFW_RELEASE) {
                            lastHitLifts[lane] = curChart.notes_perlane[lane][i];
                        }
                        curNote.hit = true;
                        curNote.hitTime = eventTime;
                        if ((curNote.len) > 0 && !curNote.lift) {
                            curNote.held = true;
                        }
                        if ((curNote.time) - perfectBackend + InputOffset < eventTime &&
                            curNote.time + perfectFrontend + InputOffset > eventTime) {
                            curNote.perfect = true;
                        }
                        if (curNote.perfect) lastNotePerfect = true;
                        else lastNotePerfect = false;
                        player::HitNote(curNote.perfect, instrument);
                        curNote.accounted = true;
                        break;
                    }
                    if (curNote.miss) lastNotePerfect = false;
                }
                if (action == GLFW_RELEASE && curNote.held && (curNote.len) > 0) {
                    curNote.held = false;
                    score += sustainScoreBuffer[curNote.lane];
                    sustainScoreBuffer[curNote.lane] = 0;
                    mute = true;
                    // SetAudioStreamVolume(loadedStreams[instrument].stream, missVolume);
                }

                if (action == GLFW_PRESS &&
                    eventTime > songList.songs[curPlayingSong].music_start &&
                    !curNote.hit &&
                    !curNote.accounted &&
                    ((curNote.time) - perfectBackend) + InputOffset > eventTime &&
                    eventTime > overdriveHitTime + 0.05
                    && !overhitFrets[lane]) {
                    if (lastHitLifts[lane] != -1) {
                        if (eventTime > curChart.notes[lastHitLifts[lane]].time - 0.1 &&
                            eventTime < curChart.notes[lastHitLifts[lane]].time + 0.1)
                            continue;
                    }
                    player::OverHit();
                    if (curChart.odPhrases.empty() && eventTime >= curChart.odPhrases[curODPhrase].start &&
                        eventTime < curChart.odPhrases[curODPhrase].end && !curChart.odPhrases[curODPhrase].missed)
                        curChart.odPhrases[curODPhrase].missed = true;
                    overhitFrets[lane] = true;
                }
            }
        }

};


// what to check when a key changes states (what was the change? was it pressed? or released? what time? what window? were any modifiers pressed?)
void RhythmLogic::keyCallback(GLFWwindow *wind, int key, int scancode, int action, int mods) {
        if (!streamsLoaded) {
            return;
        }
        if (action < 2) {  // if the key action is NOT repeat (release is 0, press is 1)
            int lane = -2;
            if (key == settings.keybindOverdrive || key == settings.keybindOverdriveAlt) {
                handleInputs(-1, action);
            } else {
                if (diff == 3) {
                    for (int i = 0; i < 5; i++) {
                        if (key == settings.keybinds5K[i] && !heldFretsAlt[i]) {
                            if (action == GLFW_PRESS) {
                                heldFrets[i] = true;
                            } else if (action == GLFW_RELEASE) {
                                heldFrets[i] = false;
                                overhitFrets[i] = false;
                            }
                            lane = i;
                        } else if (key == settings.keybinds5KAlt[i] && !heldFrets[i]) {
                            if (action == GLFW_PRESS) {
                                heldFretsAlt[i] = true;
                            } else if (action == GLFW_RELEASE) {
                                heldFretsAlt[i] = false;
                                overhitFrets[i] = false;
                            }
                            lane = i;
                        }
                    }
                } else {
                    for (int i = 0; i < 4; i++) {
                        if (key == settings.keybinds4K[i] && !heldFretsAlt[i]) {
                            if (action == GLFW_PRESS) {
                                heldFrets[i] = true;
                            } else if (action == GLFW_RELEASE) {
                                heldFrets[i] = false;
                                overhitFrets[i] = false;
                            }
                            lane = i;
                        } else if (key == settings.keybinds4KAlt[i] && !heldFrets[i]) {
                            if (action == GLFW_PRESS) {
                                heldFretsAlt[i] = true;
                            } else if (action == GLFW_RELEASE) {
                                heldFretsAlt[i] = false;
                                overhitFrets[i] = false;
                            }
                            lane = i;
                        }
                    }
                }
                if (lane > -1) {
                    handleInputs(lane, action);
                }

            }
        }
    };

void RhythmLogic::gamepadStateCallback(int jid, GLFWgamepadstate state) {
        if (!streamsLoaded) {
            return;
        }
        double eventTime = GetMusicTimePlayed(loadedStreams[0].first);
        if (settings.controllerOverdrive >= 0) {
            if (state.buttons[settings.controllerOverdrive] != buttonValues[settings.controllerOverdrive]) {
                buttonValues[settings.controllerOverdrive] = state.buttons[settings.controllerOverdrive];
                handleInputs(-1, state.buttons[settings.controllerOverdrive]);
            }
        } else {
            if (state.axes[-(settings.controllerOverdrive + 1)] != axesValues[-(settings.controllerOverdrive + 1)]) {
                axesValues[-(settings.controllerOverdrive + 1)] = state.axes[-(settings.controllerOverdrive + 1)];
                if (1.0f * settings.controllerOverdriveAxisDirection ==
                    state.axes[-(settings.controllerOverdrive + 1)]) {
                    handleInputs(-1, GLFW_PRESS);
                } else {
                    handleInputs(-1, GLFW_RELEASE);
                }
            }
        }
        if (diff == 3) {
            for (int i = 0; i < 5; i++) {
                if (settings.controller5K[i] >= 0) {
                    if (state.buttons[settings.controller5K[i]] != buttonValues[settings.controller5K[i]]) {
                        if (state.buttons[settings.controller5K[i]] == 1)
                            heldFrets[i] = true;
                        else {
                            heldFrets[i] = false;
                            overhitFrets[i] = false;
                        }

                        handleInputs(i, state.buttons[settings.controller5K[i]]);
                        buttonValues[settings.controller5K[i]] = state.buttons[settings.controller5K[i]];
                    }
                } else {
                    if (state.axes[-(settings.controller5K[i] + 1)] != axesValues[-(settings.controller5K[i] + 1)]) {
                        if (state.axes[-(settings.controller5K[i] + 1)] ==
                            1.0f * (float) settings.controller5KAxisDirection[i]) {
                            heldFrets[i] = true;
                            handleInputs(i, GLFW_PRESS);
                        } else {
                            heldFrets[i] = false;
                            overhitFrets[i] = false;
                            handleInputs(i, GLFW_RELEASE);
                        }
                        axesValues[-(settings.controller5K[i] + 1)] = state.axes[-(settings.controller5K[i] + 1)];
                    }
                }
            }
        } else {
            for (int i = 0; i < 4; i++) {
                if (settings.controller4K[i] >= 0) {
                    if (state.buttons[settings.controller4K[i]] != buttonValues[settings.controller4K[i]]) {
                        if (state.buttons[settings.controller4K[i]] == 1)
                            heldFrets[i] = true;
                        else {
                            heldFrets[i] = false;
                            overhitFrets[i] = false;
                        }
                        handleInputs(i, state.buttons[settings.controller4K[i]]);
                        buttonValues[settings.controller4K[i]] = state.buttons[settings.controller4K[i]];
                    }
                } else {
                    if (state.axes[-(settings.controller4K[i] + 1)] != axesValues[-(settings.controller4K[i] + 1)]) {
                        if (state.axes[-(settings.controller4K[i] + 1)] ==
                            1.0f * (float) settings.controller4KAxisDirection[i]) {
                            heldFrets[i] = true;
                            handleInputs(i, GLFW_PRESS);
                        } else {
                            heldFrets[i] = false;
                            overhitFrets[i] = false;
                            handleInputs(i, GLFW_RELEASE);
                        }
                        axesValues[-(settings.controller4K[i] + 1)] = state.axes[-(settings.controller4K[i] + 1)];
                    }
                }
            }
        }
    };

void RhythmLogic::gamepadStateCallbackSetControls(int jid, GLFWgamepadstate state) {
        for (int i = 0; i < 6; i++) {
            axesValues2[i] = state.axes[i];
        }
        if (changingKey || changingOverdrive) {
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
};

GLFWkeyfun RhythmLogic::origKeyCallback = glfwSetKeyCallback(glfwGetCurrentContext(), RhythmLogic::keyCallback);
GLFWgamepadstatefun RhythmLogic::origGamepadCallback = glfwSetGamepadStateCallback(RhythmLogic::gamepadStateCallback);




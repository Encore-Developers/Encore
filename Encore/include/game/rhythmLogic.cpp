//
// Created by marie on 02/05/2024.
//

#include "rhythmLogic.h"


SongList songList;
Settings settings;

bool RhythmLogic::streamsLoaded = false;
std::vector<std::pair<Music, int>> RhythmLogic::loadedStreams;
int RhythmLogic::curPlayingSong = 0;
std::vector<int> RhythmLogic::curNoteIdx = { 0,0,0,0,0 };
std::vector<bool> RhythmLogic::heldFrets{ false,false,false,false,false };
std::vector<bool> RhythmLogic::heldFretsAlt{ false,false,false,false,false };
std::vector<bool> RhythmLogic::overhitFrets{ false,false,false,false,false };
std::vector<bool> RhythmLogic::tapRegistered{ false,false,false,false,false };
std::vector<bool> RhythmLogic::liftRegistered{ false,false,false,false,false };
bool RhythmLogic::overdriveHeld = false;
bool RhythmLogic::overdriveAltHeld = false;
bool RhythmLogic::overdriveHitAvailable = false;
bool RhythmLogic::overdriveLiftAvailable = false;
std::vector<bool> RhythmLogic::overdriveLanesHit{ false,false,false,false,false };
double RhythmLogic::overdriveHitTime = 0.0;
std::vector<int> RhythmLogic::lastHitLifts{-1, -1, -1, -1, -1};
std::vector<float> RhythmLogic::axesValues{0.0f,0.0f,0.0f,0.0f,0.0f,0.0f};
std::vector<int> RhythmLogic::buttonValues{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
std::vector<float> RhythmLogic::axesValues2{ 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f };
int RhythmLogic::pressedGamepadInput = -999;
int RhythmLogic::axisDirection = -1;
int RhythmLogic::controllerID = -1;
int RhythmLogic::curODPhrase = 0;
int RhythmLogic::curBeatLine = 0;
int RhythmLogic::curBPM = 0;
int RhythmLogic::selLane = 0;
bool RhythmLogic::selSong = false;
bool RhythmLogic::songsLoaded= false;
int RhythmLogic::songSelectOffset = 0;
bool RhythmLogic::changingKey = false;
bool RhythmLogic::changingOverdrive = false;
double RhythmLogic::startedPlayingSong = 0.0;
bool RhythmLogic::midiLoaded = false;
bool RhythmLogic::isPlaying = false;

void RhythmLogic::handleInputs(int lane, int action) {

        if (lane == -2) return;
        if (settings.mirrorMode && lane != -1) {
            lane = (Player::diff == 3 ? 4 : 3) - lane;
        }
        if (!streamsLoaded) {
            return;
        }
        Chart &curChart = songList.songs[curPlayingSong].parts[Player::instrument]->charts[Player::diff];
        float eventTime = GetMusicTimePlayed(loadedStreams[0].first);
        if (action == GLFW_PRESS && (lane == -1) && Player::overdriveFill > 0 && !Player::overdrive) {
            Player::overdriveActiveTime = eventTime;
            Player::overdriveActiveFill = Player::overdriveFill;
            Player::overdrive = true;
            overdriveHitAvailable = true;
            overdriveHitTime = eventTime;
        }
        if (lane == -1) {
            if ((action == GLFW_PRESS && !overdriveHitAvailable) ||
                (action == GLFW_RELEASE && !overdriveLiftAvailable))
                return;
            Note *curNote = &curChart.notes[0];
            for (auto &note: curChart.notes) {
                if (note.time - (Player::goodBackend + Player::InputOffset) < eventTime &&
                    note.time + (Player::goodFrontend + Player::InputOffset) > eventTime &&
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
                if (curNote->time - (Player::goodBackend + Player::InputOffset) < eventTime &&
                    curNote->time + (Player::goodFrontend + Player::InputOffset) > eventTime &&
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
                                if ((chordNote.time) - Player::perfectBackend + Player::InputOffset < eventTime &&
                                    chordNote.time + Player::perfectFrontend + Player::InputOffset > eventTime) {
                                    chordNote.perfect = true;

                                }
                                if (chordNote.perfect) Player::lastNotePerfect = true;
                                else Player::lastNotePerfect = false;
                                Player::HitNote(chordNote.perfect, Player::instrument);
                                chordNote.accounted = true;
                            }
                        }
                    }
                    overdriveHitAvailable = false;
                    overdriveLiftAvailable = true;
                }
            } else if (action == GLFW_RELEASE && overdriveLiftAvailable) {
                if ((curNote->time) - (Player::goodBackend * Player::liftTimingMult) + Player::InputOffset < eventTime &&
                    (curNote->time) + (Player::goodFrontend * Player::liftTimingMult) + Player::InputOffset > eventTime &&
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

                                    if ((chordNote.time) - Player::perfectBackend + Player::InputOffset < eventTime &&
                                        chordNote.time + Player::perfectFrontend + Player::InputOffset > eventTime) {
                                        chordNote.perfect = true;

                                    }
                                    if (chordNote.perfect) Player::lastNotePerfect = true;
                                    else Player::lastNotePerfect = false;
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
                                if (!((Player::diff == 3 && settings.keybinds5K[chordNote.lane]) ||
                                      (Player::diff != 3 && settings.keybinds4K[chordNote.lane]))) {
                                    chordNote.held = false;
                                    Player::score += Player::sustainScoreBuffer[chordNote.lane];
                                    Player::sustainScoreBuffer[chordNote.lane] = 0;
                                    Player::mute = true;
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
                    if ((curNote.time) - (action == GLFW_RELEASE ? Player::goodBackend * Player::liftTimingMult : Player::goodBackend) +
                                Player::InputOffset < eventTime &&
                        (curNote.time) +
                        ((action == GLFW_RELEASE ? Player::goodFrontend * Player::liftTimingMult : Player::goodFrontend) + Player::InputOffset) >
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
                        if ((curNote.time) - Player::perfectBackend + Player::InputOffset < eventTime &&
                            curNote.time + Player::perfectFrontend + Player::InputOffset > eventTime) {
                            curNote.perfect = true;
                        }
                        if (curNote.perfect) Player::lastNotePerfect = true;
                        else Player::lastNotePerfect = false;
                        Player::HitNote(curNote.perfect, Player::instrument);
                        curNote.accounted = true;
                        break;
                    }
                    if (curNote.miss) Player::lastNotePerfect = false;
                }
                if (action == GLFW_RELEASE && curNote.held && (curNote.len) > 0) {
                    curNote.held = false;
                    Player::score += Player::sustainScoreBuffer[curNote.lane];
                    Player::sustainScoreBuffer[curNote.lane] = 0;
                    Player::mute = true;
                    // SetAudioStreamVolume(loadedStreams[instrument].stream, missVolume);
                }

                if (action == GLFW_PRESS &&
                    eventTime > songList.songs[curPlayingSong].music_start &&
                    !curNote.hit &&
                    !curNote.accounted &&
                    ((curNote.time) - Player::perfectBackend) + Player::InputOffset > eventTime &&
                    eventTime > overdriveHitTime + 0.05
                    && !overhitFrets[lane]) {
                    if (lastHitLifts[lane] != -1) {
                        if (eventTime > curChart.notes[lastHitLifts[lane]].time - 0.1 &&
                            eventTime < curChart.notes[lastHitLifts[lane]].time + 0.1)
                            continue;
                    }
                    Player::OverHit();
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
                if (Player::diff == 3) {
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
        if (Player::diff == 3) {
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




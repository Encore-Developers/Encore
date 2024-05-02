//
// Created by marie on 02/05/2024.
//

#ifndef ENCORE_RHYTHMLOGIC_H
#define ENCORE_RHYTHMLOGIC_H

#include <vector>
#include "GLFW/glfw3.h"
#include "audio/audio.h"
#include "raylib.h"
#include "song/songlist.h"
#include "settings.h"
#include "song/song.h"

bool streamsLoaded = false;
std::vector<std::pair<Music, int>> loadedStreams;
int curPlayingSong = 0;
std::vector<int> curNoteIdx = { 0,0,0,0,0 };
std::vector<bool> heldFrets{ false,false,false,false,false };
std::vector<bool> heldFretsAlt{ false,false,false,false,false };
std::vector<bool> overhitFrets{ false,false,false,false,false };
std::vector<bool> tapRegistered{ false,false,false,false,false };
std::vector<bool> liftRegistered{ false,false,false,false,false };
bool overdriveHeld = false;
bool overdriveAltHeld = false;
bool overdriveHitAvailable = false;
bool overdriveLiftAvailable = false;
std::vector<bool> overdriveLanesHit{ false,false,false,false,false };
double overdriveHitTime = 0.0;
std::vector<int> lastHitLifts{-1, -1, -1, -1, -1};
std::vector<float> axesValues{0.0f,0.0f,0.0f,0.0f,0.0f,0.0f};
std::vector<int> buttonValues{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
std::vector<float> axesValues2{ 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f };
int pressedGamepadInput = -999;
int axisDirection = -1;
int controllerID = -1;
int curODPhrase = 0;
int curBeatLine = 0;
int curBPM = 0;
int selLane = 0;
bool selSong = false;
bool songsLoaded= false;
int songSelectOffset = 0;
bool changingKey = false;
bool changingOverdrive = false;
double startedPlayingSong = 0.0;
bool midiLoaded = false;
bool isPlaying = false;

class RhythmLogic {
public:
    static void handleInputs(int lane, int action);

    static void keyCallback(GLFWwindow *wind, int key, int scancode, int action, int mods);

    static void gamepadStateCallback(int jid, GLFWgamepadstate state);

    static void gamepadStateCallbackSetControls(int jid, GLFWgamepadstate state);

    static GLFWkeyfun origKeyCallback;
    static GLFWgamepadstatefun origGamepadCallback;

};

#endif //ENCORE_RHYTHMLOGIC_H

//
// Created by marie on 02/05/2024.
//

#ifndef ENCORE_RHYTHMLOGIC_H
#define ENCORE_RHYTHMLOGIC_H

#include <vector>
#include "GLFW/glfw3.h"
#include "audio/audio.h"

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

class rhythmLogic {
public:


    void handleInputs(int lane, int action);
    void keyCallback(GLFWwindow* wind, int key, int scancode, int action, int mods);
    void gamepadStateCallback(int jid, GLFWgamepadstate state);
    void gamepadStateCallbackSetControls(int jid, GLFWgamepadstate state);
};


#endif //ENCORE_RHYTHMLOGIC_H

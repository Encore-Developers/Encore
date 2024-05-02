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
#include "player.h"
#include "settings.h"
#include "song/song.h"



class RhythmLogic {
public:
    static bool streamsLoaded;
    static std::vector<std::pair<Music, int>> loadedStreams;
    static int curPlayingSong;
    static std::vector<int> curNoteIdx;
    static std::vector<bool> heldFrets;
    static std::vector<bool> heldFretsAlt;
    static std::vector<bool> overhitFrets;
    static std::vector<bool> tapRegistered;
    static std::vector<bool> liftRegistered;
    static bool overdriveHeld;
    static bool overdriveAltHeld;
    static bool overdriveHitAvailable;
    static bool overdriveLiftAvailable;
    static std::vector<bool> overdriveLanesHit;
    static double overdriveHitTime;
    static std::vector<int> lastHitLifts;
    static std::vector<float> axesValues;
    static std::vector<int> buttonValues;
    static std::vector<float> axesValues2;
    static int pressedGamepadInput;
    static int axisDirection;
    static int controllerID;
    static int curODPhrase;
    static int curBeatLine;
    static int curBPM;
    static int selLane;
    static bool selSong;
    static bool songsLoaded;
    static int songSelectOffset;
    static bool changingKey;
    static bool changingOverdrive;
    static double startedPlayingSong;
    static bool midiLoaded;
    static bool isPlaying;

    static void handleInputs(int lane, int action);

    static void keyCallback(GLFWwindow *wind, int key, int scancode, int action, int mods);

    static void gamepadStateCallback(int jid, GLFWgamepadstate state);

    static void gamepadStateCallbackSetControls(int jid, GLFWgamepadstate state);

    static GLFWkeyfun origKeyCallback;
    static GLFWgamepadstatefun origGamepadCallback;

};

#endif //ENCORE_RHYTHMLOGIC_H

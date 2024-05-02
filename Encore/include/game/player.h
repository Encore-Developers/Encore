#pragma once

#ifndef ENCORE_PLAYER_H
#define ENCORE_PLAYER_H

#include <vector>
#include "raylib.h"

    class Player {
    public:
        static int instrument;
        static int diff;

        static int notesHit;
        static int notesMissed;
        static int perfectHit;


        static float selInstVolume;
        static float otherInstVolume;
        static float missVolume;

// time in seconds
        static float goodFrontend;
        static float goodBackend;
        static float perfectFrontend;
        static float perfectBackend;

        static float VideoOffset;
        static float InputOffset;

        static bool MissHighwayColor;

        static bool lastNotePerfect;

// make the hitwindow bigger for properly doing lifts
        static float liftTimingMult;

// 11.5f default --   23.0f 2x
        static float defaultHighwayLength;

        static float smasherPos; // used to be 2.7

        static bool extraGameplayStats;

        static int notes;
        static int combo;
        static int maxCombo;
        static int score;
        static std::vector<int> sustainScoreBuffer;
        static int playerOverhits;

        static bool goldStars;

        static bool overdrive;

        static bool FC;

        static float health;

        static bool mute;

        static float xStarThreshold[6];

        static float overdriveFill;
        static float overdriveActiveFill;
        static double overdriveActiveTime;

        static float uvOffsetX;
        static float uvOffsetY;

        static int stars(int baseScore, int difficulty);

        static int multiplier(int instrument);

        static int maxMultForMeter(int instrument);

        static float comboFillCalc(int instrument);


        static Color accentColor;

        static Color overdriveColor;

        static void resetPlayerStats();

        static void HitNote(bool perfect, int instrument);

        static void HitNoteAudio(bool perfect, int instrument);

        static void MissNote();

        static void OverHit();
    };

#endif
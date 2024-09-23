//
// Created by marie on 23/09/2024.
//

#ifndef ENCNOTE_H
#define ENCNOTE_H
#include "timingvalues.h"

#include <cstdint>
#include <vector>

class Note {
public:
    double time;
    double len;
    double beatsLen;
    double heldTime = 0.0;
    double sustainThreshold = 0.2;
    double HitOffset = 0.0;
    int lane;
    bool lift = false;
    bool hit = false;
    bool held = false;
    bool valid = false;
    bool miss = false;
    bool accounted = false;
    bool countedForSolo = false;
    bool countedForSection = false;
    bool countedForODPhrase = false;
    bool perfect = false;
    bool renderAsOD = false;
    double hitTime = 0;
    int tick;

    // CLASSIC
    // 0-4 for grybo, helps with chords
    int strumCount = 0;
    int chordSize = 0;
    bool hitWithFAS = false;
    uint8_t mask;
    bool chord = false;
    std::vector<int> pLanes;
    bool pForceOn = false;
    bool pForceOff = false;
    bool phopo = false;
    bool extendedSustain = false;
    bool pDrumTom = false;
    bool pSnare = false;
    bool pDrumAct = false;
    int GhostCount = 0;
    bool Ghosted = false;

    bool isGood(double eventTime, double inputOffset) const {
        return (
            time - goodBackend + inputOffset < eventTime
            && time + goodFrontend + inputOffset > eventTime
        );
    }
    bool isPerfect(double eventTime, double inputOffset) {
        return (
            time - perfectBackend + inputOffset < eventTime
            && time + perfectFrontend + inputOffset > eventTime
        );
    }

    bool pTap = false;
    bool pOpen = false;
};




#endif //ENCNOTE_H

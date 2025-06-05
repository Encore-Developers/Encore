//
// Created by marie on 23/09/2024.
//

#ifndef ENCNOTE_H
#define ENCNOTE_H

#include "../../timingvalues.h"

#include <cstdint>
#include <vector>

struct ClassicLane {
    double length = 0.0;
    double beatsLen = 0.0;
    double heldTime = 0.0;
    bool accounted = false;
    int lane;
    ClassicLane(double _length, double _beatsLen, int _lane) {
        length = _length;
        beatsLen = _beatsLen;
        lane = _lane;
    }
};

class Note {
public:
    double time;
    double len = 0.0;
    double beatsLen = 0.0;
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
    bool hitInFrontend = false;
    double hitTime = 0;
    int tick;

    // CLASSIC
    // 0-4 for grybo, helps with chords
    int strumCount = 0;
    int chordSize = 0;
    bool hitWithFAS = false;
    uint8_t mask;
    bool chord = false;
    std::vector<ClassicLane> pLanes;
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

    void cHitNote(double eventTime, double offset) {
        this->hit = true;
        this->HitOffset = this->time - eventTime;
        this->hitTime = eventTime - offset;

        if ((this->pLanes[0].length) > 0) {
            this->held = true;
        }
        if (isPerfect(eventTime, offset)) {
            this->perfect = true;
        }

        this->accounted = true;
    }

    void padHitNote(double eventTime, double offset) {
        hit = true;

        HitOffset = time - eventTime;
        hitTime = eventTime;

        if ((len) > 0) {
            held = true;
        }
        if (isPerfect(eventTime, offset)) {
            perfect = true;
        }

        accounted = true;
    }

    bool pTap = false;
    bool pOpen = false;
};

#endif // ENCNOTE_H

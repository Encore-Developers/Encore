//
// Created by marie on 23/09/2024.
//

#ifndef ENCCHARTEVENT_H
#define ENCCHARTEVENT_H
#include <string>

struct EncNoteEvent {
    double StartSec = 0.0;
    double EndSec = 0.0;
    int StartTick = 0;
    int EndTick = 0;
};

struct EncChartEvent : EncNoteEvent {
    int NotesHit = 0;
    int NoteCount = 0;
    EncChartEvent() = default;
    EncChartEvent(int tickStart, double secondStart, int tickEnd, double secondEnd) {
        StartTick = tickStart;
        StartSec = secondStart;
        EndTick = tickEnd;
        EndSec = secondEnd;
    };
};

struct Coda : EncChartEvent {
    bool exists = false;
    /*
    bool IsNoteInCoda(Note &note) {
        if (exists) {
            if (note.time >= StartSec && note.time < EndSec) {
                return true;
            }
        }
        return false;
    }
*/
    bool IsCodaActive(float time) {
        if (exists) {
            if (time >= StartSec && time <= EndSec) {
                return true;
            }
        }
        return false;
    }
};

struct solo : EncChartEvent {
    solo(int tickStart, double secondStart, int tickEnd, double secondEnd)
        : EncChartEvent(
              StartTick = tickStart,
              StartSec = secondStart,
              EndTick = tickEnd,
              EndSec = secondEnd
          ) {}
};

struct DrumFill : EncChartEvent {};

struct odPhrase : EncChartEvent {
    odPhrase(int tickStart, double secondStart, int tickEnd, double secondEnd)
        : EncChartEvent(
              StartTick = tickStart,
              StartSec = secondStart,
              EndTick = tickEnd,
              EndSec = secondEnd
          ) {}
    bool added = false;
    bool missed = false;
};

struct section : EncChartEvent {
    section(int tickStart, double secondStart, int tickEnd, double secondEnd)
        : EncChartEvent(
              StartTick = tickStart,
              StartSec = secondStart,
              EndTick = tickEnd,
              EndSec = secondEnd
          ) {}
    std::string Name;
};
struct tapPhrase : EncNoteEvent {};

struct forceOnPhrase : EncNoteEvent {};

struct openMarker : EncNoteEvent {};

struct forceOffPhrase : EncNoteEvent {};

#endif // ENCCHARTEVENT_H

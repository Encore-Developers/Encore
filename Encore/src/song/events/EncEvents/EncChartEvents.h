//
// Created by marie on 23/09/2024.
//

#ifndef ENCCHARTEVENT_H
#define ENCCHARTEVENT_H

struct EncNoteEvent {
    double StartSec = 0.0;
    double EndSec = 0.0;
    int StartTick = 0;
    int EndTick = 0;
};

struct EncChartEvent : EncNoteEvent {
    int NotesHit = 0;
    int NoteCount = 0;
};

struct solo : EncChartEvent {};

struct DrumFill : EncChartEvent {};

struct odPhrase : EncChartEvent {
    bool added = false;
    bool missed = false;
};

struct section : EncChartEvent {
    std::string Name;
};
struct tapPhrase : EncNoteEvent {};

struct forceOnPhrase : EncNoteEvent {};

struct forceOffPhrase : EncNoteEvent {};
#endif //ENCCHARTEVENT_H

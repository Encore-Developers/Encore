//
// Created by marie on 23/09/2024.
//

#ifndef ELSE_H
#define ELSE_H

#include "EncEventVect.h"
#include "../EncEvents/EncChartEvents.h"

struct SoloEvents : EncEventVect<solo> {};

struct FillEvents : EncEventVect<DrumFill> {};

struct ODEvents : EncEventVect<odPhrase> {
    void ResetEvents() override {
        for (auto event : events) {
            event.NotesHit = 0;
            event.added = false;
            event.missed = false;
        }
    }
    void UpdateEventViaNote(Note& note, int curEvent) override {
        if (!events.empty()) {
            if (note.time >= events[curEvent].StartSec
                && note.time < events[curEvent].EndSec) {
                if (!note.miss && !events[curEvent].missed) {
                    note.renderAsOD = true;
                } else {
                    note.renderAsOD = false;
                }
                if (note.hit && !note.countedForODPhrase) {
                    events[curEvent].NotesHit++;
                    note.countedForODPhrase = true;
                }
                if (note.miss && !events[curEvent].missed) {
                    events[curEvent].missed = true;
                }
                }
        }
    }
    float AddOverdrive(int phrase) {
        if (events[phrase].NoteCount == events[phrase].NotesHit
            && !events[phrase].added
            && !events[phrase].missed) {
            events[phrase].added = true;
            return 0.25f;
            }
        return 0;
    }
};

struct SectionEvents : EncEventVect<section> {
    void UpdateEventViaNote(Note& note, int curEvent) override {
        if (!events.empty()) {
            if (note.time >= events[curEvent].StartSec
                && note.time < events[curEvent].EndSec) {
                if (note.hit) {
                    ++events[curEvent].NotesHit;
                    ++events[curEvent].NoteCount;
                }
                if (note.miss) {
                    ++events[curEvent].NoteCount;
                }
                }
        }
    }
};

#endif //ELSE_H

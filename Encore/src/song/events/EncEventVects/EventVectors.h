//
// Created by marie on 23/09/2024.
//

#ifndef ELSE_H
#define ELSE_H

#include "EncEventVect.h"
#include "../EncEvents/EncChartEvents.h"
#include "util/enclog.h"
#include "raylib.h"

struct SoloEvents final : EncEventVect<solo> {
    void UpdateEventViaNote(bool hit, int tick) override {
        if (this->empty()) {
            return;
        }
        if (!TickDuringCurrentEvent(tick)) {
            return;
        }
        if (hit) {
            this->at(CurrentEvent).NotesHit++;
            Encore::EncoreLog(
                LOG_DEBUG,
                TextFormat(
                    "Solo note hit: %01i/%01i",
                    this->at(CurrentEvent).NotesHit,
                    this->at(CurrentEvent).NoteCount
                )
            );
        }
    }
};

struct FillEvents final : EncEventVect<DrumFill> {};

// ok so for overdrive
// if a note is hit, add it to NotesHit and NoteCount
// if a note is missed, add it to only NoteCount
struct ODEvents final : EncEventVect<odPhrase> {
    void ResetEvents() override {
        for (auto &event : *this) {
            event.NotesHit = 0;
            event.added = false;
            event.missed = false;
        }
    }

    // on note hit/miss, add the statistics of that note to the event
    // miss = just add to NoteCount
    // hit =  add to NoteCount and NotesHit
    // this does NOT cover overhits
    void UpdateEventViaNote(bool hit, int tick) override {
        if (this->empty()) {
            return;
        }

        if (!(TickDuringCurrentEvent(tick))) {
            return;
        }
        if (hit) {
            this->front().NotesHit++;
            this->front().NoteCount++;
            Encore::EncoreLog(
                LOG_DEBUG,
                TextFormat(
                    "Overdrive note hit: %01i/%01i",
                    this->front().NotesHit,
                    this->front().NoteCount
                )
            );
        }
        if (!hit && !this->front().missed) {
            this->front().NoteCount++;
            this->front().missed = true;
            Encore::EncoreLog(
               LOG_DEBUG,
               TextFormat(
                   "Overdrive note missed: %01i/%01i",
                   this->front().NotesHit,
                   this->front().NoteCount
               )
           );
        }
    }

    void CheckEvents(int tick) override {
        if (this->empty())
            return;
        Encore::EncoreLog(
            LOG_ERROR,
            "MARIA WHY THE FUCK ARE YOU USING CheckEvents(int) ON OVERDRIVE?????"
        );
        /*
        if (CurrentEvent < this->size() - 1 && tick >= this->at(CurrentEvent).EndTick) {
            CurrentEvent++;
        }*/
    }

    void MissCurrentEvent(int eventTime) {
        if (!this->empty())
            return;
        if (TickDuringCurrentEvent(eventTime))
            this->front().missed = true;
    }
    /*
    void RenderNotesAsOD(Note& note, const int curEvent) const {
        if (!events.empty()) {
            if (note.time >= events[curEvent].StartSec
                && note.time < events[curEvent].EndSec) {
                if (!note.miss && !events[curEvent].missed) {
                    note.renderAsOD = true;
                } else {
                    note.renderAsOD = false;
                }
            }
        }
    }*/
    bool TickDuringCurrentEvent(int tick) override {
        if (tick >= this->front().StartTick
            && tick < this->front().StartTick + this->front().EndTick) {
            return true;
            }
        return false;
    }
    bool Perfect() override {
        if (!this->empty()) {
            return this->front().NotesHit == this->front().NoteCount;
        }
        return false;
    }
    // run CheckOverdrive instead of CheckEvents
    float CheckOverdrive(double sec) {
        if (this->empty())
            return 0;

        float valueToReturn = 0;
        // if we're at the end of the event
        if (sec > this->front().StartSec + this->front().EndSec) {
            // and the event can potentially have overdrive added
            if (Perfect() && !this->front().missed) {
                // add overdrive
                valueToReturn = 0.25f;
                Encore::EncoreLog(LOG_DEBUG, "Perfect Overdrive phrase");
            }
            Encore::EncoreLog(LOG_DEBUG, "Removing Overdrive phrase");
            // increment if possible, make sure that the last overdrive gets added
            this->erase(this->begin());
        }

        return valueToReturn;
    }
};

struct SectionEvents final : EncEventVect<section> {
    // EncNote
    void UpdateEventViaNote(bool note, int curEvent) override {
        if (!this->empty())
            return;

        if (!TickDuringCurrentEvent(curEvent))
            return;

        if (note) {
            ++this->at(CurrentEvent).NotesHit;
        }
        ++this->at(CurrentEvent).NoteCount;
    }
};

#endif // ELSE_H

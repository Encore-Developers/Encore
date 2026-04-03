//
// Created by maria on 25/03/2026.
//

#include "EventVectors.h"

void Encore::RhythmEngine::ODEvents::ResetEvents() {
    for (auto &event : *this) {
        event.NotesHit = 0;
        event.added = false;
        event.missed = false;
    }
}

void Encore::RhythmEngine::ODEvents::UpdateEventViaNote(bool hit, int tick) {
    if (this->empty()) {
        return;
    }

    if (!(this->TickDuringCurrentEvent(tick))) {
        return;
    }
    if (hit) {
        this->at(CurrentEvent).NotesHit++;
        Encore::EncoreLog(
            LOG_DEBUG,
            TextFormat(
                "Overdrive note hit: %01i/%01i",
                this->at(CurrentEvent).NotesHit,
                this->at(CurrentEvent).NoteCount
            )
        );
    }
    if (!hit && !this->at(CurrentEvent).missed) {
        this->at(CurrentEvent).missed = true;
        Encore::EncoreLog(
            LOG_DEBUG,
            TextFormat(
                "Overdrive note missed: %01i/%01i",
                this->at(CurrentEvent).NotesHit,
                this->at(CurrentEvent).NoteCount
            )
        );
    }
}

void Encore::RhythmEngine::ODEvents::CheckEvents(int tick) {
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

void Encore::RhythmEngine::ODEvents::MissCurrentEvent(int eventTime) {
    if (this->empty())
        return;
    if (TickDuringCurrentEvent(eventTime))
        this->at(CurrentEvent).missed = true;
}

bool Encore::RhythmEngine::ODEvents::RenderNotesAsOD(double time) const {
    if (this->empty()) {
        return false;
    }

    for (auto &event : *this) {
        if ((time >= event.StartSec
            && time < event.StartSec + event.EndSec) && !event.missed) {
            return true;
            }
    }
    return false;
}

bool Encore::RhythmEngine::ODEvents::TickDuringCurrentEvent(int tick) {
    if (tick >= this->at(CurrentEvent).StartTick
        && tick < this->at(CurrentEvent).StartTick + this->at(CurrentEvent).EndTick) {
        return true;
        }
    return false;
}

bool Encore::RhythmEngine::ODEvents::Perfect() {
    if (CurrentEvent < this->size()) {
        return this->at(CurrentEvent).NotesHit == this->at(CurrentEvent).NoteCount;
    }
    return false;
}

float Encore::RhythmEngine::ODEvents::CheckOverdrive(double sec) {
    if (CurrentEvent >= this->size())
        return 0;

    float valueToReturn = 0;
    // if we're at the end of the event
    // and the event can potentially have overdrive added
    if (this->Perfect() && !this->at(CurrentEvent).missed && !this->at(CurrentEvent).added) {
        // add overdrive
        valueToReturn = 0.25f;
        Encore::EncoreLog(LOG_DEBUG, "Perfect Overdrive phrase");
        Encore::EncoreLog(LOG_DEBUG, "Removing Overdrive phrase");
        // increment if possible, make sure that the last overdrive gets added
        this->at(CurrentEvent).added = true;
        if (CurrentEvent < this->size())
            CurrentEvent++;
        return valueToReturn;
    }
    if (this->at(CurrentEvent).missed) {
        Encore::EncoreLog(LOG_DEBUG, "Removing Overdrive phrase");
        // increment if possible, make sure that the last overdrive gets added
        if (CurrentEvent < this->size())
            CurrentEvent++;
        return valueToReturn;
    }

    return valueToReturn;
}

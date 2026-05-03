//
// Created by maria on 25/03/2026.
//
#include "EventVectors.h"

bool Encore::RhythmEngine::SoloEvents::TickDuringCurrentEvent(int tick) {
    if (tick >= this->at(CurrentEvent).StartTick
        && tick < this->at(CurrentEvent).StartTick + this->at(CurrentEvent).TickLength) {
        return true;
        }
    return false;
}

void Encore::RhythmEngine::SoloEvents::CheckEvents(double time) {
    if (this->empty())
        return;

    if (CurrentEvent < this->size() - 1 && time >= this->at(CurrentEvent).EndSec + this->at(CurrentEvent).StartSec) {
        CurrentEvent++;
    }
}

void Encore::RhythmEngine::SoloEvents::UpdateEventViaNote(bool hit, int tick) {
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
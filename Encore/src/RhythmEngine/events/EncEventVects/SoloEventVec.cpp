//
// Created by maria on 25/03/2026.
//
#include "EventVectors.h"

bool Encore::RhythmEngine::SoloEvents::TickDuringCurrentEvent(int tick) {
    if (tick >= this->front().StartTick
        && tick < this->front().StartTick + this->front().EndTick) {
        return true;
        }
    return false;
}

void Encore::RhythmEngine::SoloEvents::UpdateEventViaNote(bool hit, int tick) {
    if (this->empty()) {
        return;
    }
    if (!TickDuringCurrentEvent(tick)) {
        return;
    }
    if (hit) {
        this->front().NotesHit++;
        Encore::EncoreLog(
            LOG_DEBUG,
            TextFormat(
                "Solo note hit: %01i/%01i",
                this->front().NotesHit,
                this->front().NoteCount
            )
        );
    }
}
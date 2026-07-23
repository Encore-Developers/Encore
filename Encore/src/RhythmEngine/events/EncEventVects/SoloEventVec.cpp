//
// Created by maria on 25/03/2026.
//
#include "EventVectors.h"

bool Encore::RhythmEngine::SoloEvents::TickDuringCurrentEvent(int tick) {
    if (tick >= this->at(CurrentEvent).start.tick
        && tick < this->at(CurrentEvent).end.tick) {
        return true;
        }
    return false;
}

void Encore::RhythmEngine::SoloEvents::CheckEvents(double time) {
    if (this->empty())
        return;

    if (CurrentEvent < this->size() - 1 && time >= this->at(CurrentEvent).end.sec) {
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
        Log::Debug("Solo note hit: {}/{}",
                this->at(CurrentEvent).NotesHit,
                this->at(CurrentEvent).NoteCount);
    }
}
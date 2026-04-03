//
// Created by maria on 25/03/2026.
//

#include "EventVectors.h"

void Encore::RhythmEngine::SectionEvents::UpdateEventViaNote(bool note, int curEvent) {
    if (!this->empty())
        return;

    if (!TickDuringCurrentEvent(curEvent))
        return;

    if (note) {
        ++this->at(CurrentEvent).NotesHit;
    }
    ++this->at(CurrentEvent).NoteCount;
}
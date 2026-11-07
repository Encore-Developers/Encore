//
// Created by maria on 13/06/2025.
//

#include "DrumsEngine.h"

void Encore::RhythmEngine::DrumsEngine::RunHitStateCheck(ControllerEvent &event
) {
    if (event.channel > IntIC(chart->size-1))
        return;
    int lane = ICInt(event.channel);
    if (chart->CurrentNoteIterators.at(lane) == chart->Lanes.at(lane).end())
        return;
    NoteEvent *CurrentNote = &*chart->CurrentNoteIterators.at(lane);

    //bool IsCymbal = CurrentNote->type == NoteEvent::CYMBAL;


    // auto curNoteItr = chartLane.begin();
    // while (curNoteItr->StartSeconds + goodBackend
    //        < TheSongTime.GetElapsedTime() - stats->InputOffset) {
    //     if (curNoteItr + 1 == chartLane.end()) {
    //         return CheckNextInput;
    //     }
    //     ++curNoteItr;
    // }
    // EncNote &CurrentNote = *curNoteItr;
    // bool lift = false; //action == Action::RELEASE && CurrentNote.NoteType == 1;
    if (event.action == Action::PRESS) {
        if (EarlyStrike(CurrentNote->start.sec)) {
            if (Timers["debounce"].CanBeUsedUp(EngineTime)) {
                Timers["debounce"].ResetTimer();
                return;
            }
            Overhit(lane);
            return;
        };
        if (InHitwindow(CurrentNote->start.sec)) {
            Timers["debounce"].ActivateTimer(EngineTime);
            HitNote(lane);
            return;
        };
    }
}

void Encore::RhythmEngine::DrumsEngine::HitNote(const size_t lane) {
    if (lane == 0) {
        HighwayBounceEvent event;
        FireEvent(&event);
    }
    BaseEngine::HitNote(lane);
}

void Encore::RhythmEngine::DrumsEngine::Advance(double CurrentTime) {
    BaseEngine::Advance(CurrentTime);
}

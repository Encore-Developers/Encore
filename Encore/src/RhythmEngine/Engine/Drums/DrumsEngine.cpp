//
// Created by maria on 13/06/2025.
//

#include "DrumsEngine.h"
//
// Created by maria on 01/06/2025.
//

bool Encore::RhythmEngine::DrumsEngine::ActivateOverdrive(
ControllerEvent &event
) {
    if (event.channel == InputChannel::OVERDRIVE && event.action == Action::PRESS) {
        if (stats->overdrive.Activate(stats->InputTime)) {
            OverdriveEvent gain;
            FireEvent(&gain);
        }; // time
        return true;
    }
    return false;
}

void Encore::RhythmEngine::DrumsEngine::SetStatsInputState(
ControllerEvent &event
) {
    stats->InputTime = event.timestamp - stats->InputOffset; // todo: REPLACE WITH ACTUAL SONG
    // TIME (IN SECONDS)
    if (event.action == Action::INVALID) return;
    if (event.channel == InputChannel::INVALID) return;

    bool press = event.action == Action::PRESS;
    if (event.channel < InputChannel::STRUM_UP) {
        if (chart-> size != 6 && event.channel == InputChannel::LANE_6) return;
        stats->HeldFrets.at(ICInt(event.channel)) = press;
    }
}

Encore::RhythmEngine::TimePoint Encore::RhythmEngine::DrumsEngine::NextNoteTime() {
    return BaseEngine::NextNoteTime();
}

Encore::RhythmEngine::TimePoint Encore::RhythmEngine::DrumsEngine::LastNoteTime() {
    return BaseEngine::LastNoteTime();
}

int Encore::RhythmEngine::DrumsEngine::RunHitStateCheck(ControllerEvent &event
) {
    if (event.channel > IntIC(chart->size-1))
        return CheckNextInput;
    int lane = ICInt(event.channel);
    if (chart->CurrentNoteIterators.at(lane) == chart->Lanes.at(lane).end())
        return CheckNextInput;
    NoteEvent *CurrentNote = &*chart->CurrentNoteIterators.at(lane);

    bool IsCymbal = CurrentNote->type == NoteEvent::CYMBAL;


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
            if (Timers["debounce"].CanBeUsedUp(stats->InputTime)) {
                Timers["debounce"].ResetTimer();
                return CheckNextInput;
            }
            Overhit(lane);
            return OverhitNote;
        };
        if (InHitwindow(CurrentNote->start.sec)) {
            Timers["debounce"].ActivateTimer(stats->InputTime);
            HitNote(lane);
            return HitState::HitNote;
        };
    }
    return CheckNextInput;
}

void Encore::RhythmEngine::DrumsEngine::HitNote(const size_t lane) {
    if (lane == 0) {
        HighwayBounceEvent event;
        FireEvent(&event);
    }
    BaseEngine::HitNote(lane);
}

void Encore::RhythmEngine::DrumsEngine::UpdateOnFrame(double CurrentTime) {
    LastUpdateTime = CurrentTime - stats->InputOffset;
    for (size_t Lane = 0; Lane < chart->Lanes.size(); Lane++) {
        if (stats->Bot) {
            if (chart->CurrentNoteIterators.at(Lane) < chart->Lanes.at(Lane).cend()) {
                NoteEvent *CurrentNote = &*chart->CurrentNoteIterators.at(Lane);
                if (CurrentNote->start.sec <= LastUpdateTime) {
                    stats->InputTime = LastUpdateTime;
                    HitNote(Lane);
                }
            }
        } else {
            CheckMissedNotes(Lane, LastUpdateTime);
        }
    }
    BaseUpdateOnFrame(LastUpdateTime);
}

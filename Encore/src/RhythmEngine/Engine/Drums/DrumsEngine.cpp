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
        stats->overdrive.Activate(stats->InputTime); // time
        return true;
    }
    return false;
}

void Encore::RhythmEngine::DrumsEngine::SetStatsInputState(
ControllerEvent &event
) {
    stats->InputTime = event.timestamp - stats->InputOffset; // todo: REPLACE WITH ACTUAL SONG
    // TIME (IN SECONDS)
    if (event.action == Action::PRESS) {
        switch (event.channel) {
        case InputChannel::LANE_1:
        case InputChannel::LANE_2:
        case InputChannel::LANE_3:
        case InputChannel::LANE_4:
        case InputChannel::LANE_5: {
            stats->HeldFrets.at(ICInt(event.channel)) = true;
            break;
        }
        case InputChannel::LANE_6: {
            if (chart->size == 6)
                stats->HeldFrets.at(ICInt(event.channel)) = true;
            break;
        }
        default:
            break;
        }
    }
    if (event.action == Action::RELEASE) {
        switch (event.channel) {
        case InputChannel::LANE_1:
        case InputChannel::LANE_2:
        case InputChannel::LANE_3:
        case InputChannel::LANE_4:
        case InputChannel::LANE_5: {
            stats->HeldFrets.at(ICInt(event.channel)) = false;
            break;
        }
        case InputChannel::LANE_6: {
            if (chart->size == 6)
                stats->HeldFrets.at(ICInt(event.channel)) = false;
            break;
        }
        default:
            break;
        }
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

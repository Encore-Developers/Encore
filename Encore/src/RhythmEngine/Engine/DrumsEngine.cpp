//
// Created by maria on 13/06/2025.
//

#include "DrumsEngine.h"
//
// Created by maria on 01/06/2025.
//

#include "timingvalues.h"
#include "gameplay/enctime.h"

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
    stats->InputTime = TheSongTime.GetElapsedTime(); // todo: REPLACE WITH ACTUAL SONG
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
        default:
            break;
        }
    }
}

int Encore::RhythmEngine::DrumsEngine::RunHitStateCheck(ControllerEvent &event
) {
    if (event.channel == InputChannel::STRUM_UP || event.channel == InputChannel::STRUM_DOWN)
        return CheckNextInput;
    int lane = ICInt(event.channel);
    if (chart->CurrentNoteIterators.at(lane) == chart->Lanes.at(lane).end())
        return CheckNextInput;
    EncNote *CurrentNote = &*chart->CurrentNoteIterators.at(lane);

    bool IsCymbal = CurrentNote->NoteType == 1;


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
        if (EarlyStrike(CurrentNote->StartSeconds)) {
            if (Timers["debounce"].CanBeUsedUp(stats->InputTime)) {
                Timers["debounce"].ResetTimer();
                return CheckNextInput;
            }
            Overhit(lane);
            return OverhitNote;
        };
        if (InHitwindow(CurrentNote->StartSeconds)) {
            Timers["debounce"].ActivateTimer(stats->InputTime);
            HitNote(lane);
            return HitState::HitNote;
        };
    }
    return CheckNextInput;
}

void Encore::RhythmEngine::DrumsEngine::HitNote(int lane) {
    if (lane == 0) {
        HighwayBounceEvent event;
        FireEvent(&event);
    }
    BaseEngine::HitNote(lane);
}

void Encore::RhythmEngine::DrumsEngine::UpdateOnFrame(double CurrentTime) {
    chart->solos.CheckEvents(CurrentTime);
    for (size_t Lane = 0; Lane < chart->Lanes.size(); Lane++) {
        if (stats->Bot) {
            if (chart->CurrentNoteIterators.at(Lane) < chart->Lanes.at(Lane).cend()) {
                EncNote *CurrentNote = &*chart->CurrentNoteIterators.at(Lane);
                if (CurrentNote->StartSeconds <= CurrentTime) {
                    stats->InputTime = CurrentTime;
                    HitNote(Lane);
                }
            }
        } else {
            CheckMissedNotes(Lane, CurrentTime);
        }
    }
    stats->overdrive.Add(CurrentTime, chart);
    stats->overdrive.Update(CurrentTime);
}

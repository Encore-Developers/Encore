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
    InputChannel channel,
    Action action
) {
    if (channel == InputChannel::OVERDRIVE && action == Action::PRESS) {
        stats->overdrive.Activate(stats->InputTime); // time
        return true;
    }
    return false;
}

void Encore::RhythmEngine::DrumsEngine::SetStatsInputState(
    InputChannel channel,
    Action action
) {
    stats->InputTime = TheSongTime.GetElapsedTime(); // todo: REPLACE WITH ACTUAL SONG
    // TIME (IN SECONDS)
    if (action == Action::PRESS) {
        switch (channel) {
        case InputChannel::LANE_1:
        case InputChannel::LANE_2:
        case InputChannel::LANE_3:
        case InputChannel::LANE_4:
        case InputChannel::LANE_5: {
            stats->HeldFrets.at(ICInt(channel)) = true;
            break;
        }
        default:
            break;
        }
    }
    if (action == Action::RELEASE) {
        switch (channel) {
        case InputChannel::LANE_1:
        case InputChannel::LANE_2:
        case InputChannel::LANE_3:
        case InputChannel::LANE_4:
        case InputChannel::LANE_5: {
            stats->HeldFrets.at(ICInt(channel)) = false;
            break;
        }
        default:
            break;
        }
    }
}

int Encore::RhythmEngine::DrumsEngine::RunHitStateCheck(
    InputChannel channel,
    Action action
) {
    if (channel == InputChannel::STRUM_UP || channel == InputChannel::STRUM_DOWN)
        return CheckNextInput;
    int lane = ICInt(channel);
    // auto &chartLane = chart->at(lane);
    // if (chartLane.empty())
    //     return CheckNextInput;
    EncNote *CurrentNote = &*chart->CurrentNoteIterators.at(lane);
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
    if (action == Action::PRESS) {
        if (EarlyStrike(CurrentNote->StartSeconds)) {
            Overhit(lane);
            return OverhitNote;
        };
        if (InHitwindow(CurrentNote->StartSeconds)) {
            HitNote(lane);
            return HitState::HitNote;
        };
    }
    return CheckNextInput;
}

void Encore::RhythmEngine::DrumsEngine::UpdateOnFrame(double CurrentTime) {
    for (int Lane = 0; Lane < chart->Lanes.size(); Lane++) {
        if (stats->Bot) {
            if (chart->CurrentNoteIterators.at(Lane) < chart->Lanes.at(Lane).cend()) {
                EncNote *CurrentNote = &*chart->CurrentNoteIterators.at(Lane);
                if (CurrentNote->StartSeconds <= CurrentTime) {
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

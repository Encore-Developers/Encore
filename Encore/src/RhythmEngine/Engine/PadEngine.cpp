//
// Created by maria on 13/06/2025.
//

#include "PadEngine.h"
//
// Created by maria on 01/06/2025.
//

#include "timingvalues.h"
#include "gameplay/enctime.h"

bool Encore::RhythmEngine::PadEngine::ActivateOverdrive(
    InputChannel channel, Action action
) {
    if (stats->OverdriveFill >= 0.25 && channel == InputChannel::OVERDRIVE
        && action == Action::PRESS) {
        stats->OverdriveActive = true;
        stats->OverdriveActivationTime = TheSongTime.GetElapsedTime(); // todo: set to
                                                                    // current input time
        return true;
    }
    return false;
}
void Encore::RhythmEngine::PadEngine::SetStatsInputState(
    InputChannel channel, Action action
) {
    stats->InputTime = TheSongTime.GetElapsedTime(); // todo: REPLACE WITH ACTUAL SONG TIME
                                                  // (IN SECONDS)
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

int Encore::RhythmEngine::PadEngine::RunHitStateCheck(
    InputChannel channel, Action action
) {
    if (channel == InputChannel::STRUM_UP || channel == InputChannel::STRUM_DOWN)
        return CheckNextInput;
    int lane = ICInt(channel);
    auto &chartLane = chart->at(lane);
    if (chartLane.empty())
        return CheckNextInput;

    auto curNoteItr = chartLane.begin();
    while (curNoteItr->StartSeconds + goodBackend
           < TheSongTime.GetElapsedTime() - stats->InputOffset) {
        if (curNoteItr + 1 == chartLane.end()) {
            return CheckNextInput;
        }
        ++curNoteItr;
    }
    EncNote &CurrentNote = *curNoteItr;
    bool lift = action == Action::RELEASE && CurrentNote.NoteType == 1;
    if (action == Action::PRESS || lift) {
        if (EarlyStrike(CurrentNote.StartSeconds, stats->InputTime, stats->InputOffset) && !lift) {
            chart->overdrive.UpdateEventViaNote(false, CurrentNote.StartTicks);
            return OverhitNote;
        };
        if (InHitwindow(CurrentNote.StartSeconds, stats->InputTime, stats->InputOffset)) {
            stats->HitNote(1);
            chart->overdrive.UpdateEventViaNote(true, CurrentNote.StartTicks);
            chartLane.erase(curNoteItr);
            return HitState::HitNote;
        };
    }
    return CheckNextInput;
}

void Encore::RhythmEngine::PadEngine::UpdateOnFrame(double CurrentTime) {
    for (int Lane = 0; Lane < chart->size(); Lane++) {
        CheckMissedNotes(Lane, TheSongTime.GetElapsedTime());
    }
    stats->OverdriveFill += chart->overdrive.CheckOverdrive(CurrentTime);
    if (stats->OverdriveFill > 1.0) stats->OverdriveFill = 1.0;
}

void Encore::RhythmEngine::PadEngine::HitNote() {
    // stats->HitNote(std::popcount(chart->at(0).front().Lane));
    // todo: you know the drill, maybe set track state too with this info?
    // stats->FretAfterStrum = false;
    // chart->at(0).erase(chart->at(0).begin());
}

void Encore::RhythmEngine::PadEngine::Overhit() {
    stats->Overhit();
}
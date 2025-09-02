//
// Created by maria on 13/06/2025.
//

#include "PadEngine.h"
//
// Created by maria on 01/06/2025.
//

#include "timingvalues.h"
#include "gameplay/enctime.h"
#include "song/scoring.h"

bool Encore::RhythmEngine::PadEngine::ActivateOverdrive(
    InputChannel channel, Action action
) {
    // todo: hit notes (THIS IS PAD)
    stats->InputTime = TheSongTime.GetElapsedTime();
    if (channel == InputChannel::OVERDRIVE && action == Action::PRESS) {
        // activates overdrive
        if (stats->overdrive.Activate(stats->InputTime)) {
            for (int lane = 0; lane < chart->Lanes.size(); lane++) {
                EncNote *CurrentNote = &*chart->CurrentNoteIterators.at(lane);
                if (InHitwindow(CurrentNote->StartSeconds)) {
                    this->HitNote(lane);

                };
            }
            return true;
        };
    }
    if (channel == InputChannel::OVERDRIVE && action == Action::RELEASE) {
        if (stats->overdrive.UseOverdriveLift) {
            for (int lane = 0; lane < chart->Lanes.size(); lane++) {
                EncNote *CurrentNote = &*chart->CurrentNoteIterators.at(lane);
                if (InHitwindow(CurrentNote->StartSeconds) && CurrentNote->NoteType == 1) {
                    Timers["LOP"].ActivateTimer(stats->InputTime);
                    this->HitNote(lane);
                };
            }
            stats->overdrive.UseOverdriveLift = false;
            return true;
        };
    }
    return false;
}
void Encore::RhythmEngine::PadEngine::SetStatsInputState(
    InputChannel channel, Action action
) {
    stats->InputTime = TheSongTime.GetElapsedTime();
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
            if (chart->HeldNotePointers.at(ICInt(channel))) {
                chart->HeldNotePointers.at(ICInt(channel)) = nullptr;
            }
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
    if (channel == InputChannel::OVERDRIVE)
        return CheckNextInput;
    if (channel == InputChannel::STRUM_UP || channel == InputChannel::STRUM_DOWN)
        return CheckNextInput;
    int lane = ICInt(channel);
    if (chart->CurrentNoteIterators.at(lane) == chart->Lanes.at(lane).end())
        return CheckNextInput;
    EncNote *CurrentNote = &*chart->CurrentNoteIterators.at(lane);
    bool lift = action == Action::RELEASE && CurrentNote->NoteType == 1;
    if (action == Action::PRESS || lift) {
        if (EarlyStrike(CurrentNote->StartSeconds) && !lift) {
            if (stats->overdrive.ActivationTime + overdriveHitLeniency > stats->InputTime - stats->InputOffset )
                return CheckNextInput;
            if (Timers["LOP"].CanBeUsedUp(stats->InputTime)) {
                Timers["LOP"].ResetTimer();
                return CheckNextInput;
            }
            Overhit(lane);
            return OverhitNote;
        };
        if (InHitwindow(CurrentNote->StartSeconds)) {
            this->HitNote(lane);
            return HitState::HitNote;
        };
    }
    return CheckNextInput;
}

void Encore::RhythmEngine::PadEngine::HitNote(int lane) {
    if (chart->CurrentNoteIterators.at(lane)->LengthTicks > 0) {
        chart->SetCurrentNoteAsHeldNote(lane);
    }
    BaseEngine::HitNote(lane);
}

void Encore::RhythmEngine::PadEngine::UpdateOnFrame(double CurrentTime) {
    for (int Lane = 0; Lane < chart->Lanes.size(); Lane++) {
        if (stats->Bot) {
            if (chart->CurrentNoteIterators.at(Lane) == chart->Lanes.at(Lane).end())
                continue;
            EncNote *CurrentNote = &*chart->CurrentNoteIterators.at(Lane);
            if (CurrentNote->StartSeconds <= CurrentTime) {
                HitNote(true);
            }
        }
        if (chart->HeldNotePointers.at(Lane) != nullptr
            && chart->HeldNotePointers.at(Lane)->StartSeconds
                    + chart->HeldNotePointers.at(Lane)->LengthSeconds
                >= CurrentTime) {
            double PointsPerTick = double(SUSTAIN_POINTS_PER_BEAT) / 480.0;
            stats->Score +=
                (TheSongTime.CurrentTick - TheSongTime.LastTick) * (PointsPerTick * stats->multiplier());
        }
        CheckMissedNotes(Lane, CurrentTime);
    }
    stats->overdrive.Add(CurrentTime, chart);
    stats->overdrive.Update(CurrentTime);
}

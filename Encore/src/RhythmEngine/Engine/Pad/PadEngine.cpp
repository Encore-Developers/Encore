//
// Created by maria on 13/06/2025.
//

#include "PadEngine.h"
//
// Created by maria on 01/06/2025.
//

#include "RhythmEngine/timingvalues.h"
#include "gameplay/enctime.h"
#include "RhythmEngine/scoring.h"

bool Encore::RhythmEngine::PadEngine::ActivateOverdrive(ControllerEvent &event) {
    // todo: hit notes (THIS IS PAD)
    stats->InputTime = event.timestamp - stats->InputOffset;
    if (event.channel == InputChannel::OVERDRIVE && event.action == Action::PRESS) {
        // activates overdrive
        if (stats->overdrive.Activate(stats->InputTime)) {
            HighwayBounceEvent HBevent(2.0f, 2.5f);
            FireEvent(&HBevent);
            for (size_t lane = 0; lane < chart->Lanes.size(); lane++) {
                if (chart->CurrentNoteIterators.at(lane) != chart->Lanes.at(lane).end()){
                    NoteEvent *CurrentNote = &*chart->CurrentNoteIterators.at(lane);
                    if (InHitwindow(CurrentNote->start.sec)) {
                        this->HitNote(lane);
                    };
                }
            }
            return true;
        };
    }
    if (event.channel == InputChannel::OVERDRIVE && event.action == Action::RELEASE) {
        if (stats->overdrive.UseOverdriveLift) {
            for (size_t lane = 0; lane < chart->Lanes.size(); lane++) {
                if (chart->CurrentNoteIterators.at(lane) != chart->Lanes.at(lane).end()) {
                    NoteEvent *CurrentNote = &*chart->CurrentNoteIterators.at(lane);
                    if (InHitwindow(CurrentNote->start.sec) && CurrentNote->type ==
                        NoteEvent::LIFT) {
                        Timers["LOP"].ActivateTimer(stats->InputTime);
                        this->HitNote(lane);
                        };
                }
            }
            stats->overdrive.UseOverdriveLift = false;
            return true;
        };
    }
    return false;
}

void Encore::RhythmEngine::PadEngine::SetStatsInputState(ControllerEvent &event) {
    stats->InputTime = event.timestamp - stats->InputOffset;
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
            if (chart->HeldNotePointers.at(ICInt(event.channel))) {
                chart->HeldNotePointers.at(ICInt(event.channel)) = nullptr;
            }
            stats->HeldFrets.at(ICInt(event.channel)) = false;
            break;
        }
        default:
            break;
        }
    }
}

int Encore::RhythmEngine::PadEngine::RunHitStateCheck(ControllerEvent &event) {
    if (event.channel == InputChannel::OVERDRIVE)
        return CheckNextInput;
    if (event.channel == InputChannel::STRUM_UP || event.channel ==
        InputChannel::STRUM_DOWN)
        return CheckNextInput;
    size_t lane = ICInt(event.channel);
    if (lane >= chart->Lanes.size())
        return CheckNextInput;

    if (chart->CurrentNoteIterators.at(lane) == chart->Lanes.at(lane).end())
        return CheckNextInput;
    NoteEvent *CurrentNote = &*chart->CurrentNoteIterators.at(lane);
    bool lift = event.action == Action::RELEASE && CurrentNote->type == 1;
    if (event.action == Action::PRESS || lift) {
        if (EarlyStrike(CurrentNote->start.sec) && !lift) {
            if (stats->overdrive.ActivationTime + overdriveHitLeniency > stats->InputTime)
                return CheckNextInput;
            if (Timers["LOP"].CanBeUsedUp(stats->InputTime)) {
                Timers["LOP"].ResetTimer();
                return CheckNextInput;
            }
            Overhit(lane);
            return OverhitNote;
        };
        if (InHitwindow(CurrentNote->start.sec)) {
            this->HitNote(lane);
            return HitState::HitNote;
        };
    }
    return CheckNextInput;
}

void Encore::RhythmEngine::PadEngine::HitNote(const size_t lane) {
    if (chart->CurrentNoteIterators.at(lane) != chart->Lanes.at(lane).end()) {
        if (chart->CurrentNoteIterators.at(lane)->secLen() > 0) {
            chart->SetCurrentNoteAsHeldNote(lane);
        }
    }
    BaseEngine::HitNote(lane);
}

void Encore::RhythmEngine::PadEngine::UpdateOnFrame(const double CurrentTime) {
    LastUpdateTime = CurrentTime - stats->InputOffset;
    for (size_t Lane = 0; Lane < chart->Lanes.size(); Lane++) {
        if (stats->Bot) {
            if (chart->CurrentNoteIterators.at(Lane) == chart->at(Lane).end())
                continue;
            stats->InputTime = LastUpdateTime;
            while (chart->CurrentNoteIterators.at(Lane)->start.sec <= LastUpdateTime) {
                HitNote(Lane);
                if (chart->CurrentNoteIterators.at(Lane) == chart->at(Lane).end())
                    break;
            }
        }
        if (chart->HeldNotePointers.at(Lane) != nullptr
            && chart->HeldNotePointers.at(Lane)->end.sec
            >= LastUpdateTime) {
            constexpr double PointsPerTick = double(SUSTAIN_POINTS_PER_BEAT) / 480.0;
            stats->Score +=
                (TheSongTime.CurrentTick - TheSongTime.LastTick) * (PointsPerTick * stats
                    ->multiplier());
        }
        CheckMissedNotes(Lane, LastUpdateTime);
    }
    BaseUpdateOnFrame(LastUpdateTime);
}

Encore::RhythmEngine::TimePoint Encore::RhythmEngine::PadEngine::NextNoteTime() {
    return BaseEngine::NextNoteTime();
}

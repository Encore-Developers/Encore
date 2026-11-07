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

void Encore::RhythmEngine::PadEngine::OnOverdriveActivate(TimeSeconds time) {
    for (size_t lane = 0; lane < chart->Lanes.size(); lane++) {
        if (chart->CurrentNoteIterators.at(lane) != chart->Lanes.at(lane).end()){
            NoteEvent *CurrentNote = &*chart->CurrentNoteIterators.at(lane);
            if (InHitwindow(CurrentNote->start.sec)) {
                this->HitNote(lane);
            };
        }
    }
    OverdriveLiftInput = true;
}

void Encore::RhythmEngine::PadEngine::RunHitStateCheck(ControllerEvent &event) {
    if (event.channel == InputChannel::STRUM_UP || event.channel ==
        InputChannel::STRUM_DOWN)
        return;

    bool press = event.action == Action::PRESS;
    if (event.channel < InputChannel::LANE_6) {
        stats->HeldFrets.at(ICInt(event.channel)) = press;
    }


    if (event.channel == InputChannel::OVERDRIVE && event.action == Action::RELEASE && OverdriveLiftInput) {
        for (int i = 0; i < chart->Lanes.size(); i++) {
            LaneHitStateCheck(i, event);
        }
        OverdriveLiftInput = false;
        return;
    }
    size_t lane = ICInt(event.channel);
    if (lane >= chart->Lanes.size())
        return;
    LaneHitStateCheck(lane, event);

}
void Encore::RhythmEngine::PadEngine::LaneHitStateCheck(int lane, ControllerEvent &event) {
    if (chart->CurrentNoteIterators.at(lane) == chart->Lanes.at(lane).end())
        return;
    NoteEvent *CurrentNote = &*chart->CurrentNoteIterators.at(lane);
    if (event.action == Action::RELEASE) {
        for (auto& sustain : ActiveSustains) {
            if (sustain.note->lane == PlasticFrets[lane]) {
                DropSustain(EngineTime, sustain);
                break;
            }
        }
    }
    bool lift = event.action == Action::RELEASE && CurrentNote->type == 1;

    if (event.action == Action::PRESS || lift) {
        if (EarlyStrike(CurrentNote->start.sec) && !lift) {
            if (OverdriveActivateTime + overdriveHitLeniency > EngineTime)
                return;
            if (Timers["LOP"].CanBeUsedUp(EngineTime)) {
                Timers["LOP"].ResetTimer();
                return;
            }
            Overhit(lane);
            return;
        };
        if (InHitwindow(CurrentNote->start.sec)) {
            HitNote(lane);
        };
    }
}

void Encore::RhythmEngine::PadEngine::Advance(const double CurrentTime) {
    BaseEngine::Advance(CurrentTime);
}
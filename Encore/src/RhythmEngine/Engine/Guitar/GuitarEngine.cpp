//
// Created by maria on 01/06/2025.
//

#include "GuitarEngine.h"

#include "../../timingvalues.h"
#include "gameplay/enctime.h"
#include "RhythmEngine/scoring.h"

#include <bit>

bool Encore::RhythmEngine::GuitarEngine::MaskMatch(const NoteEvent *itr, const uint8_t mask) const {
    uint8_t note = itr->lane;
    uint8_t maxMask = note << 1;
    if (std::has_single_bit(note))
        return (mask < maxMask) && (mask >= note);
    if (itr->type == 2) {
        if ((mask & ~note) < note) {
            return (mask & note) == note;
        }
    }
    return mask == note;
}

bool Encore::RhythmEngine::GuitarEngine::IsEarly() const {
    if (chart->CurrentNoteIterators.at(0)->start - goodFrontend > EngineTime) {
        return true;
    }
    return false;
}

bool HittableAsStrumless(const uint8_t type, const bool CanHitHopo) {
    if (type == Encore::RhythmEngine::NoteEvent::NORMAL) return false;
    if (CanHitHopo && type == Encore::RhythmEngine::NoteEvent::HOPO) return true;
    if (type == Encore::RhythmEngine::NoteEvent::TAP) return true;
    return false;
}


bool Encore::RhythmEngine::GuitarEngine::IsInputTooEarly() const {
    if (chart->CurrentNoteIterators.at(0) == chart->Lanes.at(0).end())
        return false;
    NoteEvent &CurrentNote = *chart->CurrentNoteIterators.at(0);
    if (chart->CurrentNoteIterators.at(0) + 1 != chart->Lanes.at(0).end()) {
        auto NextNote = chart->CurrentNoteIterators.at(0);
        NextNote += 1;
        double noteMiddlePoint = (NextNote->start - CurrentNote.start).sec / dynamicHitwindowRatio;
        if (noteMiddlePoint < minimumHitwindowSize) noteMiddlePoint = minimumHitwindowSize;
        if (CurrentNote.start - noteMiddlePoint > EngineTime) {
            return true;
        }
    }
    if (CurrentNote.start - goodFrontend > EngineTime) {
        return true;
    }
    return false;
}

void Encore::RhythmEngine::GuitarEngine::Advance(double CurrentTime) {
    BaseEngine::Advance(CurrentTime);
}

/*
bool Encore::RhythmEngine::GuitarEngine::CanNoteBeHit() {
    return true;
}*/

void Encore::RhythmEngine::GuitarEngine::RunHitStateCheck(ControllerEvent &event
) {
    bool press = event.action == Action::PRESS;
    if (event.channel < InputChannel::LANE_6) {
        stats->HeldFrets.at(ICInt(event.channel)) = press;
    }
    if (chart->CurrentNoteIterators.at(0) == chart->Lanes.at(0).end())
        return;
    NoteEvent *CurrentNote = &*chart->CurrentNoteIterators.at(0);

    uint8_t pMask = stats->HeldFretsArrayToMask();

    // if (chart->IsHeldNotePresent(0)) {
    //     auto *held = chart->HeldNotePointers.at(0);
    //
    //     // second note:
    //     // what the fuck is this. i cannot read this
    //     if (press &&
    //
    //         // if the frets dont match
    //         !MaskMatch(held, pMask) &&
    //
    //         // is not an extended sustain
    //         held->end <= CurrentNote->start &&
    //
    //         // is before the end of the sustain
    //         held->end >= EngineTime
    //     ) {
    //         chart->DropSustain(0);
    //     }
    //
    //
    //     if (held->end > CurrentNote->start) {
    //         pMask &= ~held->lane;
    //     };
    // }

    bool early = IsEarly();
    // STRUM PATH
    bool StrumInput = press
        && (event.channel == InputChannel::STRUM_UP || event.channel == InputChannel::STRUM_DOWN);

    if (StrumInput) {
        if (Timers["SAH"].CanBeUsedUp(EngineTime)) {
            Timers["SAH"].ResetTimer();
            Log::Trace("SAH Disabled");
            return;
        }
        // miss should be managed by current frame
        // overhit is managed here
        if (early) {
            Overhit();
            return;
        }
        // if frets match, continue and try to hit
        if (!MaskMatch(CurrentNote, pMask)) {
            Timers["FAS"].ActivateTimer(EngineTime);
            Log::Trace("FAS Enabled");
            return;
        }
    }
    if (early) return;
    if (!MaskMatch(CurrentNote, pMask)) return;
    // if FAS is active, or if there was a strum
    // really couldve just put it up there LMFAO
    bool strum = Timers["FAS"].CanBeUsedUp(EngineTime) || StrumInput;
    bool fret = HittableAsStrumless(CurrentNote->type, stats->CanHitHopo);

    if ((fret || strum || player->bindingType == PAD)) {
        GuitarHitNote(StrumInput);
    }
}

void Encore::RhythmEngine::GuitarEngine::GuitarHitNote(bool strumInput) {
    GhostCount = 0;
    if ((chart->CurrentNoteIterators.at(0)->type == NoteEvent::HOPO || chart->CurrentNoteIterators.
        at(0)->type == NoteEvent::TAP) && !strumInput) {
        if (chart->CurrentNoteIterators.at(0) < chart->Lanes.at(0).end() - 1) {
            double nextNoteTime = (chart->CurrentNoteIterators.at(0)+1)->start.sec;
            double curNoteTime = chart->CurrentNoteIterators.at(0)->start.sec;
            double midpoint = (nextNoteTime + curNoteTime) / 2;
            double duration = midpoint - EngineTime;
            if (duration > goodFrontend) {
                duration = goodFrontend;
            }
            Timers["SAH"].Duration = duration;
            Timers["SAH"].ActivateTimer(EngineTime);
            Log::Trace("SAH Enabled");
        }
    }
    if (Timers["FAS"].CanBeUsedUp(EngineTime)) {
        Timers["FAS"].ResetTimer();
        Log::Trace("FAS Disabled");
    }
    HitNote(0);
}

void Encore::RhythmEngine::GuitarEngine::Overhit() {
    // TODO: drop all sustains
    BaseEngine::Overhit(0);
}

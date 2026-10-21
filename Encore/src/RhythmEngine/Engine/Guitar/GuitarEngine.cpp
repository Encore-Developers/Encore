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
    return mask == note;
}

bool Encore::RhythmEngine::GuitarEngine::IsEarly() const {
    if (chart->CurrentNoteIterators.at(0)->start - goodFrontend > stats->InputTime) {
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

bool Encore::RhythmEngine::GuitarEngine::ActivateOverdrive(ControllerEvent &event
) {
    if (event.channel == InputChannel::OVERDRIVE && event.action == Action::PRESS) {
        // int InstrumentNum = stats->Type == Guitar ? inst - 5 : inst;
        if (stats->overdrive.Activate(stats->InputTime)) {
            OverdriveEvent activate;
            FireEvent(&activate);
            // TheAudioManager.StartEffect(TheAudioManager.GetAudioStreamByInstrument(inst));
        }
        return true;
    }
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
        if (CurrentNote.start - noteMiddlePoint > stats->InputTime) {
            return true;
        }
    }
    if (CurrentNote.start - goodFrontend > stats->InputTime) {
        return true;
    }
    return false;
}

void Encore::RhythmEngine::GuitarEngine::CheckMissedNotes(double CurrentTime) {
    // if (!player->BrutalMode) {
    //    BaseEngine::CheckMissedNotes(0, CurrentTime);
   // }
    if (chart->CurrentNoteIterators.at(0) == chart->Lanes.at(0).end())
        return;
    NoteEvent &CurrentNote = *chart->CurrentNoteIterators.at(0);
    bool MissedNote = false;
    if (chart->CurrentNoteIterators.at(0) + 1 != chart->Lanes.at(0).end()) {
        auto NextNote = chart->CurrentNoteIterators.at(0);
        NextNote += 1;
        double noteMiddlePoint = (NextNote->start - CurrentNote.start).sec / dynamicHitwindowRatio;
        if (noteMiddlePoint < minimumHitwindowSize) noteMiddlePoint = minimumHitwindowSize;
        if (CurrentNote.start + noteMiddlePoint < CurrentTime) {
            MissedNote = true;
        }
    }
    // testing
    if (CurrentNote.start.sec + goodBackend < CurrentTime
        && &CurrentNote != chart->HeldNotePointers.at(0)) {
        MissedNote = true;
        }
    if (!MissedNote) return;
    GhostCount = 0;
    MissNote(0);
}


void Encore::RhythmEngine::GuitarEngine::UpdateOnFrame(double CurrentTime) {
    this->LastUpdateTime = CurrentTime - stats->InputOffset;
    if (stats->Bot) {
        if (chart->CurrentNoteIterators.at(0) == chart->Lanes.at(0).end())
            return;
        NoteEvent *CurrentNote = &*chart->CurrentNoteIterators.at(0);
        if (CurrentNote->start.sec - goodBackend < LastUpdateTime) {
            for (int g = 0; g < 5; g++) {
                if (CurrentNote->lane & PlasticFrets[g]) {
                    stats->HeldFrets.at(g) = true;
                } else {
                    stats->HeldFrets.at(g) = false;
                }
            }
        }
        while (chart->CurrentNoteIterators.at(0)->start.sec <= CurrentTime) {
            stats->InputTime = CurrentTime;
            HitNote(true);
            if (chart->CurrentNoteIterators.at(0) == chart->Lanes.at(0).end())
                break;
        }
    }
    auto heldNote = chart->HeldNotePointers.at(0);
    if (heldNote != nullptr
        && heldNote->end.sec >= CurrentTime
        && heldNote->start.sec <= CurrentTime) {
        constexpr double PointsPerTick = double(SUSTAIN_POINTS_PER_BEAT) / 480.0;
        int chordMult = heldNote->lane == 0 ? 25 : std::popcount(heldNote->lane);
        stats->Score += (TheSongTime.CurrentTick - TheSongTime.LastTick) * ((PointsPerTick
            * stats->multiplier()) * chordMult);
        stats->Health += (TheSongTime.CurrentTick - TheSongTime.LastTick) * (PointsPerTick * 0.0005);
        if (stats->Health > 1) stats->Health = 1;
    }
    if (heldNote && heldNote->end.sec <= CurrentTime) {
        chart->DropSustain(0);
    }
    this->CheckMissedNotes(CurrentTime);
    BaseUpdateOnFrame(CurrentTime);
    // there is ONLY lane 0 for guitar
}

void Encore::RhythmEngine::GuitarEngine::SetStatsInputState(
    ControllerEvent &event
) {
    stats->InputTime = event.timestamp - stats->InputOffset; // todo: REPLACE WITH ACTUAL SONG
    if (event.channel == InputChannel::INVALID) return;
    if (event.channel == InputChannel::LANE_6) return;
    // TIME
    // (IN SECONDS)
    if (event.channel == InputChannel::WHAMMY && chart->IsHeldNotePresent(0)) {
        whammy = float(event.axis) / 255.0f;
        return;
    }

    bool press = event.action == Action::PRESS;
    if (event.channel < InputChannel::LANE_6) {
        stats->HeldFrets.at(ICInt(event.channel)) = press;
        if (press) return;

        // sustain release logic
        if (!chart->HeldNotePointers.at(0)) return;
        auto note = chart->HeldNotePointers.at(0);
        if ( note->lane != 0 && note->lane & PlasticFrets[ICInt(event.channel)] &&
            note->end.tick >= TheSongTime.CurrentTick + 240)
        {
            chart->DropSustain(0);
        }
        return;
    }


    if (event.channel == InputChannel::STRUM_UP)
        stats->strumState = press ? StrumState::UpStrum : StrumState::Default;
    else if (event.channel == InputChannel::STRUM_DOWN)
        stats->strumState = press ? StrumState::DownStrum : StrumState::Default;

}

Encore::RhythmEngine::TimePoint Encore::RhythmEngine::GuitarEngine::NextNoteTime() {
    if (chart->CurrentNoteIterators.at(0) != chart->Lanes.at(0).end()) {
        return chart->CurrentNoteIterators.at(0)->start;
    }
    return {0,0};
}

Encore::RhythmEngine::TimePoint Encore::RhythmEngine::GuitarEngine::LastNoteTime() {
    if (chart->at(0).begin() == chart->CurrentNoteIterators.at(0))
        return {0,0};
    auto end = (chart->CurrentNoteIterators.at(0)-1)->end;
    if (end == 0) {
        end = (chart->CurrentNoteIterators.at(0)-1)->start;
    }
    return end;
}

/*
bool Encore::RhythmEngine::GuitarEngine::CanNoteBeHit() {
    return true;
}*/

int Encore::RhythmEngine::GuitarEngine::RunHitStateCheck(ControllerEvent &event
) {
    // GetCurrentNote(0);
    CheckMissedNotes(stats->InputTime);
    if (chart->CurrentNoteIterators.at(0) == chart->Lanes.at(0).end())
        return CheckNextInput;
    NoteEvent *CurrentNote = &*chart->CurrentNoteIterators.at(0);

    uint8_t pMask = stats->HeldFretsArrayToMask();
    bool press = event.action == Action::PRESS;

    if (chart->IsHeldNotePresent(0)) {
        auto *held = chart->HeldNotePointers.at(0);

        // second note:
        // what the fuck is this. i cannot read this
        if (press &&

            // if the frets dont match
            !MaskMatch(held, pMask) &&

            // is not an extended sustain
            held->end <= CurrentNote->start &&

            // is before the end of the sustain
            held->end >= stats->InputTime
        ) {
            chart->DropSustain(0);
        }


        if (held->end > CurrentNote->start) {
            pMask &= ~held->lane;
        };
    }

    bool early = IsEarly();
    // STRUM PATH
    bool StrumInput = press
        && (event.channel == InputChannel::STRUM_UP || event.channel == InputChannel::STRUM_DOWN);

    if (StrumInput) {
        if (Timers["SAH"].CanBeUsedUp(stats->InputTime)) {
            Timers["SAH"].ResetTimer();
            Log::Trace("SAH Disabled");
            return CheckNextInput;
        }
        // miss should be managed by current frame
        // overhit is managed here
        if (early) {
            Overhit();
            return OverhitNote;
        }
        // if frets match, continue and try to hit
        if (!MaskMatch(CurrentNote, pMask)) {
            Timers["FAS"].ActivateTimer(stats->InputTime);
            Log::Trace("FAS Enabled");
            return CheckNextInput;
        }
    }
    if (early) return CheckNextInput;
    if (!MaskMatch(CurrentNote, pMask)) return CheckNextInput;
    // if FAS is active, or if there was a strum
    // really couldve just put it up there LMFAO
    bool strum = Timers["FAS"].CanBeUsedUp(stats->InputTime) || StrumInput;
    bool fret = HittableAsStrumless(CurrentNote->type, stats->CanHitHopo);

    if ((fret || strum || player->bindingType == PAD)) {
        HitNote(StrumInput);
        return HitState::HitNote;
    }

    // GHOSTING -- this doesnt work, please fixme
    // if (event.action == Action::RELEASE && event.channel <= InputChannel::LANE_5 && event.
    //     channel !=
    //     InputChannel::INVALID) {
    //     if (pMask > CurrentNote->lane) {
    //         GhostCount++;
    //     }
    // }

    return CheckNextInput;
}

void Encore::RhythmEngine::GuitarEngine::HitNote(bool strumInput) {
    GhostCount = 0;
    whammy = 0;
    if ((chart->CurrentNoteIterators.at(0)->type == NoteEvent::HOPO || chart->CurrentNoteIterators.
        at(0)->type == NoteEvent::TAP) && !strumInput) {
        if (chart->CurrentNoteIterators.at(0) < chart->Lanes.at(0).end() - 1) {
            double nextNoteTime = (chart->CurrentNoteIterators.at(0)+1)->start.sec;
            double curNoteTime = chart->CurrentNoteIterators.at(0)->start.sec;
            double midpoint = (nextNoteTime + curNoteTime) / 2;
            double duration = midpoint - stats->InputTime;
            if (duration > goodFrontend) {
                duration = goodFrontend;
            }
            Timers["SAH"].Duration = duration;
            Timers["SAH"].ActivateTimer(stats->InputTime);
            Log::Trace("SAH Enabled");
        }
    }
    if (Timers["FAS"].CanBeUsedUp(stats->InputTime)) {
        Timers["FAS"].ResetTimer();
        Log::Trace("FAS Disabled");
    }
    if (chart->CurrentNoteIterators.at(0)->tickLen() > 0) {
        if (chart->IsHeldNotePresent(0)) {
            chart->HeldNotePointers.at(0)->lane |= chart->CurrentNoteIterators.at(0)->
                                                          lane;
        } else {
            chart->SetCurrentNoteAsHeldNote(0);
        }
    }
    BaseEngine::HitNote(0);
}

void Encore::RhythmEngine::GuitarEngine::Overhit() {
    chart->DropSustain(0);
    BaseEngine::Overhit(0);
}

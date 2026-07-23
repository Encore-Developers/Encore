//
// Created by maria on 01/06/2025.
//

#include "GuitarEngine.h"

#include "../../timingvalues.h"
#include "gameplay/enctime.h"
#include "RhythmEngine/scoring.h"

#include <bit>

bool MaskMatch(uint8_t noteMask, uint8_t playerMask) {
    // chord check
    if (std::has_single_bit(noteMask)) {
        return (playerMask < noteMask * 2) && (playerMask >= noteMask);
        // since its a single note, the mask just needs to be lesser than the
        // note's mask * 2 AND equal to or above the current note's mask
    }
    return playerMask == noteMask;
    // if its a chord just. go wild lmfao
}

bool IsEarly(const double noteTime, const double currentTime) {
    if (noteTime - goodFrontend > currentTime) {
        return true;
    }
    return false;
}

bool HittableAsHopo(uint8_t NoteType, bool CanHitHopo, int GhostCount) {
    if (CanHitHopo > 0 && NoteType == Encore::RhythmEngine::NoteEvent::HOPO  && GhostCount < 4)
        return true;
    return false;
}

bool HittableAsTap(uint8_t NoteType) {
    if (NoteType == Encore::RhythmEngine::NoteEvent::TAP)
        return true;
    return false;
}

bool HittableAsStrum(const int NoteType, const bool FAS, const double inputTime, const double FASTime) {
    // checks if FAS is active
    if (FAS && inputTime < FASTime + fretAfterStrumTime && NoteType == 0) {
        return true;
    }
    return false;
}

bool Encore::RhythmEngine::GuitarEngine::ActivateOverdrive(ControllerEvent &event
) {
    if (event.channel == InputChannel::OVERDRIVE && event.action == Action::PRESS) {
        // int InstrumentNum = stats->Type == Guitar ? inst - 5 : inst;
        if (stats->overdrive.Activate(stats->InputTime)) {
            HighwayBounceEvent HBevent;
            FireEvent(&HBevent);
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
        double noteMiddlePoint = (NextNote->start.sec - CurrentNote.start.sec) / dynamicHitwindowRatio;
        if (noteMiddlePoint < minimumHitwindowSize) noteMiddlePoint = minimumHitwindowSize;
        if (CurrentNote.start.sec - noteMiddlePoint > stats->InputTime) {
            return true;
        }
    }
    if (CurrentNote.start.sec - goodFrontend > stats->InputTime) {
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
        double noteMiddlePoint = (NextNote->start.sec - CurrentNote.start.sec) / dynamicHitwindowRatio;
        if (noteMiddlePoint < minimumHitwindowSize) noteMiddlePoint = minimumHitwindowSize;
        if (CurrentNote.start.sec + noteMiddlePoint < CurrentTime) {
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
    // TIME
    // (IN SECONDS)
    if (event.channel == InputChannel::WHAMMY && chart->IsHeldNotePresent(0)) {
        whammy = float(event.axis) / 255.0f;
    }
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
        case InputChannel::STRUM_UP: {
            stats->strumState = StrumState::UpStrum;
            break;
        }
        case InputChannel::STRUM_DOWN: {
            stats->strumState = StrumState::DownStrum;
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
            if (chart->HeldNotePointers.at(0)) {
                auto note = chart->HeldNotePointers.at(0);
                if ( note->lane != 0 && note->lane & PlasticFrets[ICInt(event.channel)] &&
                    note->end.tick >= TheSongTime.CurrentTick +
                    240) {
                    chart->DropSustain(0);
                }
            }
            stats->HeldFrets.at(ICInt(event.channel)) = false;
            break;
        }
        case InputChannel::STRUM_UP:
        case InputChannel::STRUM_DOWN: {
            stats->strumState = StrumState::Default;
            break;
        }
        default:
            break;
        }
    }
}

Encore::RhythmEngine::TimePoint Encore::RhythmEngine::GuitarEngine::NextNoteTime() {
    return TimePoint(chart->CurrentNoteIterators.at(0)->start.sec, chart->CurrentNoteIterators.at(0)->start.tick);
}

Encore::RhythmEngine::TimePoint Encore::RhythmEngine::GuitarEngine::LastNoteTime() {
    if (chart->at(0).begin() == chart->CurrentNoteIterators.at(0))
        return TimePoint{0,0};
    double endTime = (chart->CurrentNoteIterators.at(0)-1)->end.sec;
    int endTick = (chart->CurrentNoteIterators.at(0)-1)->end.tick;
    if (endTick == 0) {
        endTime = (chart->CurrentNoteIterators.at(0)-1)->start.sec;
        endTick = (chart->CurrentNoteIterators.at(0)-1)->start.tick;
    }
    return TimePoint{endTime, endTick};
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

    bool extSus = false;
    if (chart->IsHeldNotePresent(0)) {
        if (chart->HeldNotePointers.at(0)->end.sec > CurrentNote->start.sec) {
                extSus = true;
            };

        // second note:
        // what the fuck is this. i cannot read this
        if (event.action == Action::PRESS &&

            // if the frets dont match
            !MaskMatch(chart->HeldNotePointers.at(0)->lane,
                       stats->HeldFretsArrayToMask()) &&

            // is not an extended sustain
            !(chart->HeldNotePointers.at(0)->end.sec > CurrentNote->start.sec) &&

            // is before the end of the sustain
            (chart->HeldNotePointers.at(0)->end.sec >= TheSongTime.CurrentTick + SUSTAIN_DROP_THRESHOLD)
        ) {
            chart->DropSustain(0);
        }
    }
    uint8_t pMask = stats->HeldFretsArrayToMask();
    // if an upper note is pressed when a sustain is active, unless the sustain goes on longer than the current note
    if (extSus) {
        pMask &= ~chart->HeldNotePointers.at(0)->lane;
    }
    // STRUM PATH
    bool StrumInput = (stats->strumState != StrumState::Default)
        && event.action == Action::PRESS
        && ((event.channel == InputChannel::STRUM_UP || event.channel ==
            InputChannel::STRUM_DOWN));
    if (StrumInput) {
        if (Timers["SAH"].CanBeUsedUp(stats->InputTime)) {
            Timers["SAH"].ResetTimer();
            Log::Trace("SAH Disabled");
            return CheckNextInput;
        }/*
        // commented out for the sake of this not overriding the miss checks
        if (stats->Combo == 0 && chart->CurrentNoteIterators.at(0) != chart->Lanes.at(0).begin()) {
            auto MissCheckNote = chart->CurrentNoteIterators.at(0);
            float offset = goodBackend;
            while (true) {
                if (MissCheckNote >= chart->Lanes.at(0).end() - 1) {
                    break;
                }
                MissCheckNote += 1;
                float nextOffset = (stats->InputTime - stats->InputOffset) - MissCheckNote->start.sec;
                if (std::abs(nextOffset) < std::abs(offset)) {
                    offset = nextOffset;
                } else {
                    MissCheckNote -= 1;
                    while (chart->CurrentNoteIterators.at(0) != MissCheckNote) {
                        MissNote(0);
                    }
                    CurrentNote = &*MissCheckNote;
                    break;
                }
            }
        }*/
        // miss should be managed by current frame
        // overhit is managed here
        if (IsEarly(CurrentNote->start.sec, stats->InputTime)) {
            Overhit();
            return OverhitNote;
        }
        // if frets match, continue and try to hit
        if (!MaskMatch(CurrentNote->lane, pMask)) {
            Timers["FAS"].ActivateTimer(stats->InputTime);
            Log::Trace("FAS Enabled");
            return CheckNextInput;
        }

    }
    // if FAS is active, or if there was a strum
    // really couldve just put it up there LMFAO
    bool strum = Timers["FAS"].CanBeUsedUp(stats->InputTime) || StrumInput;

    if (MaskMatch(CurrentNote->lane, pMask)
        && !IsEarly(CurrentNote->start.sec, stats->InputTime)
        && (HittableAsHopo(CurrentNote->type, stats->CanHitHopo, GhostCount)
            || HittableAsTap(CurrentNote->type) || strum || player->bindingType == PAD)) {
        HitNote(StrumInput);
        return HitState::HitNote;
    }

    // GHOSTING
    if (event.action == Action::PRESS && event.channel <= InputChannel::LANE_5 && event.
        channel !=
        InputChannel::INVALID) {
        if (stats->HeldFretsArrayToMask() > CurrentNote->lane) {
            GhostCount++;
        }
    }

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

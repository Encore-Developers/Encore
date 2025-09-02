//
// Created by maria on 01/06/2025.
//

#include "GuitarEngine.h"

#include "timingvalues.h"
#include "gameplay/enctime.h"
#include "song/scoring.h"

#include <bit>
#include <filesystem>

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
bool HittableAsHopo(int NoteType, int Combo, int GhostCount) {
    if (Combo > 0 && NoteType == 1  && GhostCount < 3)
        return true;
    return false;
}
bool HittableAsTap(int NoteType) {
    if (NoteType == 2)
        return true;
    return false;
}
bool HittableAsStrum(int NoteType, bool FAS, double inputTime, double FASTime) {
    // checks if FAS is active
    if (FAS && inputTime < FASTime + fretAfterStrumTime && NoteType == 0) {
        return true;
    }
    return false;
}

bool Encore::RhythmEngine::GuitarEngine::ActivateOverdrive(
    InputChannel channel, Action action
) {
    if (channel == InputChannel::OVERDRIVE && action == Action::PRESS) {
        stats->overdrive.Activate(stats->InputTime);
        return true;
    }
    return false;
}
void Encore::RhythmEngine::GuitarEngine::CheckMissedNotes(double CurrentTime) {
    GhostCount = 0;
    BaseEngine::CheckMissedNotes(0, CurrentTime);
}
void Encore::RhythmEngine::GuitarEngine::UpdateOnFrame(double CurrentTime) {
    if (stats->Bot) {
        if (chart->CurrentNoteIterators.at(0) == chart->Lanes.at(0).end())
            return;
        EncNote *CurrentNote = &*chart->CurrentNoteIterators.at(0);
        if (CurrentNote->StartSeconds <= CurrentTime) {
            HitNote(true);
        }
    }
    if (chart->HeldNotePointers.at(0) != nullptr
        && chart->HeldNotePointers.at(0)->StartSeconds
                + chart->HeldNotePointers.at(0)->LengthSeconds
            >= CurrentTime) {
        double PointsPerTick = double(SUSTAIN_POINTS_PER_BEAT) / 480.0;
        stats->Score += (TheSongTime.CurrentTick - TheSongTime.LastTick) * ((PointsPerTick * stats->multiplier()) * std::popcount(chart->HeldNotePointers.at(0)->Lane));
    }
    this->CheckMissedNotes(CurrentTime);
    stats->overdrive.Add(CurrentTime, chart);
    stats->overdrive.Update(CurrentTime);
    // there is ONLY lane 0 for guitar
}
void Encore::RhythmEngine::GuitarEngine::SetStatsInputState(
    InputChannel channel, Action action
) {
    stats->InputTime = TheSongTime.GetElapsedTime(); // todo: REPLACE WITH ACTUAL SONG
                                                     // TIME
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
    if (action == Action::RELEASE) {
        switch (channel) {
        case InputChannel::LANE_1:
        case InputChannel::LANE_2:
        case InputChannel::LANE_3:
        case InputChannel::LANE_4:
        case InputChannel::LANE_5: {
            if (chart->HeldNotePointers.at(0)) {
                chart->HeldNotePointers.at(0) = nullptr;
            }
            stats->HeldFrets.at(ICInt(channel)) = false;
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

/*
bool Encore::RhythmEngine::GuitarEngine::CanNoteBeHit() {
    return true;
}*/

int Encore::RhythmEngine::GuitarEngine::RunHitStateCheck(
    InputChannel channel, Action action
) {
    // GetCurrentNote(0);
    if (chart->CurrentNoteIterators.at(0) == chart->Lanes.at(0).end())
        return CheckNextInput;
    EncNote &CurrentNote = *chart->CurrentNoteIterators.at(0);

    // GHOSTING
    if (action == Action::RELEASE && channel <= InputChannel::LANE_5 && channel != InputChannel::INVALID) {
        if (!InHitwindow(CurrentNote.StartSeconds)) {
            GhostCount++;
        }
    }

    // STRUM PATH
    bool StrumInput = (stats->strumState != StrumState::Default)
        && action == Action::PRESS
        && (channel == InputChannel::STRUM_UP || channel == InputChannel::STRUM_DOWN);
    if (StrumInput) {
        // miss should be managed by current frame
        // overhit is managed here
        if (EarlyStrike(CurrentNote.StartSeconds)) {
            if (Timers["SAH"].CanBeUsedUp(stats->InputTime)) {
                Timers["SAH"].ResetTimer();
                TraceLog(LOG_DEBUG, "SAH Disabled");
                return CheckNextInput;
            }
            Overhit();
            return OverhitNote;
        }
        // if frets match, continue and try to hit
        if (InHitwindow(CurrentNote.StartSeconds)) {
            if (!MaskMatch(CurrentNote.Lane, stats->HeldFretsArrayToMask())) {
                Timers["FAS"].ActivateTimer(stats->InputTime);
                EncoreLog(LOG_DEBUG, "FAS Enabled");
                return CheckNextInput;
            }
        }
    }
    // if FAS is active, or if there was a strum
    // really couldve just put it up there LMFAO
    bool strum = Timers["FAS"].CanBeUsedUp(stats->InputTime) || StrumInput;

    if (MaskMatch(CurrentNote.Lane, stats->HeldFretsArrayToMask())
        && InHitwindow(CurrentNote.StartSeconds)
        && (HittableAsHopo(CurrentNote.NoteType, stats->Combo, GhostCount)
            || HittableAsTap(CurrentNote.NoteType) || strum)) {
        HitNote(StrumInput);
        return HitState::HitNote;
    }
    return CheckNextInput;
}
void Encore::RhythmEngine::GuitarEngine::HitNote(bool strumInput) {
    GhostCount = 0;
    if (chart->CurrentNoteIterators.at(0)->NoteType == 1 && !strumInput) {
        Timers["SAH"].ActivateTimer(stats->InputTime);
        EncoreLog(LOG_DEBUG, "SAH Enabled");
    }
    if (Timers["FAS"].CanBeUsedUp(stats->InputTime)) {
        Timers["FAS"].ResetTimer();
        EncoreLog(LOG_DEBUG, "FAS Disabled");
    }
    if (chart->CurrentNoteIterators.at(0)->LengthTicks > 0) {
        chart->SetCurrentNoteAsHeldNote(0);
    }
    BaseEngine::HitNote(0);
}
void Encore::RhythmEngine::GuitarEngine::Overhit() {
    if (Timers["SAH"].CanBeUsedUp(stats->InputTime)) {
        Timers["SAH"].ResetTimer();
        TraceLog(LOG_DEBUG, "SAH Disabled");
    }
    BaseEngine::Overhit(0);
}

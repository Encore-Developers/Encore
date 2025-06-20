//
// Created by maria on 01/06/2025.
//

#include "GuitarEngine.h"

#include "timingvalues.h"
#include "gameplay/enctime.h"

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
bool HittableAsHopo(int NoteType, int Combo) {
    if (Combo > 0 && NoteType == 1)
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
    if (stats->OverdriveFill >= 0.25 && channel == InputChannel::OVERDRIVE
        && action == Action::PRESS) {
        stats->OverdriveActive = true;
        stats->OverdriveActivationTime = TheSongTime.GetElapsedTime(); // todo: set to
                                                                       // current input
                                                                       // time
        return true;
    }
    return false;
}
void Encore::RhythmEngine::GuitarEngine::UpdateOnFrame(double CurrentTime) {
    CheckMissedNotes(0, TheSongTime.GetElapsedTime());
    stats->OverdriveFill += chart->overdrive.CheckOverdrive(CurrentTime);
    if (stats->OverdriveFill > 1.0)
        stats->OverdriveFill = 1.0;
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

int Encore::RhythmEngine::GuitarEngine::RunHitStateCheck(
    InputChannel channel, Action action
) {
    if (chart->at(0).empty())
        return CheckNextInput;

    auto curNoteItr = chart->at(0).begin();
    while (curNoteItr->StartSeconds + goodBackend
           < TheSongTime.GetElapsedTime() - stats->InputOffset) {
        if (curNoteItr + 1 == chart->at(0).end()) {
            return CheckNextInput;
        }
        ++curNoteItr;
    }
    EncNote &CurrentNote = *curNoteItr;
    // STRUM PATH
    bool StrumInput = (stats->strumState != StrumState::Default)
        && action == Action::PRESS
        && (channel == InputChannel::STRUM_UP || channel == InputChannel::STRUM_DOWN);
    if (StrumInput) {
        // miss should be managed by current frame
        // overhit is managed here
        if (Timers["SAH"].CanBeUsedUp(stats->InputTime)) {
            Timers["SAH"].ResetTimer();
            return CheckNextInput;
        }
        if (EarlyStrike(CurrentNote.StartSeconds, stats->InputTime, stats->InputOffset)) {

            chart->overdrive.UpdateEventViaNote(false, CurrentNote.StartTicks);
            return OverhitNote;
        }
        // if frets match, continue and try to hit
        if (InHitwindow(CurrentNote.StartSeconds, stats->InputTime, stats->InputOffset)) {
            if (!MaskMatch(CurrentNote.Lane, stats->HeldFretsArrayToMask())) {
                Timers["FAS"].ActivateTimer(stats->InputTime);
                return CheckNextInput;
            }
        }
    }
    // if FAS is active, or if there was a strum
    // really couldve just put it up there LMFAO
    bool strum = Timers["FAS"].CanBeUsedUp(stats->InputTime) || StrumInput;

    if (MaskMatch(CurrentNote.Lane, stats->HeldFretsArrayToMask())
        && InHitwindow(CurrentNote.StartSeconds, stats->InputTime, stats->InputOffset)
        && (HittableAsHopo(CurrentNote.NoteType, stats->Combo)
            || HittableAsTap(CurrentNote.NoteType) || strum)) {
        stats->HitNote(std::popcount(CurrentNote.Lane));
        chart->overdrive.UpdateEventViaNote(true, CurrentNote.StartTicks);
        if (CurrentNote.NoteType == 1 && !StrumInput && !Timers["FAS"].CanBeUsedUp(stats->InputTime)) {
            Timers["SAH"].ActivateTimer(stats->InputTime);
        }
        Timers["FAS"].ResetTimer();

        chart->at(0).erase(curNoteItr);
        return HitState::HitNote;
    }
    return CheckNextInput;
}
void Encore::RhythmEngine::GuitarEngine::HitNote() {
    // stats->HitNote(std::popcount(chart->at(0).front().Lane));
    // todo: you know the drill, maybe set track state too with this info?
    // stats->FretAfterStrum = false;
    // chart->at(0).erase(chart->at(0).begin());
}
void Encore::RhythmEngine::GuitarEngine::Overhit() {
    stats->Overhit();
}

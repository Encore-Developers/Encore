//
// Created by maria on 01/06/2025.
//

#include "GuitarEngine.h"

#include "timingvalues.h"

#include <bit>

bool EarlyStrike(double noteStartTime, double inputTime, double inputOffset) {
    if (noteStartTime - goodFrontend > inputTime - inputOffset) {
        return true;
    }
    return false;
}
bool InHitwindow(double noteStartTime, double inputTime, double inputOffset) {
    if ((noteStartTime - goodFrontend < inputTime - inputOffset)
        && (noteStartTime + goodBackend > inputTime - inputOffset)) {
        return true;
    }
    return false;
}
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
        stats->OverdriveActivationTime = 0.0; // todo: set to current input time
        return true;
    }
    return false;
}
void Encore::RhythmEngine::GuitarEngine::SetStatsInputState(
    InputChannel channel, Action action
) {
    stats->InputTime = 0.0; // todo: REPLACE WITH ACTUAL SONG TIME (IN SECONDS)
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
            stats->StrumState = StrumState::UpStrum;
            break;
        }
        case InputChannel::STRUM_DOWN: {
            stats->StrumState = StrumState::DownStrum;
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
            stats->StrumState = StrumState::Default;
            break;
        }
        default:
            break;
        }
    }
}
int Encore::RhythmEngine::GuitarEngine::RunHitStateCheck(Action action) {
    EncNote &CurrentNote = chart->at(0).front();

    // STRUM PATH
    if ((stats->StrumState != StrumState::Default) && action == Action::PRESS) {
        // miss should be managed by current frame
        // overhit is managed here
        if (EarlyStrike(CurrentNote.StartSeconds, stats->InputTime, stats->InputOffset)) {
            return HitState::OverhitNote;
        }
        // if frets match, continue and try to hit
        if (InHitwindow(CurrentNote.StartSeconds, stats->InputTime, stats->InputOffset)) {
            if (!MaskMatch(CurrentNote.Lane, stats->HeldFretsArrayToMask())) {
                stats->FretAfterStrum = true;
                stats->FretAfterStrumTime = stats->InputTime;
                return HitState::CheckNextInput;
            }
        }
    }
    // if FAS is active, or if there was a strum
    // really couldve just put it up there LMFAO
    bool strum = HittableAsStrum(CurrentNote.NoteType, stats->FretAfterStrum, stats->InputTime, stats->FretAfterStrumTime)
        || (stats->StrumState != StrumState::Default && action == Action::PRESS);

    if (MaskMatch(CurrentNote.Lane, stats->HeldFretsArrayToMask())
        && InHitwindow(CurrentNote.StartSeconds, stats->InputTime, stats->InputOffset)
        && (HittableAsHopo(CurrentNote.NoteType, stats->Combo) || HittableAsTap(CurrentNote.NoteType)
            || strum)) {
        return HitState::HitNote;
    }
    return HitState::CheckNextInput;
}
void Encore::RhythmEngine::GuitarEngine::HitNote() {
    stats->HitNote(std::popcount(chart->at(0).front().Lane));
    // todo: you know the drill, maybe set track state too with this info?
    stats->FretAfterStrum = false;
    chart->at(0).pop();
}
void Encore::RhythmEngine::GuitarEngine::Overhit() {
    stats->Overhit();
}

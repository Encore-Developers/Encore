//
// Created by maria on 01/06/2025.
//

#include "GuitarEngine.h"

#include "timingvalues.h"
#include "gameplay/enctime.h"
#include "song/scoring.h"

#include <bit>
#include <filesystem>

#include "song/audio.h"

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
    if (Combo > 0 && NoteType == 1 && GhostCount < 4)
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

bool Encore::RhythmEngine::GuitarEngine::ActivateOverdrive(ControllerEvent &event
) {
    if (event.channel == InputChannel::OVERDRIVE && event.action == Action::PRESS) {
        int InstrumentNum =
            stats->Type == Guitar ? inst - 5 : inst;
        stats->overdrive.Activate(stats->InputTime);
        TheAudioManager.StartEffect(TheAudioManager.GetAudioStreamByInstrument(inst));
        EncoreLog(LOG_DEBUG, TextFormat("Instrument: %i", inst));
        return true;
    }
    return false;
}

void Encore::RhythmEngine::GuitarEngine::CheckMissedNotes(double CurrentTime) {
    BaseEngine::CheckMissedNotes(0, CurrentTime);
}


void Encore::RhythmEngine::GuitarEngine::UpdateOnFrame(double CurrentTime) {
    this->LastUpdateTime = CurrentTime;
    if (stats->Bot) {
        if (chart->CurrentNoteIterators.at(0) == chart->Lanes.at(0).end())
            return;
        EncNote *CurrentNote = &*chart->CurrentNoteIterators.at(0);
        if (CurrentNote->StartSeconds <= CurrentTime) {
            HitNote(true);
        }
    }
    auto heldNote = chart->HeldNotePointers.at(0);
    if (heldNote != nullptr
        && heldNote->StartSeconds + heldNote->LengthSeconds >= CurrentTime
        && heldNote->StartSeconds <= CurrentTime) {
        double PointsPerTick = double(SUSTAIN_POINTS_PER_BEAT) / 480.0;
        stats->Score += (TheSongTime.CurrentTick - TheSongTime.LastTick) * ((PointsPerTick
            * stats->multiplier()) * std::popcount(chart->HeldNotePointers.at(0)->Lane));
    }
    if (heldNote && heldNote->StartSeconds + heldNote->LengthSeconds < CurrentTime) {
        chart->DropSustain(0);
    }
    this->CheckMissedNotes(CurrentTime);
    stats->overdrive.Add(CurrentTime, chart);
    bool odWasActive = stats->overdrive.Active;
    stats->overdrive.Update(CurrentTime);
    if (odWasActive == true && odWasActive != stats->overdrive.Active) {
        int InstrumentNum =
            stats->Type == Guitar ? inst - 5 : inst;
        TheAudioManager.StopEffect(TheAudioManager.GetAudioStreamByInstrument(inst));
        EncoreLog(LOG_DEBUG, TextFormat("Instrument: %i", inst));
    }
    // there is ONLY lane 0 for guitar
}

void Encore::RhythmEngine::GuitarEngine::SetStatsInputState(
ControllerEvent &event
) {
    stats->InputTime = LastUpdateTime; // todo: REPLACE WITH ACTUAL SONG
    // TIME
    // (IN SECONDS)
    if (event.channel == InputChannel::WHAMMY) {

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
                if (note->Lane & PlasticFrets[ICInt(event.channel)] &&
                    note->StartTicks+note->LengthTicks >= TheSongTime.CurrentTick + 240) {
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

/*
bool Encore::RhythmEngine::GuitarEngine::CanNoteBeHit() {
    return true;
}*/

int Encore::RhythmEngine::GuitarEngine::RunHitStateCheck(ControllerEvent &event
) {
    // GetCurrentNote(0);
    if (chart->CurrentNoteIterators.at(0) == chart->Lanes.at(0).end())
        return CheckNextInput;
    EncNote &CurrentNote = *chart->CurrentNoteIterators.at(0);

    if (chart->IsHeldNotePresent(0) &&
        event.action == Action::PRESS &&
        !MaskMatch(chart->HeldNotePointers.at(0)->Lane, stats->HeldFretsArrayToMask()) &&
        !(chart->HeldNotePointers.at(0)->StartSeconds + chart->HeldNotePointers.at(0)->
            LengthSeconds > CurrentNote.StartSeconds
        ) &&
        (!(chart->HeldNotePointers.at(0)->StartTicks + chart->HeldNotePointers.at(0)->LengthTicks < TheSongTime.CurrentTick + 240))
        ) {
        chart->DropSustain(0);
    }
    // if an upper note is pressed when a sustain is active, unless the sustain goes on longer than the current note

    // STRUM PATH
    bool StrumInput = (stats->strumState != StrumState::Default)
        && event.action == Action::PRESS
        && (event.channel == InputChannel::STRUM_UP || event.channel == InputChannel::STRUM_DOWN);
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

    // GHOSTING
    if (event.action == Action::PRESS && event.channel <= InputChannel::LANE_5 && event.channel !=
        InputChannel::INVALID) {
        if (stats->HeldFretsArrayToMask() > CurrentNote.Lane) {
            GhostCount++;
        }
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
        if (chart->IsHeldNotePresent(0)) {
            chart->HeldNotePointers.at(0)->Lane |= chart->CurrentNoteIterators.at(0)->
                                                          Lane;
        } else {
            chart->SetCurrentNoteAsHeldNote(0);
        }
    }
    BaseEngine::HitNote(0);
}

void Encore::RhythmEngine::GuitarEngine::Overhit() {
    if (Timers["SAH"].CanBeUsedUp(stats->InputTime)) {
        Timers["SAH"].ResetTimer();
        TraceLog(LOG_DEBUG, "SAH Disabled");
    }
    chart->DropSustain(0);
    BaseEngine::Overhit(0);
}

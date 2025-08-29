//
// Created by maria on 01/06/2025.
//

#include "BaseEngine.h"

#include "gameplay/enctime.h"

#include <bit>
#include <memory>

bool Encore::RhythmEngine::BaseEngine::EarlyStrike(
    double noteStartTime
) {
    if (noteStartTime - goodFrontend > stats->InputTime - stats->InputOffset) {
        return true;
    }
    return false;
}
bool Encore::RhythmEngine::BaseEngine::InHitwindow(
    double noteStartTime
) {
    if ((noteStartTime - goodFrontend < stats->InputTime - stats->InputOffset)
        && (noteStartTime + goodBackend > stats->InputTime - stats->InputOffset)) {
        return true;
        }
    return false;
}
void Encore::RhythmEngine::BaseEngine::ProcessInput(InputChannel channel, Action action) {
    if (channel == InputChannel::INVALID)
        return;

    if (PauseGame(channel, action)) {
        return;
    }
    if (PlayerIsPaused())
        return;
    if (ActivateOverdrive(channel, action)) {
        return;
    }
    SetStatsInputState(channel, action);
    // look its really stupid but like. cmon
    if ((channel == InputChannel::STRUM_DOWN || channel == InputChannel::STRUM_UP)
        && action == Action::RELEASE) {
        return;
    }
    RunHitStateCheck(channel, action);
}
// i dont think this is needed because it was technically already taken care of by
// BaseChart
/*bool Encore::RhythmEngine::BaseEngine::GetCurrentNote(int lane) {
    if (chart->at(lane).empty())
        return false;

    curNoteItrs[lane] = chart->at(lane).begin();
    while (curNoteItrs[lane]->StartSeconds + goodBackend
           < TheSongTime.GetElapsedTime() - stats->InputOffset) {
        if (curNoteItrs[lane] + 1 == chart->at(lane).end()) {
            return false;
        }
        ++curNoteItrs[lane];
    }
    // EncNote& lol = *curNoteItrs[lane];
    CurrentNoteItr = std::make_shared<NoteVector::iterator>(curNoteItrs[lane]);
    return true;
}*/

bool Encore::RhythmEngine::BaseEngine::PauseGame(InputChannel channel, Action action) {
    if (channel == InputChannel::PAUSE && action == Action::PRESS) {
        TogglePause();
        return true;
    }
    return false;
}
void Encore::RhythmEngine::BaseEngine::CheckMissedNotes(int Lane, double SongTime) {
    if (chart->CurrentNoteIterators.at(Lane) == chart->Lanes.at(Lane).end())
        return;
    EncNote &CurrentNote = *chart->CurrentNoteIterators.at(Lane);
    if (CurrentNote.StartSeconds + goodBackend < SongTime - stats->InputOffset
        && &CurrentNote != chart->HeldNotePointers.at(Lane)) {
        MissNote(Lane);
        Encore::EncoreLog(
            LOG_DEBUG, TextFormat("Missed note %01i:%01i", Lane, std::distance(chart->Lanes.at(Lane).begin(), chart->CurrentNoteIterators.at(Lane)))
        );
        }
}
void Encore::RhythmEngine::BaseEngine::HitNote(int lane) {
    int chordSize = std::popcount(chart->CurrentNoteIterators.at(lane)->Lane);
    int startTick = chart->CurrentNoteIterators.at(lane)->StartTicks;
    Encore::EncoreLog(
            LOG_DEBUG, TextFormat("Hit note %01i:%01i", lane, std::distance(chart->Lanes.at(lane).begin(), chart->CurrentNoteIterators.at(lane)))
        );
    if (!chart->UpdateCurrentNote(lane))
        return;
    stats->HitNote(chordSize);
    chart->overdrive.UpdateEventViaNote(true, startTick);
    chart->solos.UpdateEventViaNote(true, startTick);
}

void Encore::RhythmEngine::BaseEngine::MissNote(int lane) {
    stats->MissNote();
    chart->overdrive.UpdateEventViaNote(
        false, chart->CurrentNoteIterators.at(lane)->StartTicks
    );
    chart->UpdateCurrentNote(lane);
}

void Encore::RhythmEngine::BaseEngine::Overhit(int lane) {
    EncoreLog(LOG_DEBUG, "Overhit note");
    stats->Overhit();
    chart->overdrive.UpdateEventViaNote(
        false, chart->CurrentNoteIterators.at(lane)->StartTicks
    );
}

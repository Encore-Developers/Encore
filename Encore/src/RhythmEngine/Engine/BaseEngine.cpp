//
// Created by maria on 01/06/2025.
//

#include "BaseEngine.h"

#include "gameplay/enctime.h"

#include <bit>
#include <memory>

void Encore::RhythmEngine::BaseEngine::ProcessInput(InputChannel channel, Action action) {
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

void Encore::RhythmEngine::BaseEngine::HitNote(int lane) {
    stats->HitNote(std::popcount(chart->CurrentNoteIterators.at(0)->Lane));
    chart->overdrive.UpdateEventViaNote(
        true, chart->CurrentNoteIterators.at(0)->StartTicks
    );
    chart->solos.UpdateEventViaNote(true, chart->CurrentNoteIterators.at(0)->StartTicks);
    chart->UpdateCurrentNote(lane);
}

void Encore::RhythmEngine::BaseEngine::MissNote(int lane) {
    stats->MissNote();
    chart->overdrive.UpdateEventViaNote(
        false, chart->CurrentNoteIterators.at(0)->StartTicks
    );
    chart->UpdateCurrentNote(lane);
}

void Encore::RhythmEngine::BaseEngine::Overhit() {
    stats->Overhit();
    chart->overdrive.UpdateEventViaNote(
        false, chart->CurrentNoteIterators.at(0)->StartTicks
    );
}

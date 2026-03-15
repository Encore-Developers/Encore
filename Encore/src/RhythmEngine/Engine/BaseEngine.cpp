//
// Created by maria on 01/06/2025.
//

#include "BaseEngine.h"

#include "gameplay/enctime.h"
#include "song/song.h"
#include "users/player.h"

#include <bit>
#include <memory>

#include "song/audio.h"

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

bool Encore::RhythmEngine::BaseEngine::PerfectHit(
    double noteStartTime
) {
    if ((noteStartTime - perfectFrontend < stats->InputTime - stats->InputOffset)
        && (noteStartTime + perfectBackend > stats->InputTime - stats->InputOffset)) {
        return true;
        }
    return false;
}
void Encore::RhythmEngine::BaseEngine::ProcessInput(ControllerEvent &event) {
    if (event.channel == InputChannel::INVALID)
        return;

    if (PauseGame(event)) {
        return;
    }
    if (PlayerIsPaused())
        return;
    if (ActivateOverdrive(event)) {
        return;
    }
    SetStatsInputState(event);
    // look its really stupid but like. cmon
    if ((event.channel == InputChannel::STRUM_DOWN || event.channel == InputChannel::STRUM_UP)
        && event.action == Action::RELEASE) {
        return;
    }
    RunHitStateCheck(event);
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

bool Encore::RhythmEngine::BaseEngine::PauseGame(ControllerEvent &event) {
    if (event.channel == InputChannel::PAUSE && event.action == Action::PRESS) {
        TogglePause();
        // TODO: make this work depending on player count and probably not in here
        if (stats->Paused)
            TheAudioManager.pauseStreams();
        else
            TheAudioManager.unpauseStreams();
        return true;
    }
    return false;
}

std::pair<int, int> Encore::RhythmEngine::BaseEngine::GetNotePoolSize(int lane) {
    int NotePoolStart =
            std::distance(chart->at(lane).begin(), chart->CurrentNoteIterators.at(lane));
    int NotePoolEnd = NOTE_POOL_SIZE
        + std::distance(chart->at(lane).begin(), chart->CurrentNoteIterators.at(lane));
    NotePoolEnd =
                    chart->at(lane).size() > NotePoolEnd ? NotePoolEnd : chart->at(lane).size();
    int NotePoolSize = NotePoolEnd - NotePoolStart;

    return std::pair<int, int> {NotePoolStart, NotePoolEnd};
}
void Encore::RhythmEngine::BaseEngine::CheckMissedNotes(int Lane, double SongTime) {
    if (chart->CurrentNoteIterators.at(Lane) == chart->Lanes.at(Lane).end())
        return;
    EncNote &CurrentNote = *chart->CurrentNoteIterators.at(Lane);
    if (CurrentNote.StartSeconds + goodBackend < SongTime - stats->InputOffset
        && &CurrentNote != chart->HeldNotePointers.at(Lane)) {
        GhostCount = 0;
        MissNote(Lane);
        Encore::EncoreLog(
            LOG_DEBUG, TextFormat("Missed note %01i:%01i", Lane, std::distance(chart->Lanes.at(Lane).begin(), chart->CurrentNoteIterators.at(Lane)))
        );
        }
}
void Encore::RhythmEngine::BaseEngine::HitNote(int lane) {
    int chordSize = std::popcount(chart->CurrentNoteIterators.at(lane)->Lane);
    if (chordSize == 0) chordSize = 1;

    int startTick = chart->CurrentNoteIterators.at(lane)->StartTicks;
    double startTime = chart->CurrentNoteIterators.at(lane)->StartSeconds;
    Encore::EncoreLog(
            LOG_DEBUG, TextFormat("Hit note %01i:%01i", lane, std::distance(chart->Lanes.at(lane).begin(), chart->CurrentNoteIterators.at(lane)))
        );

    NoteHitEvent event = NoteHitEvent(&*chart->CurrentNoteIterators.at(lane));
    if (PerfectHit(startTime)) {
        stats->LastPerfectTime = stats->InputTime;
        event.perfect = true;
    }
    FireEvent(&event);
    if (!chart->UpdateCurrentNote(lane))
        return;

    stats->LastHitAccuracy = stats->InputTime - startTime;
    stats->HitNote(chordSize, PerfectHit(startTime));
    chart->overdrive.UpdateEventViaNote(true, startTick);
    chart->solos.UpdateEventViaNote(true, startTick);
}

void Encore::RhythmEngine::BaseEngine::MissNote(int lane) {
    stats->MissNote();
    chart->overdrive.MissCurrentEvent(chart->CurrentNoteIterators.at(lane)->StartTicks);
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
void Encore::RhythmEngine::BaseEngine::UpdateStats(int instrument, int difficulty) {

    inst = instrument >= PlasticDrums ? instrument - 5 : instrument;
    stats->StarThresholdValue = stats->Score / chart->BaseScore;
    for (int i = 0; i < 6 ; i++) {
        if (stats->StarThresholdValue < STAR_THRESHOLDS[inst][i]) {
            stats->Stars = i;
            return;
        }
    }
    stats->Stars = 5;

}

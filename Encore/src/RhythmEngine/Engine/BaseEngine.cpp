//
// Created by maria on 01/06/2025.
//

#include "BaseEngine.h"

#include "gameplay/enctime.h"
#include "song/song.h"
#include "users/player.h"

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

bool Encore::RhythmEngine::BaseEngine::PerfectHit(
    double noteStartTime
) {
    if ((noteStartTime - perfectFrontend < stats->InputTime - stats->InputOffset)
        && (noteStartTime + perfectBackend > stats->InputTime - stats->InputOffset)) {
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
    if (chordSize == 0) chordSize = 1;

    int startTick = chart->CurrentNoteIterators.at(lane)->StartTicks;
    double startTime = chart->CurrentNoteIterators.at(lane)->StartSeconds;
    Encore::EncoreLog(
            LOG_DEBUG, TextFormat("Hit note %01i:%01i", lane, std::distance(chart->Lanes.at(lane).begin(), chart->CurrentNoteIterators.at(lane)))
        );
    if (!chart->UpdateCurrentNote(lane))
        return;
    if (PerfectHit(startTime)) stats->LastPerfectTime = stats->InputTime;
    stats->HitNote(chordSize, PerfectHit(startTime));
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
void Encore::RhythmEngine::BaseEngine::UpdateStats(int instrument, int difficulty) {
    int inst = instrument >= PlasticDrums ? instrument - 5 : instrument;
    stats->StarThresholdValue = stats->Score / chart->BaseScore;
    if (stats->StarThresholdValue < STAR_THRESHOLDS[inst][0]) {
        stats->Stars = 0;
        return;
    } else if (stats->StarThresholdValue < STAR_THRESHOLDS[inst][1]) {
        stats->Stars = 1;
        return;
    } else if (stats->StarThresholdValue < STAR_THRESHOLDS[inst][2]) {
        stats->Stars = 2;
        return;
    } else if (stats->StarThresholdValue < STAR_THRESHOLDS[inst][3]) {
        stats->Stars = 3;
        return;
    } else if (stats->StarThresholdValue < STAR_THRESHOLDS[inst][4]) {
        stats->Stars = 4;
        return;
    } else if (stats->StarThresholdValue < STAR_THRESHOLDS[inst][5]) {
        stats->Stars = 5;
        return;
    } else if (stats->StarThresholdValue >= STAR_THRESHOLDS[inst][5]) {
        stats->Stars = 5;
        return;
    } else {
        stats->Stars = 5;
        return;
    }

    //if (difficulty != 3 && stats->Stars > 4)
    //    stats->Stars = 4;

}

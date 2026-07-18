//
// Created by maria on 01/06/2025.
//

#include "BaseEngine.h"

#include "gameplay/enctime.h"
#include "settings/settings.h"
#include "song/song.h"
#include "users/player.h"

#include <bit>

#include "song/audio.h"

bool Encore::RhythmEngine::BaseEngine::EarlyStrike(
    double noteStartTime
) const {
    if (noteStartTime - goodFrontend > stats->InputTime) {
        return true;
    }
    return false;
}
bool Encore::RhythmEngine::BaseEngine::InHitwindow(
    double noteStartTime
) const {
    if ((noteStartTime - goodFrontend < stats->InputTime)
        && (noteStartTime + goodBackend > stats->InputTime)) {
        return true;
        }
    return false;
}

bool Encore::RhythmEngine::BaseEngine::PerfectHit(
    double noteStartTime
) const {
    if ((noteStartTime - perfectFrontend < stats->InputTime)
        && (noteStartTime + perfectBackend > stats->InputTime)) {
        return true;
        }
    return false;
}

void Encore::RhythmEngine::BaseEngine::ProcessInput(ControllerEvent &event) {
    if (event.channel == InputChannel::INVALID)
        return;

    // if (PauseGame(event)) {
    //     return;
    // }
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
    if (IsWithinPracticeSection(event.timestamp) || !practice)
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

bool Encore::RhythmEngine::BaseEngine::PauseGame(const ControllerEvent &event) {
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

std::pair<int, int> Encore::RhythmEngine::BaseEngine::GetNotePoolSize(int lane) const {
    size_t NotePoolStart =
            std::distance(chart->at(lane).begin(), chart->CurrentNoteIterators.at(lane));
    size_t NotePoolEnd = NOTE_POOL_SIZE
        + std::distance(chart->at(lane).begin(), chart->CurrentNoteIterators.at(lane));
    NotePoolEnd =
                    chart->at(lane).size() > NotePoolEnd ? NotePoolEnd : chart->at(lane).size();

    return std::pair<int, int> {NotePoolStart, NotePoolEnd};
}

bool Encore::RhythmEngine::BaseEngine::IsWithinPracticeSection(double time) const {
    return practice && time < pStopTime && time >= pStartTime;
}

void Encore::RhythmEngine::BaseEngine::BaseUpdateOnFrame(double CurrentTime) {
    stats->overdrive.ticks.UpdateOverdriveTick();
    if (stats->overdrive.Add(CurrentTime, chart) && stats->overdrive.Fill >= 0.5 && !stats->overdrive.Active) {
        TrackNotificationEvent odReady(TheSongTime.GetElapsedTime(), TrackNotificationEvent::OVERDRIVE_READY);
        FireEvent(&odReady);
    }

    chart->solos.CheckEvents(CurrentTime);
    bool odWasActive = stats->overdrive.Active;
    stats->overdrive.Update(CurrentTime);
    if (odWasActive == true && odWasActive != stats->overdrive.Active) {
        // int InstrumentNum = stats->Type == Guitar ? inst - 5 : inst;
        // EncoreLog(LOG_DEBUG, TextFormat("Disabling effect on instrument: %i", inst));
        // TheAudioManager.StopEffect(TheAudioManager.GetAudioStreamByInstrument(inst));
    }
}

void Encore::RhythmEngine::BaseEngine::CheckMissedNotes(size_t Lane, double SongTime) {
    if (chart->CurrentNoteIterators.at(Lane) == chart->Lanes.at(Lane).end())
        return;
    EncNote &CurrentNote = *chart->CurrentNoteIterators.at(Lane);
    if (CurrentNote.StartSeconds + goodBackend < SongTime
        && &CurrentNote != chart->HeldNotePointers.at(Lane)) {
        GhostCount = 0;
        MissNote(Lane);
        Log::Debug("Player {} missed note {} : {}", player->Name, Lane, std::distance(chart->Lanes.at(Lane).begin(), chart->CurrentNoteIterators.at(Lane)));
    }
}

void Encore::RhythmEngine::BaseEngine::HitNote(const size_t lane) {
    int chordSize = std::popcount(chart->CurrentNoteIterators.at(lane)->Lane);
    if (chordSize == 0) chordSize = 1;
    int startTick = chart->CurrentNoteIterators.at(lane)->StartTicks;
    double startTime = chart->CurrentNoteIterators.at(lane)->StartSeconds;
    const double offset = (stats->InputTime) - startTime;

    stats->accuracies.emplace_back(startTime, offset, false);

    // you REALLY dont need to be firing every few seconds
    // Encore::EncoreLog(
    //        LOG_DEBUG, TextFormat("Hit note %01i:%01i", lane, std::distance(chart->Lanes.at(lane).begin(), chart->CurrentNoteIterators.at(lane)))
    //    );
    chart->UpdateSections(startTick);
    if (!chart->sections.empty())
        chart->sections.at(chart->CurrentSection).hit++;
    auto event = NoteHitEvent(&*chart->CurrentNoteIterators.at(lane));
    if (PerfectHit(startTime)) {
        stats->LastPerfectTime = stats->InputTime;
        if (!chart->sections.empty())
            chart->sections.at(chart->CurrentSection).perfects++;
        event.judgement = PERFECT;
    }

    if (!chart->sections.empty())
        chart->sections.at(chart->CurrentSection).notes++;
    event.offset = offset;
    stats->TotalOffset += event.offset;
    FireEvent(&event);
    if (!chart->UpdateCurrentNote(lane))
        return;
    stats->LastHitAccuracy = (stats->InputTime) - startTime;
    stats->HitNote(chordSize, event.judgement);

    if (PerfectHit(startTime)) {
        stats->Accuracy += 1;
        Log::Debug("Accuracy: {}", 1);
    } else {
        double acc = (goodFrontend - std::abs(offset)) / goodFrontend;
        if (acc < 0) acc = 0;
        stats->Accuracy += acc;

        Log::Debug("Accuracy: {}", acc);
    }

    if (stats->Combo == 25 && stats->Overhits == 0 && stats->Misses == 0) {
        TrackNotificationEvent event2 {startTime, TrackNotificationEvent::HOTSTART};
        FireEvent(&event2);
    }

    int comboNotif = stats->Combo > 200 ? stats->Combo % 100 : stats->Combo % 50;
    if (stats->Combo == 50 && (inst == PartBass || inst == PlasticBass)) {
        TrackNotificationEvent event2 {startTime, TrackNotificationEvent::BASSGROOVE};
        FireEvent(&event2);
    } else if (comboNotif == 0) {
        TrackNotificationEvent event2 {startTime, TrackNotificationEvent::COMBO, stats->Combo};
        FireEvent(&event2);;
    }
    int multiplierIncrease = stats->Combo % 10;
    // in the unplanned/planned career mode, id like this to be adjustable
    if (multiplierIncrease == 0 && stats->Combo <= (stats->SixMultiplier ? 50 : 30)) {
        MultFlashEvent e {false};
        FireEvent(&e);
    }
    chart->overdrive.UpdateEventViaNote(true, startTick);
    chart->solos.UpdateEventViaNote(true, startTick);
}

void Encore::RhythmEngine::BaseEngine::MissNote(const size_t lane) {
    if (stats->Combo >= 10) {
        MultFlashEvent e {true};
        FireEvent(&e);
    }
    stats->accuracies.emplace_back(chart->CurrentNoteIterators.at(lane)->StartSeconds, 0, true);
    if (!chart->sections.empty())
        chart->sections.at(chart->CurrentSection).notes++;
    stats->MissNote();
    chart->overdrive.MissCurrentEvent(chart->CurrentNoteIterators.at(lane)->StartTicks);
    chart->overdrive.UpdateEventViaNote(
        false, chart->CurrentNoteIterators.at(lane)->StartTicks
    );
    chart->MissedNotePointers.push_back(&*chart->CurrentNoteIterators.at(lane));

    chart->UpdateCurrentNote(lane);
}

void Encore::RhythmEngine::BaseEngine::Overhit(const size_t lane) {

    double earliestNoteTime = 0.0;
    for (int i = 0; i < chart->Lanes.size(); i++) {
        if (!chart->at(i).empty()) {
            if (earliestNoteTime == 0.0)
                earliestNoteTime = chart->at(i).front().StartSeconds;
            if (earliestNoteTime > chart->at(i).front().StartSeconds) {
                earliestNoteTime = chart->at(i).front().StartSeconds;
            }
        }
    }
    if (stats->InputTime < earliestNoteTime)
        return;

    chart->UpdateSections(TheSongTime.GetCurrentTick());
    // should be safe, right?
    if (!chart->sections.empty())
        chart->sections.at(chart->CurrentSection).overhits++;
    TextFormat("Player %s overhit", player->Name.c_str());
    if (stats->Combo > 0) {
        MultFlashEvent e {true};
        FireEvent(&e);
    }
    FireEventTemp(OverhitEvent(lane));
    stats->Overhit();
    chart->overdrive.UpdateEventViaNote(
        false, chart->CurrentNoteIterators.at(lane)->StartTicks
    );
}
void Encore::RhythmEngine::BaseEngine::UpdateStats(const int instrument, int difficulty) {

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
void Encore::RhythmEngine::BaseEngine::UpdateCalibration(const double playerInputOffset) const {
    // stats->InputOffset is in seconds, our settings values are in ms
    stats->InputOffset = (playerInputOffset + TheGameSettings.VideoOffset)/1000.0; // TODO: this isn't the right way to calculate this
}

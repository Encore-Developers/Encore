//
// Created by maria on 01/06/2025.
//

#include "BaseEngine.h"

#include "RhythmEngine/timetypes.h"
#include "gameplay/enctime.h"
#include "settings/settings.h"
#include "song/song.h"
#include "users/player.h"

#include <bit>

#include "song/audio.h"

double Encore::RhythmEngine::Sustain::GetValueAtTime(TimeBeats time) const {
    if (end < start) {
        return 0;
    }
    double progress = 0;
    if (time < start) {
        return 0;
    }
    if (time > end) {
        progress = 1;
    } else {
        progress = (time - start) / (end - start);
    }
    return progress * multiplier * valuePerBeat * (end - start);
}
void Encore::RhythmEngine::BaseEngine::CalculateWeightedScore() {
    double weighted = 0;
    double count = 1;
    /*
     * Going lane-by-lane introduces some issues with the ordering on
     * pad charts (important for the combo to be correct) so we'll do this nastiness
     */
    std::vector<NoteEvent*> notes;
    size_t reserve = 0;

    for (auto &lane : chart->Lanes) {
        reserve += lane.size();
    }
    notes.reserve(reserve);
    for (auto &lane : chart->Lanes) {
        for (auto &note : lane) {
            notes.push_back(&note);
        }
    }
    std::ranges::sort(notes, [](const NoteEvent* a, const NoteEvent* b){return a->start < b->start;});

    for (auto note : notes) {
        double Multiplier = (int(count) / stats->ep.mult.notesPerMult) + 1;
        if (Multiplier > stats->ep.mult.max) {
            Multiplier = stats->ep.mult.max;
        }
        int chordSize = std::popcount(note->lane);
        if (chordSize == 0)
            chordSize = 1;
        weighted += chordSize * Multiplier;
        if (note->end != 0) {
            auto length = note->end.tick - note->start.tick;
            double beats = (double(length) / TheSongTime.songPPQN);
            // TODO: should this be multiplied by 4? (quarter note)
            weighted += (beats * Multiplier);
        }
        count++;
    }
    stats->PointsPerNote = BASE_SCORE / weighted;
    stats->weightedCount = weighted;
}
bool Encore::RhythmEngine::BaseEngine::EarlyStrike(double noteStartTime) const {
    if (noteStartTime - goodFrontend > EngineTime) {
        return true;
    }
    return false;
}
bool Encore::RhythmEngine::BaseEngine::InHitwindow(double noteStartTime) const {
    if ((noteStartTime - goodFrontend < EngineTime)
        && (noteStartTime + goodBackend > EngineTime)) {
        return true;
    }
    return false;
}

bool Encore::RhythmEngine::BaseEngine::PerfectHit(double noteStartTime) const {
    if ((noteStartTime - perfectFrontend < EngineTime)
        && (noteStartTime + perfectBackend > EngineTime)) {
        return true;
    }
    return false;
}

void Encore::RhythmEngine::BaseEngine::ProcessInput(ControllerEvent &event) {
    if (event.channel == InputChannel::INVALID)
        return;
    if (event.channel == InputChannel::WHAMMY) {
        UpdateWhammy(EngineTime, event.axis / (float)255);
    }
    if (!OverdriveActive && OverdriveValue > 0 && event.channel == InputChannel::OVERDRIVE && event.action == Action::PRESS) {
        ActivateOverdrive(EngineTime);
    }
    // look its really stupid but like. cmon
    if ((event.channel == InputChannel::STRUM_DOWN
         || event.channel == InputChannel::STRUM_UP)
        && event.action == Action::RELEASE) {
        return;
    }
    RunHitStateCheck(event);
}

std::pair<int, int> Encore::RhythmEngine::BaseEngine::GetNotePoolSize(int lane) const {
    size_t NotePoolStart =
        std::distance(chart->at(lane).begin(), chart->CurrentNoteIterators.at(lane));
    size_t NotePoolEnd = NOTE_POOL_SIZE
        + std::distance(chart->at(lane).begin(), chart->CurrentNoteIterators.at(lane));
    NotePoolEnd =
        chart->at(lane).size() > NotePoolEnd ? NotePoolEnd : chart->at(lane).size();

    return std::pair<int, int> { NotePoolStart, NotePoolEnd };
}
void Encore::RhythmEngine::BaseEngine::Advance(double CurrentTime) {
    if (CurrentTime < EngineTime) {
        Log::Warn("Engine backstepping by {}ms", (CurrentTime-EngineTime) * 1000);
    }
    if (CurrentTime == EngineTime) {
        return;
    }
    EngineTime = CurrentTime;
    deferMultiplierUpdates = true;
    UpdateOverdrive(CurrentTime);
    for (int i = 0; i < chart->Lanes.size(); i++) {
        CheckMissedNotes(i, EngineTime);
    }
    UpdateSustains();
    chart->solos.CheckEventsSeconds(CurrentTime);
    deferMultiplierUpdates = false;

    // This is the ensure sustains calculate the correct values over large delta times
    // TODO: this might cause issues with pad chord sustains
    if (defferredMultiplierUpdates.size() > 1) {
        std::ranges::sort(defferredMultiplierUpdates.begin(), defferredMultiplierUpdates.end(), [](MultUpdate& a, MultUpdate& b){return a.time < b.time;});
    }
    for (auto& upd : defferredMultiplierUpdates) {
        SegmentSustains(upd.time, upd.mult);
    }
    defferredMultiplierUpdates.clear();
}
void Encore::RhythmEngine::BaseEngine::CheckMissedNotes(size_t Lane, TimeSeconds time) {
    if (chart->CurrentNoteIterators.at(Lane) == chart->Lanes.at(Lane).end())
        return;

    bool MissedNote = false;
    do {
        auto CurrentNote = chart->CurrentNoteIterators.at(Lane);
        MissedNote = false;
        TimeSeconds MissTime = 0.0;
        if (chart->CurrentNoteIterators.at(Lane) + 1 != chart->Lanes.at(Lane).end()) {
            auto NextNote = chart->CurrentNoteIterators.at(Lane);
            NextNote += 1;
            double noteMiddlePoint = (NextNote->start - CurrentNote->start).sec / dynamicHitwindowRatio;
            if (noteMiddlePoint < minimumHitwindowSize) noteMiddlePoint = minimumHitwindowSize;
            if (CurrentNote->start + noteMiddlePoint < time) {
                MissedNote = true;
                MissTime = CurrentNote->start + noteMiddlePoint;
            }
        }
        if (CurrentNote->start.sec + goodBackend < time) {
            MissedNote = true;
            MissTime = CurrentNote->start.sec + goodBackend;
        }
        if (MissedNote) {
            MissNote(Lane, MissTime);
        }
    } while (MissedNote);
}

void Encore::RhythmEngine::BaseEngine::HitNote(const size_t lane) {
    if (ActiveSustains.empty() && whammy > 0.1) {
        earlyWhammyPrevention = true;
    }
    whammy = 0;

    auto note = chart->CurrentNoteIterators.at(lane);
    int chordSize = std::popcount(note->lane);
    if (chordSize == 0)
        chordSize = 1;
    int startTick = note->start.tick;
    double startTime = note->start.sec;
    const double offset = (EngineTime)-startTime;

    stats->accuracies.emplace_back(startTime, offset, false);

    // you REALLY dont need to be firing every few seconds
    // Encore::EncoreLog(
    //        LOG_DEBUG, TextFormat("Hit note %01i:%01i", lane,
    //        std::distance(chart->Lanes.at(lane).begin(),
    //        chart->CurrentNoteIterators.at(lane)))
    //    );
    if (!chart->sections.empty())
        chart->sections.at(chart->CurrentSection).hit++;
    auto event = NoteHitEvent(&*note);
    if (PerfectHit(startTime)) {
        stats->LastPerfectTime = EngineTime;
        if (!chart->sections.empty())
            chart->sections.at(chart->CurrentSection).perfects++;
        event.judgement = PERFECT;
    }

    if (!chart->sections.empty())
        chart->sections.at(chart->CurrentSection).notes++;
    event.offset = offset;
    stats->TotalOffset += event.offset;
    FireEvent(&event);
    SetSucceeding(true);
    stats->Combo++;
    stats->NotesHit++;
    stats->AttemptedNotes++;

    if (PerfectHit(startTime)) {
        stats->Accuracy += 1;
        stats->PerfectHits++;
        //Log::Debug("Accuracy: {}", 1);
        stats->AddHealth(1);
        HealthChangeEvent hce;
        FireEvent(&hce);
    } else {
        double acc = (goodFrontend - std::abs(offset)) / goodFrontend;
        if (acc < 0)
            acc = 0;
        stats->Accuracy += acc;

        stats->AddHealth(acc);
        HealthChangeEvent hce;
        FireEvent(&hce);
        //Log::Debug("Accuracy: {}", acc);
    }

    TrackNotificationEvent event2 { startTime, TrackNotificationEvent::HOTSTART };
    if (stats->Combo == 25 && stats->Overhits == 0 && stats->Misses == 0) {
        FireEvent(&event2);
    }

    int comboNotif = stats->Combo > 200 ? stats->Combo % 100 : stats->Combo % 50;
    if (stats->Combo == stats->ep.mult.comboForMax()
        && (inst == PartBass || inst == PlasticBass)) {
        event2.type = TrackNotificationEvent::BASSGROOVE;
        FireEvent(&event2);
    } else if (comboNotif == 0) {
        event2.type = TrackNotificationEvent::COMBO;
        event2.combo = stats->Combo;
        FireEvent(&event2);
    }
    int multiplierIncrease = stats->Combo % stats->ep.mult.notesPerMult;
    // in the unplanned/planned career mode, id like this to be adjustable
    if (multiplierIncrease == 0 && stats->Combo <= stats->ep.mult.comboForMax()) {
        UpdateMultiplier(EngineTime);
        MultFlashEvent e { false };
        FireEvent(&e);
    }
    stats->Score += stats->PointsPerNote * Multiplier * std::popcount(note->lane);

    if (note->end != 0) {
        Sustain sus {
            &*note,
            note->start.tick/(double)TheSongTime.songPPQN,
            note->end.tick/(double)TheSongTime.songPPQN,
            Multiplier,
            stats->PointsPerNote * std::popcount(note->lane),
        };
        AddSustain(sus);
    }

    if (!chart->AdvanceCurrentNote(lane))
        return;

    chart->overdrive.UpdateEventViaNote(true, startTick);
    auto od = chart->overdrive.CheckOverdrive(EngineTime);
    if (od > 0) {
        GainOverdrive(od);
    }
    chart->solos.UpdateEventViaNote(true, startTick);
}

void Encore::RhythmEngine::BaseEngine::MissNote(const size_t lane, TimeSeconds time) {
    if (stats->Combo >= stats->ep.mult.notesPerMult) {
        MultFlashEvent e { true };
        FireEvent(&e);
    }
    bool cbreak = stats->ComboToMultiplier(stats->Combo) == stats->ep.mult.max;
    stats->Combo = 0;
    stats->Misses++;
    stats->AttemptedNotes++;

    stats->RemoveHealth(cbreak);
    HealthChangeEvent hce;
    FireEvent(&hce);
    auto currentNote = chart->CurrentNoteIterators.at(lane);

    UpdateMultiplier(time);
    SetSucceeding(false);

    stats->accuracies.emplace_back(
        currentNote->start.sec, 0, true
    );
    if (!chart->sections.empty())
        chart->sections.at(chart->CurrentSection).notes++;
    chart->overdrive.MissCurrentEvent(currentNote->start.tick);
    chart->overdrive.UpdateEventViaNote(
        false, currentNote->start.tick
    );
    chart->MissedNotePointers.push_back(&*currentNote);

    chart->AdvanceCurrentNote(lane);
}

void Encore::RhythmEngine::BaseEngine::Overhit(const size_t lane) {
    bool cbreak = stats->ComboToMultiplier(stats->Combo) == stats->ep.mult.max;
    double earliestNoteTime = 0.0;
    for (size_t i = 0; i < chart->Lanes.size(); i++) {
        if (!chart->at(i).empty()) {
            if (earliestNoteTime == 0.0)
                earliestNoteTime = chart->at(i).front().start.sec;
            if (earliestNoteTime > chart->at(i).front().start.sec) {
                earliestNoteTime = chart->at(i).front().start.sec;
            }
        }
    }
    if (EngineTime < earliestNoteTime)
        return;

    stats->RemoveHealth(cbreak);
    HealthChangeEvent hce;
    FireEvent(&hce);

    // should be safe, right?
    if (!chart->sections.empty())
        chart->sections.at(chart->CurrentSection).overhits++;
    if (stats->Combo > 0) {
        MultFlashEvent e { true };
        FireEvent(&e);
    }
    FireEventTemp(OverhitEvent(lane));

    stats->Combo = 0;
    stats->Overhits++;
    UpdateMultiplier(EngineTime);

    SetSucceeding(false);
    chart->overdrive.UpdateEventViaNote(
        false, chart->CurrentNoteIterators.at(lane)->start.tick
    );
}
void Encore::RhythmEngine::BaseEngine::UpdateStats(const int instrument, int difficulty) {
    // inst = instrument >= PlasticDrums ? instrument - 5 : instrument;
    // stats->StarThresholdValue = stats->Score / chart->BaseScore;
    // for (int i = 0; i < 6; i++) {
    //     if (stats->StarThresholdValue < STAR_THRESHOLDS[inst][i]) {
    //         stats->Stars = i;
    //         return;
    //     }
    // }
    // stats->Stars = 5;
}
void Encore::RhythmEngine::BaseEngine::SetSucceeding(bool succeed) {
    if (succeed) {
        stats->CanHitHopo = true;
        stats->AudioMuted = false;
    } else {
        stats->CanHitHopo = false;
        stats->AudioMuted = true;
    }
}
void Encore::RhythmEngine::BaseEngine::UpdateWhammy(TimeSeconds time, float newWhammy) {
    if (newWhammy <= 0.1) {
        earlyWhammyPrevention = false;
    }
    if (earlyWhammyPrevention) {
        return;
    }
    whammy = newWhammy;
    if (std::abs(lastWhammyTriggerPoint - whammy) > 0.1) {
        if (!ActiveSustains.empty()) {
            bool hasODSustain = false;
            TimeSeconds minWhammyTime = INFINITY;
            TimeSeconds maxWhammyTime = 0.0;
            for (auto& sustain : ActiveSustains) {
                if (chart->overdrive.TimeIsInODPhrase(sustain.note->start.sec)) {
                    hasODSustain = true;
                    if (minWhammyTime > sustain.note->start.sec) {
                        minWhammyTime = sustain.note->start.sec;
                    }
                    if (maxWhammyTime < sustain.note->end.sec) {
                        maxWhammyTime = sustain.note->end.sec;
                    }
                }
            }
            if (time < minWhammyTime + 0.2) {
                time = minWhammyTime;
            }
            if (hasODSustain) {
                TimeSeconds whammyTime = time > minWhammyTime ? time : minWhammyTime;
                StartWhammy(whammyTime, maxWhammyTime);
            }
        }
    }
}
void Encore::RhythmEngine::BaseEngine::StartWhammy(TimeSeconds time, TimeSeconds max) {
    if (time > WhammyODTime) {
        OverdriveValue = GetOverdriveFillAtTime(time);
        OverdriveActivateTime = time;
        HasWhammyOD = true;
        WhammyMaxTime = max;
        WhammyODTime = time;
    }
}
void Encore::RhythmEngine::BaseEngine::UpdateOverdrive(TimeSeconds time) {
    if (OverdriveActive) {
        TimeSeconds endTime = GetOverdriveEndTime();
        if (time > endTime) {
            EndOverdrive(endTime);
        }
    }
}
void Encore::RhythmEngine::BaseEngine::GainOverdrive(double amount) {
    if (OverdriveActive) {
        auto oldValue = GetOverdriveFillAtTime(EngineTime);
        OverdriveValue = oldValue+amount;
        OverdriveActivateTime = EngineTime;
        assert(OverdriveValue >= oldValue);
    } else {
        OverdriveValue += amount;
    }
    if (OverdriveValue > 1) {
        OverdriveValue = 1;
    }
}
void Encore::RhythmEngine::BaseEngine::ActivateOverdrive(TimeSeconds time) {
    OverdriveActive = true;
    OverdriveActivateTime = time;
    UpdateMultiplier(time);
    OnOverdriveActivate(time);
    // TODO: fire all the events n shit
}
void Encore::RhythmEngine::BaseEngine::EndOverdrive(TimeSeconds time) {
    OverdriveActive = false;
    HasWhammyOD = false;
    OverdriveValue = 0;
    UpdateMultiplier(time);
}
void Encore::RhythmEngine::BaseEngine::AddSustain(Sustain sustain) {
    ActiveSustains.push_back(sustain);
}
void Encore::RhythmEngine::BaseEngine::ClearSustain(Sustain &sustain) {
    for (auto it = ActiveSustains.begin(); it != ActiveSustains.end(); it++) {
        if (&*it == &sustain) {
            ActiveSustains.erase(it);
            break;
        }
    }
}
void Encore::RhythmEngine::BaseEngine::UpdateSustains() {
    bool doneProcessingSustains = true;
    TimeBeats time = TheSongTime.SecondsToBeat(EngineTime);
    do {
        doneProcessingSustains = true;
        for (auto it = ActiveSustains.begin(); it != ActiveSustains.end(); it++) {
            if (it->end <= time) {
                CompleteSustain(EngineTime, *it);
                ClearSustain(*it);
                doneProcessingSustains = false;
                break;
            }
        }
    } while (!doneProcessingSustains);
}
double Encore::RhythmEngine::BaseEngine::GetOverdriveFillAtTime(TimeSeconds time) {
    TimeODTicks curTick = TheSongTime.SecondsToODTick(time);
    double accumulate = OverdriveValue;
    if (OverdriveActive && time > OverdriveActivateTime) {
        TimeODTicks actTime = TheSongTime.SecondsToODTick(OverdriveActivateTime);
        TimeODTicks timeInOD = curTick - actTime;
        accumulate -= timeInOD * stats->ep.od.DrainPerTick();
    }
    if (HasWhammyOD && time >= WhammyODTime) {
        TimeSeconds whammyTime = time;
        if (whammyTime > WhammyMaxTime) {
            whammyTime = WhammyMaxTime;
        }
        TimeODTicks curBeat = TheSongTime.SecondsToODTick(whammyTime);
        TimeODTicks whammyBeat = TheSongTime.SecondsToODTick(WhammyODTime);
        TimeODTicks timeWhammied = curBeat - whammyBeat;
        timeWhammied = std::clamp(timeWhammied, 0.0, whammyHoldTime);
        accumulate += timeWhammied * whammyGainPerTick; // idk what this value should be, will be adjusted
    }
    if (accumulate > 1) {
        accumulate = 1;
    }
    return accumulate;
}
TimeSeconds Encore::RhythmEngine::BaseEngine::GetOverdriveEndTime() {
    // If called while overdrive isn't active we can just use this to predict it
    TimeSeconds start = OverdriveActive ? OverdriveActivateTime : EngineTime;

    // First check the end time with whammy + drain
    if (HasWhammyOD && start >= WhammyODTime) {
        TimeODTicks whammyStart = TheSongTime.SecondsToODTick(WhammyODTime);
        TimeODTicks whammyEnd = whammyStart + whammyHoldTime;
        TimeSeconds endTime = TheSongTime.ODTickToSeconds(whammyEnd);
        if (endTime > WhammyMaxTime) {
            endTime = WhammyMaxTime;
            whammyEnd = TheSongTime.SecondsToODTick(endTime);
        }
        TimeSeconds GainAmount = ((whammyEnd - whammyStart) * whammyGainPerTick);
        TimeSeconds DrainLength = TheSongTime.ODTickToSeconds(TheSongTime.SecondsToODTick(start) + std::min(1.0, OverdriveValue+GainAmount) / stats->ep.od.DrainPerTick());
        return DrainLength;
    } else {
        return TheSongTime.ODTickToSeconds(TheSongTime.SecondsToODTick(start) + (OverdriveValue) / stats->ep.od.DrainPerTick());
    }
}
double Encore::RhythmEngine::BaseEngine::GetVisualScoreAtTime(TimeSeconds time) {
    TimeBeats beats = TheSongTime.SecondsToBeat(time);
    double accumulate = stats->Score;

    for (auto& sustain : ActiveSustains) {
        accumulate += sustain.GetValueAtTime(beats);
    }

    return accumulate;
}
void Encore::RhythmEngine::BaseEngine::CompleteSustain(
    TimeSeconds time, Sustain &sustain
) {
    auto beats = TheSongTime.SecondsToBeat(time);
    stats->Score += sustain.GetValueAtTime(beats);
}
void Encore::RhythmEngine::BaseEngine::DropSustain(TimeSeconds time, Sustain &sustain) {
    CompleteSustain(time, sustain);
    stats->AudioMuted = true;
    chart->DroppedSustainPointers.push_back(sustain.note);
    ClearSustain(sustain);
}
void Encore::RhythmEngine::BaseEngine::UpdateMultiplier(TimeSeconds time) {
    Multiplier = stats->ComboToMultiplier(stats->Combo) * (OverdriveActive ? stats->ep.mult.odMult : 1);
    if (deferMultiplierUpdates) {
        defferredMultiplierUpdates.push_back({time, Multiplier});
    } else {
        SegmentSustains(time, Multiplier);
    }
}
void Encore::RhythmEngine::BaseEngine::SegmentSustains(TimeSeconds time, double newMultiplier) {
    for (auto& sustain : ActiveSustains) {
        CompleteSustain(time, sustain);
        TimeBeats newStart = TheSongTime.SecondsToBeat(time);
        if (newStart > sustain.start) {
            sustain.start = newStart;
        }
        sustain.multiplier = newMultiplier;
    }
}

Encore::RhythmEngine::TimePoint Encore::RhythmEngine::BaseEngine::NextNoteTime() {
    TimePoint tp { 0, 0 };
    for (size_t Lane = 0; Lane < chart->Lanes.size(); Lane++) {
        if (chart->CurrentNoteIterators.at(Lane) == chart->at(Lane).end())
            continue;
        if (tp == 0 && tp == 0.0)
            tp = chart->CurrentNoteIterators.at(Lane)->start;
        if (chart->CurrentNoteIterators.at(Lane)->start < tp)
            tp = chart->CurrentNoteIterators.at(Lane)->start;
    }
    return tp;
}

Encore::RhythmEngine::TimePoint Encore::RhythmEngine::BaseEngine::LastNoteTime() {
    TimePoint tp { 1, 0 };
    for (size_t Lane = 0; Lane < chart->Lanes.size(); Lane++) {
        if (chart->CurrentNoteIterators.at(Lane) == chart->at(Lane).end())
            continue;
        auto lastNote = (chart->CurrentNoteIterators.at(Lane) - 1);
        if (lastNote < chart->at(Lane).begin())
            continue;
        if (tp == 0) {
            tp = lastNote->start;
            continue;
        }
        if (lastNote->start > tp) {
            if (lastNote->end == 0) {
                tp = lastNote->start;
            } else {
                tp = lastNote->end;
            }
        }
    }
    return tp;
}
//
// Created by maria on 17/05/2025.
//

#include "MidiPadLoader.h"
#include "RhythmEngine/REenums.h"

#include "RhythmEngine/scoring.h"

[[nodiscard]] bool IsInLiftMarkerRange(int diff, const smf::MidiEvent &event) {
    return event[1] >= LiftMinMaxDiff[diff].first
        && event[1] <= LiftMinMaxDiff[diff].second;
}
[[nodiscard]] int GetLiftLane(int diff, const smf::MidiEvent &event) {
    return event[1] - LiftMinMaxDiff[diff].first;
}
void Encore::RhythmEngine::MidiPadLoader::CreateLiftMarker(const smf::MidiEvent &event) {
    LiftMarkers[GetLiftLane(Difficulty, event)].emplace(event.tick);
}
void Encore::RhythmEngine::MidiPadLoader::GetNoteModifiers(smf::MidiEventList track) {
    track.linkNotePairs();
    for (int eventInt = 0; eventInt < track.size(); eventInt++) {
        smf::MidiEvent &event = track[eventInt];
        if (IsInLiftMarkerRange(Difficulty, event) && event.isNoteOn()) {
            CreateLiftMarker(event);
        }
    }
};

void Encore::RhythmEngine::MidiPadLoader::CheckModifiers(const smf::MidiEvent &event) {
    if (!LiftMarkers[GetEventLane(Difficulty, event)].empty()) {
        if (LiftMarkers[GetEventLane(Difficulty, event)].front() < event.tick)
            LiftMarkers[GetEventLane(Difficulty, event)].pop();
    }
}

void Encore::RhythmEngine::MidiPadLoader::CheckEvents(const smf::MidiEvent &event) {
    ITERATE_EVENT_BY_NOTE(solos, CurrentSolo, event)
    if (!chart.overdrive.empty()) {
        if (CurrentOverdrive < chart.overdrive.size() - 1
            && chart.overdrive[CurrentOverdrive].StartTick
                    + chart.overdrive[CurrentOverdrive].TickLength
                <= event.tick)
            CurrentOverdrive++;
    }
    if (!chart.sections.empty()) {
        if (CurrentSection < chart.sections.size() - 1 && event.tick >= chart.sections[CurrentSection+1].tickStart)
            CurrentSection++;
    }
    // ITERATE_EVENT_BY_NOTE(overdrive, CurrentOverdrive, event)
}

[[nodiscard]] int
Encore::RhythmEngine::MidiPadLoader::GetNoteType(const smf::MidiEvent &event) {
    if (!LiftMarkers[GetEventLane(Difficulty, event)].empty()) {
        if (LiftMarkers[GetEventLane(Difficulty, event)].front() == event.tick) {
            return 1; // lift
        }
    }
    return 0;
}
void Encore::RhythmEngine::MidiPadLoader::GetChartEvents(smf::MidiEventList track) {
    track.linkNotePairs();
    for (int eventInt = 0; eventInt < track.size(); eventInt++) {
        smf::MidiEvent &event = track[eventInt];
        ATTEMPT_TO_ADD_CHART_EVENT(116, overdrive, event);
        ATTEMPT_TO_ADD_CHART_EVENT(108, solos, event);
        ATTEMPT_TO_ADD_CHART_EVENT(101, solos, event);
        ATTEMPT_TO_ADD_CHART_EVENT(127, trills, event);
        ATTEMPT_TO_ADD_CHART_EVENT(126, rolls, event);
    }
}

void Encore::RhythmEngine::MidiPadLoader::CreateNote(const smf::MidiEvent &event) {
    int lengthTicks = event.getLinkedEvent()->tick - event.tick;
    double lengthSec = event.getLinkedEvent()->seconds - event.seconds;
    if (event.getLinkedEvent()->tick - event.tick < 170) {
        lengthTicks = 0;
        lengthSec = 0;
    }
    chart.BaseScore += BASE_SCORE_NOTE_POINT;

    if (lengthTicks > 0) {
        chart.BaseScore += (lengthTicks / 480) * BASE_SCORE_SUSTAIN_POINTS;
    }
    chart[GetEventLane(Difficulty, event)].emplace_back(

        event.tick,
        lengthTicks,
        event.seconds,
        lengthSec,
        GetNoteType(event),
        PlasticFrets[GetEventLane(Difficulty, event)]

    );

    if (!chart.sections.empty()) {
        int end = midiFile->getFileDurationInTicks();
        if (CurrentSection < chart.sections.size() - 1) {
            end = chart.sections.at(CurrentSection + 1).tickStart;
        }
        if (event.tick >= chart.sections.at(CurrentSection).tickStart && event.tick < end)
            chart.sections.at(CurrentSection).notes++;
    }
    // i hate how solos need note counts before entering lol
    if (!chart.solos.empty()) {
        if (event.tick >= chart.solos[CurrentSolo].StartTick  && event.tick < chart.solos[CurrentSolo].StartTick + chart.solos[CurrentSolo].TickLength) {
            chart.solos[CurrentSolo].NoteCount++;
        }
    }
    if (!chart.overdrive.empty()) {
        if (event.tick >= chart.overdrive[CurrentOverdrive].StartTick && event.tick < chart.overdrive[CurrentOverdrive].StartTick + chart.overdrive[CurrentOverdrive].TickLength) {
            chart.overdrive[CurrentOverdrive].NoteCount++;
        }
    }
}

void Encore::RhythmEngine::MidiPadLoader::GetNotes(smf::MidiEventList track) {
    track.linkNotePairs();
    for (int eventInt = 0; eventInt < track.size(); eventInt++) {
        smf::MidiEvent &event = track[eventInt];
        if (event[0] == 255)
            continue;
        CheckEvents(event);
        if (IsInPitchRange(Difficulty, event) && event.isNoteOn()) {
            CheckModifiers(event);
            CreateNote(event);
        }
    }
};
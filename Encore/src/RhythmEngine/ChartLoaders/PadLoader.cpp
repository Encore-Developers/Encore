//
// Created by maria on 17/05/2025.
//

#include "PadLoader.h"

Encore::RhythmEngine::PadLoader::PadLoader(int diff_, smf::MidiEventList track_)
    : BaseLoader(Difficulty = diff_, track = track_) {}

[[nodiscard]] bool IsInPitchRange(int diff, const smf::MidiEvent &event) {
    return event[1] >= MinMaxDiff[diff].first && event[1] <= MinMaxDiff[diff].second;
}
[[nodiscard]] int GetEventLane(int diff, const smf::MidiEvent &event) {
    return event[1] - MinMaxDiff[diff].first;
}
[[nodiscard]] bool IsInLiftMarkerRange(int diff, const smf::MidiEvent &event) {
    return event[1] >= LiftMinMaxDiff[diff].first
        && event[1] <= LiftMinMaxDiff[diff].second;
}
[[nodiscard]] int GetLiftLane(int diff, const smf::MidiEvent &event) {
    return event[1] - LiftMinMaxDiff[diff].first;
}
void Encore::RhythmEngine::PadLoader::CreateLiftMarker(const smf::MidiEvent &event) {
    LiftMarkers[GetLiftLane(Difficulty, event)].emplace(event.tick);
}
void Encore::RhythmEngine::PadLoader::GetNoteModifiers() {
    for (int eventInt = 0; eventInt < track.size(); eventInt++) {
        smf::MidiEvent &event = track[eventInt];
        if (IsInLiftMarkerRange(Difficulty, event) && event.isNoteOn()) {
            CreateLiftMarker(event);
        }
    }
};
void Encore::RhythmEngine::PadLoader::CheckModifiers(const smf::MidiEvent &event) {
    if (!LiftMarkers[GetEventLane(Difficulty, event)].empty()) {
        if (LiftMarkers[GetEventLane(Difficulty, event)].front() < event.tick)
            LiftMarkers[GetEventLane(Difficulty, event)].pop();
    }
}

void Encore::RhythmEngine::PadLoader::CheckEvents(const smf::MidiEvent &event) {
    if (chart.solos[CurrentSolo].EndTick < event.tick
        && CurrentSolo < chart.solos.size() - 1)
        CurrentSolo++;

    if (chart.overdrive[CurrentOverdrive].EndTick < event.tick
        && CurrentOverdrive < chart.overdrive.size() - 1)
        CurrentOverdrive++;
}

[[nodiscard]] int
Encore::RhythmEngine::PadLoader::GetNoteType(const smf::MidiEvent &event) {
    if (LiftMarkers[GetEventLane(Difficulty, event)].front() == event.tick) {
        return 1; // lift
    }
    return 0;
}
void Encore::RhythmEngine::PadLoader::GetChartEvents() {
    for (int eventInt = 0; eventInt < track.size(); eventInt++) {
        smf::MidiEvent &event = track[eventInt];
        if (event[1] == 116 && event.isNoteOn()) {
            chart.overdrive.emplace_back(
                event.tick,
                event.seconds,
                event.getLinkedEvent()->tick - event.tick,
                event.getLinkedEvent()->seconds - event.seconds
            );
        }
        if ((event[1] == 108 || event[1] == 101) && event.isNoteOn()) {
            chart.solos.emplace_back(
                event.tick,
                event.seconds,
                event.getLinkedEvent()->tick - event.tick,
                event.getLinkedEvent()->seconds - event.seconds
            );
        }
    }
}

void Encore::RhythmEngine::PadLoader::CreateNote(const smf::MidiEvent &event) {
    chart[GetEventLane(Difficulty, event)].push(
        {
            event.tick,
            event.getLinkedEvent()->tick - event.tick,
            event.seconds,
            event.getLinkedEvent()->seconds - event.seconds,
            GetNoteType(event),
        }
    );
    // i hate how solos need note counts before entering lol
    if (event.tick >= chart.solos[CurrentSolo].StartTick) {
        chart.solos[CurrentSolo].NoteCount++;
    }
}

void Encore::RhythmEngine::PadLoader::GetNotes() {
    for (int eventInt = 0; eventInt < track.size(); eventInt++) {
        smf::MidiEvent &event = track[eventInt];
        CheckEvents(event);
        CheckModifiers(event);
        if (IsInLiftMarkerRange(Difficulty, event) && event.isNoteOn()) {
            CreateNote(event);
        }
    }
};
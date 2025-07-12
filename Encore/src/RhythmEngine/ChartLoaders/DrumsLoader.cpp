//
// Created by maria on 17/05/2025.
//

#include "DrumsLoader.h"
// i hate sysex

void Encore::RhythmEngine::DrumsLoader::CheckToms(const smf::MidiEvent &event) {
    if (event.isNoteOn()) {
        if (event[1] == 110) {
            YellowTom.emplace(event.tick, event.getLinkedEvent()->tick);
        } else if (event[1] == 111) {
            BlueTom.emplace(event.tick, event.getLinkedEvent()->tick);
        } else if (event[1] == 112) {
            GreenTom.emplace(event.tick, event.getLinkedEvent()->tick);
        };
        ;
    }
}

[[nodiscard]] int
Encore::RhythmEngine::DrumsLoader::GetNoteType(const smf::MidiEvent &event) {
    if (GetEventLane(Difficulty, event) == 4) {
        if (!GreenTom.empty()) {
            if (GreenTom.front().first <= event.tick) {
                return 0;
            }
        }
    }
    if (GetEventLane(Difficulty, event) == 3) {
        if (!BlueTom.empty()) {
            if (BlueTom.front().first <= event.tick) {
                return 0;
            }
        }
    }
    if (GetEventLane(Difficulty, event) == 2) {
        if (!YellowTom.empty()) {
            if (YellowTom.front().first <= event.tick) {
                return 0;
            }
        }
    }
    if (GetEventLane(Difficulty, event) <= 1) {
        return 0;
    }
    return 1;
}
void Encore::RhythmEngine::DrumsLoader::CheckEvents(const smf::MidiEvent &event) {
    if (!chart.solos.empty()) {
        if (CurrentSolo < chart.solos.size() - 1
            && chart.solos[CurrentSolo].StartTick + chart.solos[CurrentSolo].EndTick
                < event.tick)
            CurrentSolo++;
    }

    if (!chart.overdrive.empty()) {
        if (CurrentOverdrive < chart.overdrive.size() - 1
            && chart.overdrive[CurrentOverdrive].StartTick
                    + chart.overdrive[CurrentOverdrive].EndTick
                < event.tick)
            CurrentOverdrive++;
    }
}
void Encore::RhythmEngine::DrumsLoader::GetChartEvents(smf::MidiEventList track) {
    track.linkNotePairs();
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
        if (event[1] == 103 && event.isNoteOn()) {
            chart.solos.emplace_back(
                event.tick,
                event.seconds,
                event.getLinkedEvent()->tick - event.tick,
                event.getLinkedEvent()->seconds - event.seconds
            );
        }
    }
}
void Encore::RhythmEngine::DrumsLoader::CreateNote(const smf::MidiEvent &event) {
    int lengthTicks = event.getLinkedEvent()->tick - event.tick;
    double lengthSec = event.getLinkedEvent()->seconds - event.seconds;
    if (event.getLinkedEvent()->tick - event.tick < 170) {
        lengthTicks = 0;
        lengthSec = 0;
    }
    chart[GetEventLane(Difficulty, event)].emplace_back(
        event.tick, lengthTicks, event.seconds, lengthSec, GetNoteType(event)
    );
    if (!chart.solos.empty()) {
        if (event.tick >= chart.solos[CurrentSolo].StartTick) {
            chart.solos[CurrentSolo].NoteCount++;
        }
    }
}

void Encore::RhythmEngine::DrumsLoader::GetNoteModifiers(smf::MidiEventList track) {
    track.linkNotePairs();
    for (int eventInt = 0; eventInt < track.size(); eventInt++) {
        smf::MidiEvent &event = track[eventInt];
        CheckToms(event);
        // can have events for things like overdrive here
        // ugh
        // can i just have notes work for now
    }
}

void Encore::RhythmEngine::DrumsLoader::CheckModifiers(const smf::MidiEvent &event) {
    if (!BlueTom.empty()) {
        if (BlueTom.front().second <= event.tick)
            BlueTom.pop();
    }
    if (!YellowTom.empty()) {
        if (YellowTom.front().second <= event.tick)
            YellowTom.pop();
    }
    if (!GreenTom.empty()) {
        if (GreenTom.front().second <= event.tick)
            GreenTom.pop();
    }
}

void Encore::RhythmEngine::DrumsLoader::GetNotes(smf::MidiEventList track) {
    track.linkNotePairs();
    for (int eventInt = 0; eventInt < track.size(); eventInt++) {
        smf::MidiEvent &event = track[eventInt];
        // im tired boss
        if (event[0] == 255)
            continue;
        CheckEvents(event);
        CheckModifiers(event);
        if (IsInPitchRange(Difficulty, event) && event.isNoteOn()) {
            CreateNote(event);
        }
    }
}

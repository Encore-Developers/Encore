//
// Created by maria on 17/05/2025.
//

#include "DrumsLoader.h"

#include "song/scoring.h"
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
    ITERATE_EVENT_BY_NOTE(solos, CurrentSolo, event)
    ITERATE_EVENT_BY_NOTE(overdrive, CurrentOverdrive, event)
}
void Encore::RhythmEngine::DrumsLoader::GetChartEvents(smf::MidiEventList track) {
    track.linkNotePairs();
    for (int eventInt = 0; eventInt < track.size(); eventInt++) {
        smf::MidiEvent &event = track[eventInt];
        ATTEMPT_TO_ADD_CHART_EVENT(116, overdrive, event);
        ATTEMPT_TO_ADD_CHART_EVENT(103, solos, event);
        ATTEMPT_TO_ADD_CHART_EVENT(127, trills, event);
        ATTEMPT_TO_ADD_CHART_EVENT(126, rolls, event);
    }
}
void Encore::RhythmEngine::DrumsLoader::CreateNote(const smf::MidiEvent &event) {
    int lengthTicks = event.getLinkedEvent()->tick - event.tick;
    double lengthSec = event.getLinkedEvent()->seconds - event.seconds;
    if (event.getLinkedEvent()->tick - event.tick < 170) {
        lengthTicks = 0;
        lengthSec = 0;
    }
    int lane = GetEventLane(Difficulty, event);
    int type = GetNoteType(event);
    if (!DiscoFlip.empty()) {
        if (DiscoFlip.front().first <= event.tick) {
            if (lane == 1) {
                lane = 2;
                type = 1;
            } else if (lane == 2) {
                lane = 1;
                type = 0;
            }
        }
    }
    chart.BaseScore += BASE_SCORE_NOTE_POINT;
    chart[lane].emplace_back(
        event.tick, lengthTicks, event.seconds, lengthSec, type, PlasticFrets[1]
    );
    if (!chart.solos.empty()) {
        if (event.tick >= chart.solos[CurrentSolo].StartTick) {
            chart.solos[CurrentSolo].NoteCount++;
        }
    }
    if (!chart.overdrive.empty()) {
        if (event.tick >= chart.overdrive[CurrentOverdrive].StartTick) {
            chart.overdrive[CurrentOverdrive].NoteCount++;
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
    ITERATE_MODIFIER_BY_NOTE(BlueTom, event)
    ITERATE_MODIFIER_BY_NOTE(YellowTom, event)
    ITERATE_MODIFIER_BY_NOTE(GreenTom, event)
    ITERATE_MODIFIER_BY_NOTE(DiscoFlip, event)
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

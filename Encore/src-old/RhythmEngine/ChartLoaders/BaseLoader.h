//
// Created by maria on 17/05/2025.
//

#ifndef BASELOADER_H
#define BASELOADER_H

#include "RhythmEngine/NoteVector.h"
#include "midifile/MidiFile.h"

#define ATTEMPT_TO_ADD_CHART_EVENT(lane, type, event)                                    \
    if (event[1] == lane && event.isNoteOn()) {                                          \
        chart.type.emplace_back(                                                         \
            event.tick,                                                                  \
            event.seconds,                                                               \
            event.getLinkedEvent()->tick - event.tick,                                   \
            event.getLinkedEvent()->seconds - event.seconds                              \
        );                                                                               \
        continue;                                                                        \
    }

#define ITERATE_MODIFIER_BY_NOTE(modifier, note)                                         \
    if (!modifier.empty()) {                                                             \
        if (modifier.front().second <= note.tick)                                        \
            modifier.pop();                                                              \
    }

#define ITERATE_EVENT_BY_NOTE(event, itr, note)                                          \
    if (!chart.event.empty()) {                                                          \
        if (itr < chart.event.size() - 1                                                 \
            && chart.event[itr].StartTick + chart.event[itr].EndTick < note.tick)        \
            itr++;                                                                       \
    }

namespace Encore::RhythmEngine {
    class BaseLoader {
        // returns -1 for no event, good for plastic guitar i think
        virtual void GetChartEvents(smf::MidiEventList track) {};
        virtual void GetNoteModifiers(smf::MidiEventList track) {};
        virtual void GetNotes(smf::MidiEventList track) {};
        virtual int GetNoteType(const smf::MidiEvent &event) { return 0; };

    public:
        virtual ~BaseLoader() = default;

        [[nodiscard]] static bool IsInPitchRange(int diff, const smf::MidiEvent &event) {
            return event[1] >= MinMaxDiff[diff].first
                && event[1] <= MinMaxDiff[diff].second;
        }

        [[nodiscard]] static bool IsInPitchRangeGB(int diff, const smf::MidiEvent &event) {
            return event[1] >= GuitarMinMaxDiff[diff].first
                && event[1] <= GuitarMinMaxDiff[diff].second;
        }

        [[nodiscard]] static int GetEventLane(int diff, const smf::MidiEvent &event) {
            return event[1] - MinMaxDiff[diff].first;
        }

        // when handing the loader a midi
        // run these few things:
        // midiFile.absoluteTicks();
        // midiFile.doTimeAnalysis();
        BaseLoader(int diff_, int thresh_) : Difficulty(diff_), Threshold(thresh_) {}

        int CurrentSolo = 0;
        int CurrentOverdrive = 0;
        int CurrentTrill = 0;
        int CurrentRoll = 0;

        BaseChart chart;
        int Difficulty;
        int Threshold; // shouldnt be here but who care
        virtual void LoadChart(smf::MidiEventList track) {
            track.linkNotePairs();
            // first get events, hopos, taps, lifts, the likes
            GetChartEvents(track);
            GetNoteModifiers(track);
            GetNotes(track);
        };
        // have a for loop in here to get the shit in the right lanes
    };
}

#endif // BASELOADER_H

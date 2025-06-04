//
// Created by maria on 17/05/2025.
//

#ifndef BASELOADER_H
#define BASELOADER_H

#include "RhythmEngine/NoteVector.h"
#include "midifile/MidiFile.h"

namespace Encore::RhythmEngine {
    template <typename ChartType>
    class BaseLoader {
        // returns -1 for no event, good for plastic guitar i think
        virtual void GetChartEvents() {};
        virtual void GetNoteModifiers() {};
        virtual void GetNotes() {};
        virtual int GetNoteType(const smf::MidiEvent &event) { return 0; };

    public:
        virtual ~BaseLoader() = default;

        [[nodiscard]] static bool IsInPitchRange(int diff, const smf::MidiEvent &event) {
            return event[1] >= MinMaxDiff[diff].first
                && event[1] <= MinMaxDiff[diff].second;
        }
        [[nodiscard]] static int GetEventLane(int diff, const smf::MidiEvent &event) {
            return event[1] - MinMaxDiff[diff].first;
        }
        // when handing the loader a midi
        // run these few things:
        // midiFile.absoluteTicks();
        // midiFile.doTimeAnalysis();
        BaseLoader(int diff_, smf::MidiEventList track_) {
            Difficulty = diff_;
            track = track_;
        }

        ChartType chart;
        int Difficulty;
        smf::MidiEventList track;

        virtual void LoadChart() {
            track.linkNotePairs();
            // first get events, hopos, taps, lifts, the likes
            GetChartEvents();
            GetNoteModifiers();
            GetNotes();
        };
        // have a for loop in here to get the shit in the right lanes
    };
}

#endif // BASELOADER_H

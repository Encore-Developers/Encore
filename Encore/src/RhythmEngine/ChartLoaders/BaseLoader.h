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
        [[nodiscard]] static int GetEventLane(int diff, const smf::MidiEvent &event) {
            return event[1] - MinMaxDiff[diff].first;
        }
        // when handing the loader a midi
        // run these few things:
        // midiFile.absoluteTicks();
        // midiFile.doTimeAnalysis();
        BaseLoader(int diff_, int thresh_) : Difficulty(diff_), Threshold(thresh_) {}

        ChartType chart;
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

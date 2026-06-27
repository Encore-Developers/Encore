//
// Created by maria on 17/05/2025.
//

#ifndef DRUMSLOADER_H
#define DRUMSLOADER_H

#include "../BaseLoader.h"
#include <queue>

namespace Encore::RhythmEngine {
    class MidiDrumsLoader final : public BaseLoader {
        std::pair<int, int>DrumLanes[4] { {60, 65}, {72, 77}, {84, 89}, {96, 101} };
        // start, end
        std::queue<std::pair<int, int> > GreenTom = {};
        std::queue<std::pair<int, int> > BlueTom = {};
        std::queue<std::pair<int, int> > YellowTom = {};
        std::queue<std::pair<int, int> > DiscoFlip = {};
        std::queue<int> OpenMarker = {};


        [[nodiscard]] bool IsInPitchRange(int diff, const smf::MidiEvent &event) override {
            return event[1] >= DrumLanes[diff].first
                && event[1] <= DrumLanes[diff].second;
        }

        [[nodiscard]] int GetEventLane(const int diff, const smf::MidiEvent &event) override {
            return event[1] - DrumLanes[diff].first;
        }

        void CheckToms(const smf::MidiEvent &event);
        void CheckModifiers(const smf::MidiEvent &event);
        void CheckEvents(const smf::MidiEvent &event);
        void CreateNote(const smf::MidiEvent &event);
        int GetNoteType(const smf::MidiEvent &event) override;

        void GetChartEvents(smf::MidiEventList &track) override;
        void GetNoteModifiers(smf::MidiEventList &track) override;
        void GetNotes(smf::MidiEventList &track) override;

    public:
        void LoadChart(smf::MidiEventList &track) override {
            // first get events, hopos, taps, lifts, the likes
            GetChartEvents(track);
            GetNoteModifiers(track);
            GetNotes(track);
            if (chart.at(5).empty()) {
                chart.Lanes.pop_back();
                chart.CurrentNoteIterators.pop_back();
                chart.HeldNotePointers.pop_back();
                chart.size = 5;
            } else {
                for (int i = 0; i < chart.size; i++) {
                    if (i == 0) continue;
                    for (auto& note : chart.at(i)) {
                        note.NoteType = (i + 1) % 2;
                    }
                }
            }
        };

        MidiDrumsLoader(int diff_, smf::MidiFile* midiFile_) : BaseLoader(diff_, 0, midiFile_) {
            Resolution = midiFile_->getTPQ();
            chart.resize(6);
        }
    };
}

#endif // DRUMSLOADER_H

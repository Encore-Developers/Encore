//
// Created by maria on 17/05/2025.
//

#ifndef DRUMSLOADER_H
#define DRUMSLOADER_H

#include "../BaseLoader.h"
#include <queue>

namespace Encore::RhythmEngine {
    class MidiDrumsLoader final : public BaseLoader {
        // start, end
        std::queue<std::pair<int, int> > GreenTom = {};
        std::queue<std::pair<int, int> > BlueTom = {};
        std::queue<std::pair<int, int> > YellowTom = {};
        std::queue<std::pair<int, int> > DiscoFlip = {};
        std::queue<int> OpenMarker = {};

        void CheckToms(const smf::MidiEvent &event);
        void CheckModifiers(const smf::MidiEvent &event);
        void CheckEvents(const smf::MidiEvent &event);
        void CreateNote(const smf::MidiEvent &event);
        int GetNoteType(const smf::MidiEvent &event) override;

        void GetChartEvents(smf::MidiEventList track) override;
        void GetNoteModifiers(smf::MidiEventList track) override;
        void GetNotes(smf::MidiEventList track) override;

    public:
        MidiDrumsLoader(int diff_, smf::MidiFile* midiFile_) : BaseLoader(diff_, 0, midiFile_) {
            Resolution = midiFile_->getTPQ();
        }
    };
}

#endif // DRUMSLOADER_H

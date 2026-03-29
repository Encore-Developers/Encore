//
// Created by maria on 17/05/2025.
//

#ifndef DRUMSLOADER_H
#define DRUMSLOADER_H

#include "BaseLoader.h"

namespace Encore::RhythmEngine {
    class DrumsLoader final : public BaseLoader {
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
        DrumsLoader(int diff_, smf::MidiFile* midiFile_) : BaseLoader(diff_, 0, midiFile_) {}
    };
}

#endif // DRUMSLOADER_H

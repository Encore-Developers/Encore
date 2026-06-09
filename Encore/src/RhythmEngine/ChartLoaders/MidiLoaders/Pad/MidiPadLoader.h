//
// Created by maria on 17/05/2025.
//

#ifndef PADLOADER_H
#define PADLOADER_H

#include "../BaseLoader.h"
#include <array>
#include <queue>

namespace Encore::RhythmEngine {
    class MidiPadLoader final : public BaseLoader {
        std::array<std::queue<int>, 5> LiftMarkers = {};
        void CreateLiftMarker(const smf::MidiEvent &event);
        void CheckModifiers(const smf::MidiEvent &event);
        void CheckEvents(const smf::MidiEvent &event);
        void CreateNote(const smf::MidiEvent &event);
        int GetNoteType(const smf::MidiEvent &event) override;

        void GetChartEvents(smf::MidiEventList track) override;
        void GetNoteModifiers(smf::MidiEventList track) override;
        void GetNotes(smf::MidiEventList track) override;

    public:
        MidiPadLoader(int diff_, int thresh_, smf::MidiFile* midiFile_, int _maxMult = 4)
            : BaseLoader(diff_, thresh_, midiFile_, _maxMult)  {
            Resolution = midiFile_->getTPQ();
        }
    };
}

#endif // PADLOADER_H

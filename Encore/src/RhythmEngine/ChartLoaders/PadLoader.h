//
// Created by maria on 17/05/2025.
//

#ifndef PADLOADER_H
#define PADLOADER_H

#include "BaseLoader.h"

namespace Encore::RhythmEngine {
    class PadLoader final : public BaseLoader {
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
        PadLoader(int diff_, smf::MidiFile *midiFile_);
    };
}

#endif // PADLOADER_H

//
// Created by maria on 17/05/2025.
//

#ifndef PADLOADER_H
#define PADLOADER_H

#include "BaseLoader.h"

namespace Encore::RhythmEngine {
    class PadLoader final : public BaseLoader<PadChart> {
        std::array<std::queue<int>, 5> LiftMarkers = {};
        int CurrentSolo = 0;
        int CurrentOverdrive = 0;
        void CreateLiftMarker(const smf::MidiEvent &event);
        void CheckModifiers(const smf::MidiEvent &event);
        void CheckEvents(const smf::MidiEvent &event);
        void CreateNote(const smf::MidiEvent &event);
        int GetNoteType(const smf::MidiEvent &event) override;

        void GetChartEvents(smf::MidiEventList track) override;
        void GetNoteModifiers(smf::MidiEventList track) override;
        void GetNotes(smf::MidiEventList track) override;

    public:
        PadLoader(int diff_);
    };
}

#endif // PADLOADER_H

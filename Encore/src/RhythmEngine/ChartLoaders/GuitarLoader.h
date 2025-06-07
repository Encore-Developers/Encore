//
// Created by maria on 17/05/2025.
//

#ifndef GUITARLOADER_H
#define GUITARLOADER_H

#include "BaseLoader.h"

namespace Encore::RhythmEngine {
    class GuitarLoader final : public BaseLoader<GuitarChart> {
        // start, end
        std::queue<std::pair<int, int> > ForceHopoOn = {};
        std::queue<std::pair<int, int> > ForceHopoOff = {};
        std::queue<std::pair<int, int> > TapMarker = {};
        std::queue<int> OpenMarker = {};
        int CurrentSolo = 0;
        int CurrentOverdrive = 0;

        void CheckSysEx(const smf::MidiEvent &event);
        void SysExTap(const smf::MidiEvent &event);
        void SysExOpen(const smf::MidiEvent &event);
        void CheckHopos(const smf::MidiEvent &event);
        void CheckTaps(const smf::MidiEvent &event);
        void CheckModifiers(const smf::MidiEvent &event);
        void CheckEvents(const smf::MidiEvent &event);
        void CreateNote(const smf::MidiEvent &event);
        int GetNoteType(const smf::MidiEvent &event) override;

        void GetChartEvents() override;
        void GetNoteModifiers() override;
        void GetNotes() override;

    public:
        GuitarLoader(int diff_, smf::MidiEventList track_);
    };
}

#endif // GUITARLOADER_H

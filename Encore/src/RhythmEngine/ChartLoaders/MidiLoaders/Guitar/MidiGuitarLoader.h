//
// Created by maria on 17/05/2025.
//

#ifndef GUITARLOADER_H
#define GUITARLOADER_H

#include "../BaseLoader.h"
#include <queue>

namespace Encore::RhythmEngine {
    class MidiGuitarLoader final : public BaseLoader {
        // start, end
        std::pair<int, int>HopoFlags[4] { {66, 65}, {78, 77}, {90, 89}, {102, 101} };

        std::queue<std::pair<int, int> > ForceHopoOn = {};
        std::queue<std::pair<int, int> > ForceHopoOff = {};
        std::queue<std::pair<int, int> > TapMarker = {};
        std::queue<std::pair<int, int> > OpenMarker = {};

        void CheckSysEx(const smf::MidiEvent &event);
        void SysExTap(const smf::MidiEvent &event);
        void SysExOpen(const smf::MidiEvent &event);
        void CheckHopos(const smf::MidiEvent &event);
        void CheckTaps(const smf::MidiEvent &event);
        void CheckModifiers(const smf::MidiEvent &event);
        void CheckEvents(const smf::MidiEvent &event);
        void CreateNote(const smf::MidiEvent &event);
        int GetNoteType(const smf::MidiEvent &event) override;

        void GetChartEvents(smf::MidiEventList track) override;
        void GetNoteModifiers(smf::MidiEventList track) override;
        void GetNotes(smf::MidiEventList track) override;
    public:
        MidiGuitarLoader(int diff_, int thresh_, smf::MidiFile* midiFile_, const int _maxMult = 4)
            : BaseLoader(diff_, thresh_, midiFile_, _maxMult)  {
            chart.Lanes.resize(1);
            Resolution = midiFile_->getTPQ();
        }
    };
}

#endif // GUITARLOADER_H

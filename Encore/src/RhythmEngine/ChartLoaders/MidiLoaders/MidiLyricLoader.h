//
// Created by maria on 25/03/2026.
//

#ifndef ENCORE_LYRICLOADER_H
#define ENCORE_LYRICLOADER_H

#include "midifile/MidiFile.h"

#include <cstdint>

#include "RhythmEngine/events/EncEvents/EncChartEvents.h"

namespace Encore::RhythmEngine {
    class MidiLyricLoader {
        uint32_t CurrentPhrase = 0;
        smf::MidiFile *midiFile;
        int trackIdx;

        void GetPhrases(smf::MidiEventList *midiEventList);
        void IteratePhrases(int tick);
        void GetNotes(smf::MidiEventList *midiEventList);
    public:
        void LoadLyrics();
        std::vector<LyricPhrase> lyrics;

        MidiLyricLoader(smf::MidiFile *midiFile_, int trackIdx_)
            : midiFile(midiFile_), trackIdx(trackIdx_) {
        };

        ~MidiLyricLoader() {
            midiFile = nullptr;
        }

    };
}

#endif //ENCORE_LYRICLOADER_H
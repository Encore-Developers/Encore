//
// Created by maria on 25/03/2026.
//

#ifndef ENCORE_LYRICLOADER_H
#define ENCORE_LYRICLOADER_H
#include "BaseLoader.h"

namespace Encore::RhythmEngine {
    class LyricLoader {
        int CurrentPhrase = 0;
        smf::MidiFile *midiFile;
        int trackIdx;


        void GetPhrases(smf::MidiEventList *midiEventList);
        void IteratePhrases(int tick);
        void GetNotes(smf::MidiEventList *midiEventList);
    public:
        void LoadLyrics();
        std::vector<EncLyricPhrase> lyrics;
        LyricLoader(smf::MidiFile *midiFile_, int trackIdx_)
            : midiFile(midiFile_), trackIdx(trackIdx_) {
        };
    };
}

#endif //ENCORE_LYRICLOADER_H
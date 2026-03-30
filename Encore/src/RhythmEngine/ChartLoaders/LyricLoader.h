//
// Created by maria on 25/03/2026.
//

#ifndef ENCORE_LYRICLOADER_H
#define ENCORE_LYRICLOADER_H
#include "BaseLoader.h"

namespace Encore::RhythmEngine {
    class LyricLoader {
    public:
        void GetLyrics(smf::MidiFile *midiFile);
        std::vector<EncLyricPhrase> lyrics;
        int CurrentPhrase = 0;
        smf::MidiFile *midiFile;
        int trackIdx;

        void GetPhrases(smf::MidiEventList *midiEventList) {
            midiEventList->linkNotePairs();
            for (int eventInt = 0; eventInt < midiEventList->size(); eventInt++) {
                smf::MidiEvent &event = midiEventList->operator[](eventInt);
                if (!lyrics.empty()) {
                    if (lyrics.back().StartTick == event.tick && event.isNoteOn())
                        continue;
                }
                if ((event[1] == 105 || event[1] == 106) && event.isNoteOn()) {
                    EncLyricPhrase phrase;
                    phrase.StartTick = event.tick;
                    phrase.EndTick = event.getLinkedEvent()->tick;
                    phrase.StartSec = event.seconds;
                    phrase.EndSec = event.getLinkedEvent()->seconds;
                    lyrics.emplace_back(phrase);
                }
            }
        };

        void IteratePhrases(int tick) {
            if (!lyrics.empty()) {
                while (CurrentPhrase < lyrics.size() - 1 && lyrics[CurrentPhrase].EndTick < tick) {
                    CurrentPhrase++;
                }
            }
        };

        void GetNotes(smf::MidiEventList *midiEventList) {
            midiEventList->linkNotePairs();
            for (int eventInt = 0; eventInt < midiEventList->size(); eventInt++) {
                if (!midiEventList->operator[](eventInt).isMeta())
                    continue;
                smf::MidiEvent &event = midiEventList->operator[](eventInt);
                if (event[1] != 1)
                    continue;
                IteratePhrases(event.tick);
                std::string lyric = "";

                bool talkie = false;
                for (int k = 3; k < event.getSize(); k++) {
                    lyric += event[k];
                }
                if (lyric.front() == '[')
                    continue;
                switch (lyric.back()) {
                case '%':
                case '+':
                case '$': continue;
                case '*':
                case '#':
                case '^': lyric.pop_back(); talkie = true; break;
                }
                if (lyric.back() == '=' || lyric.back() == '-') {
                    lyric.pop_back();
                } else {
                    lyric.push_back(' ');
                }
                EncoreLog(LOG_DEBUG, lyric.c_str());
                lyrics.at(CurrentPhrase).lyrics.emplace_back(event.seconds, lyric, talkie);
            }
        };

        void LoadLyrics() {
            if (midiFile) {
                GetPhrases(&midiFile->operator[](trackIdx));
                GetNotes(&midiFile->operator[](trackIdx));
            }
        };

        LyricLoader(smf::MidiFile *midiFile_, int trackIdx_)
            : midiFile(midiFile_), trackIdx(trackIdx_) {
        };
    };
}

#endif //ENCORE_LYRICLOADER_H
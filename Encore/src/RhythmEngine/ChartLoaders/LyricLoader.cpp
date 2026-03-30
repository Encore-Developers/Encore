//
// Created by maria on 25/03/2026.
//

#include "LyricLoader.h"

void Encore::RhythmEngine::LyricLoader::LoadLyrics() {
    if (midiFile) {
        midiFile->doTimeAnalysis();
        GetPhrases(&midiFile->operator[](trackIdx));
        GetNotes(&midiFile->operator[](trackIdx));
    }
};

void Encore::RhythmEngine::LyricLoader::GetPhrases(smf::MidiEventList *midiEventList) {
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

void Encore::RhythmEngine::LyricLoader::IteratePhrases(int tick) {
    if (!lyrics.empty()) {
        while (CurrentPhrase < lyrics.size() - 1 && lyrics[CurrentPhrase].EndTick < tick) {
            CurrentPhrase++;
        }
    }
};

void Encore::RhythmEngine::LyricLoader::GetNotes(smf::MidiEventList *midiEventList) {
    midiEventList->linkNotePairs();
    for (int eventInt = 0; eventInt < midiEventList->size(); eventInt++) {
        if (!midiEventList->operator[](eventInt).isMeta())
            continue;
        smf::MidiEvent &event = midiEventList->operator[](eventInt);
        if (!(event[1] == 1 || event[1] == 5))
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
        for (int i = 0; i < lyric.size()-1; i++) {
            static const char* sectionSym = "§";
            if (lyric[i] == sectionSym[0] && lyric[i+1] == sectionSym[1]) {
                lyric.erase(lyric.begin()+i);
                lyric.erase(lyric.begin()+i);
                i--;
            }
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


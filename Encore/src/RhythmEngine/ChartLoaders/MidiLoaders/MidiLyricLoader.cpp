//
// Created by maria on 25/03/2026.
//

#include "MidiLyricLoader.h"
#include "raylib.h"
#include "util/enclog.h"

void Encore::RhythmEngine::MidiLyricLoader::LoadLyrics() {
    Log::Info("Loading lyrics...");
    if (midiFile) {
        midiFile->doTimeAnalysis();
        GetPhrases(&midiFile->operator[](trackIdx));
        GetNotes(&midiFile->operator[](trackIdx));
    }
};

void Encore::RhythmEngine::MidiLyricLoader::GetPhrases(smf::MidiEventList *midiEventList) {
    midiEventList->linkNotePairs();
    EncLyricPhrase fakefirst;
    lyrics.emplace_back(fakefirst);
    for (int eventInt = 0; eventInt < midiEventList->size(); eventInt++) {
        smf::MidiEvent &event = midiEventList->operator[](eventInt);
        if (!lyrics.empty()) {
            if (lyrics.back().StartTick == event.tick && event.isNoteOn())
                continue;
        }
        if ((event[1] == 105 || event[1] == 106) && event.isNoteOn()) {
            if (lyrics.size() == 1) {
                lyrics.front().EndTick = event.tick - midiFile->getTPQ();
                lyrics.front().EndSec = event.seconds - 1;
            }
            EncLyricPhrase phrase;
            phrase.StartTick = event.tick;
            phrase.EndTick = event.getLinkedEvent()->tick;
            phrase.StartSec = event.seconds;
            phrase.EndSec = event.getLinkedEvent()->seconds;
            if (lyrics.back().EndSec < event.seconds - 4) {
                EncLyricPhrase prevPhrase1;
                int EmptySpaceStart = lyrics.back().EndTick + midiFile->getTPQ();
                int EmptySpaceEnd = event.tick - midiFile->getTPQ();
                int EmptySpaceMiddle = EmptySpaceStart + ((EmptySpaceEnd - EmptySpaceStart) / 2);
                prevPhrase1.StartTick = EmptySpaceStart;
                prevPhrase1.EndTick = EmptySpaceMiddle;
                prevPhrase1.StartSec = midiFile->getTimeInSeconds( EmptySpaceStart);
                prevPhrase1.EndSec = midiFile->getTimeInSeconds( EmptySpaceMiddle);
                EncLyricPhrase prevPhrase2;
                prevPhrase2.StartTick = EmptySpaceMiddle;
                prevPhrase2.EndTick = EmptySpaceEnd;
                prevPhrase2.StartSec = midiFile->getTimeInSeconds( EmptySpaceMiddle);
                prevPhrase2.EndSec = midiFile->getTimeInSeconds( EmptySpaceEnd);

                lyrics.emplace_back(prevPhrase1);
                lyrics.emplace_back(prevPhrase2);
            }
            lyrics.emplace_back(phrase);
        }
    }
    EncLyricPhrase last;
    last.StartSec = lyrics.back().EndSec;
    last.StartTick = lyrics.back().EndTick;
    last.EndTick = midiEventList->last().tick;
    last.EndSec = midiEventList->last().seconds;
    lyrics.emplace_back(last);
};

void Encore::RhythmEngine::MidiLyricLoader::IteratePhrases(int tick) {
    if (!lyrics.empty()) {
        while (CurrentPhrase < lyrics.size() - 1
               && lyrics[CurrentPhrase].EndTick < tick) {
            CurrentPhrase++;
        }
    }
};

void Encore::RhythmEngine::MidiLyricLoader::GetNotes(smf::MidiEventList *midiEventList) {
    midiEventList->linkNotePairs();
    for (int eventInt = 0; eventInt < midiEventList->size(); eventInt++) {
        if (!(*midiEventList)[eventInt].isMeta())
            continue;
        smf::MidiEvent &event = (*midiEventList)[eventInt];
        if (!(event[1] == 1 || event[1] == 5))
            continue;
        IteratePhrases(event.tick);
        if (lyrics.empty()) break;
        //     Encore::EncoreLog(
        //         LOG_DEBUG, "chart has at least one lyricless vox note. report to charter"
        //     );
        //     continue;
        std::string lyric = "";

        bool talkie = false;
        auto &currentPhrase = lyrics.at(CurrentPhrase);
        for (int k = 3; k < event.getSize(); k++) {
            lyric += event[k];
        }

        if (lyric == "+-") {
            if (!currentPhrase.lyrics.empty()) {
                if (currentPhrase.lyrics.back().Lyric.back() == ' ') {
                    currentPhrase.lyrics.back().Lyric.pop_back();
                }
            }
            continue;
        }
        if (lyric.front() == '[')
            continue;

        switch (lyric.back()) {
        case '+':
        case '$':
            continue;
        case '*':
        case '#':
        case '^':
            lyric.pop_back();
            talkie = true;
            break;
        }
        for (size_t i = 0; i < lyric.size() - 1; i++) {
            static const char *sectionSym = "§";
            if (lyric[i] == sectionSym[0] && lyric[i + 1] == sectionSym[1]) {
                lyric.erase(lyric.begin() + i);
                lyric.erase(lyric.begin() + i);
                i--;
            }
        }
        if (lyric.back() == '-' || lyric.back() == '%') {
            lyric.pop_back();
        } else if (lyric.back() == '=') {
            lyric.back() = '-';
        } else {
            lyric.push_back(' ');
        }
        currentPhrase.lyrics.emplace_back(event.seconds, lyric, talkie);
    }
};

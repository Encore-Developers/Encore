//
// Created by maria on 25/03/2026.
//

#include "MidiLyricLoader.h"
#include "raylib.h"
#include "RhythmEngine/events/EncEvents/EncChartEvents.h"
#include "util/enclog.h"

void Encore::RhythmEngine::MidiLyricLoader::LoadLyrics() {
    Log::Info("Loading lyrics...");
    if (midiFile) {
        GetPhrases(&midiFile->operator[](trackIdx));
        GetNotes(&midiFile->operator[](trackIdx));
    }
}

void Encore::RhythmEngine::MidiLyricLoader::GetPhrases(smf::MidiEventList *midiEventList) {
    LyricPhrase fakefirst{0,0,0,0};
    lyrics.emplace_back(fakefirst);
    for (int eventInt = 0; eventInt < midiEventList->size(); eventInt++) {
        smf::MidiEvent &event = midiEventList->operator[](eventInt);
        if (!lyrics.empty()) {
            if (lyrics.back().start.tick == event.tick && event.isNoteOn())
                continue;
        }
        if ((event[1] == 105 || event[1] == 106) && event.isNoteOn()) {
            if (lyrics.size() == 1) {
                lyrics.front().end.tick = event.tick - midiFile->getTPQ();
                lyrics.front().end.sec = midiFile->getTimeInSeconds(lyrics.front().end.tick);
            }
            LyricPhrase phrase{event.seconds, event.tick, event.getLinkedEvent()->seconds, event.getLinkedEvent()->tick};
            if (lyrics.back().end.sec < event.seconds - 4) {
                int EmptySpaceStart = lyrics.back().end.tick + midiFile->getTPQ();
                int EmptySpaceEnd = event.tick - midiFile->getTPQ();
                int EmptySpaceMiddle = EmptySpaceStart + ((EmptySpaceEnd - EmptySpaceStart) / 2);

                TimePoint start{midiFile->getTimeInSeconds(EmptySpaceStart), EmptySpaceStart};
                TimePoint middle{midiFile->getTimeInSeconds(EmptySpaceMiddle), EmptySpaceMiddle};
                TimePoint end{midiFile->getTimeInSeconds(EmptySpaceEnd), EmptySpaceEnd};
                LyricPhrase prevPhrase1{start, middle};
                LyricPhrase prevPhrase2{middle, end};

                lyrics.emplace_back(prevPhrase1);
                lyrics.emplace_back(prevPhrase2);
            }
            lyrics.emplace_back(phrase);
        }
    }
    LyricPhrase last{lyrics.back().end.sec, lyrics.back().end.tick,
        midiEventList->last().seconds, midiEventList->last().tick};
    lyrics.emplace_back(last);
};

void Encore::RhythmEngine::MidiLyricLoader::IteratePhrases(int tick) {
    if (!lyrics.empty()) {
        while (CurrentPhrase < lyrics.size() - 1
               && lyrics[CurrentPhrase].end.tick < tick) {
            CurrentPhrase++;
        }
    }
};

void Encore::RhythmEngine::MidiLyricLoader::GetNotes(smf::MidiEventList *midiEventList) {
    for (int eventInt = 0; eventInt < midiEventList->size(); eventInt++) {
        if (!(*midiEventList)[eventInt].isMeta())
            continue;
        smf::MidiEvent &event = (*midiEventList)[eventInt];
        if (!(event[1] == 1 || event[1] == 5))
            continue;
        TimePoint tp {event.seconds, event.tick};
        IteratePhrases(event.tick);
        if (lyrics.empty()) break;
        //     Encore::EncoreLog(
        //         LOG_DEBUG, "chart has at least one lyricless vox note. report to charter"
        //     );
        //     continue;
        std::string syllable = "";

        bool talkie = false;
        auto &currentPhrase = lyrics.at(CurrentPhrase);
        for (int k = 3; k < event.getSize(); k++) {
            syllable += event[k];
        }

        if (syllable == "+-") {
            if (!currentPhrase.syllables.empty()) {
                if (currentPhrase.syllables.back().syllable.back() == ' ') {
                    currentPhrase.syllables.back().syllable.pop_back();
                }
            }
            continue;
        }
        if (syllable.front() == '[')
            continue;

        switch (syllable.back()) {
        case '+':
        case '$':
            continue;
        case '*':
        case '#':
        case '^':
            syllable.pop_back();
            talkie = true;
            break;
        }
        for (size_t i = 0; i < syllable.size() - 1; i++) {
            static const char *sectionSym = "§";
            if (syllable[i] == sectionSym[0] && syllable[i + 1] == sectionSym[1]) {
                syllable.erase(syllable.begin() + i);
                syllable.erase(syllable.begin() + i);
                i--;
            }
        }
        if (syllable.back() == '-' || syllable.back() == '%') {
            syllable.pop_back();
        } else if (syllable.back() == '=') {
            syllable.back() = '-';
        } else {
            syllable.push_back(' ');
        }
        currentPhrase.syllables.emplace_back(tp, syllable, talkie);
    }
};

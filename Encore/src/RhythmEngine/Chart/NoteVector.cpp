#include "NoteVector.h"

using namespace Encore::RhythmEngine;

void BaseChart::resize(const size_t _size) {
    size = _size;
    Lanes.resize(_size);
    CurrentNoteIterators.resize(_size);
}
bool BaseChart::AdvanceCurrentNote(const size_t lane) {
    // if (std::distance(Lanes.at(lane).begin(), CurrentNoteIterators.at(lane))
    //     >= Lanes.at(lane).size()) {
    //     // bounds check
    //     return false;
    // }
    if (CurrentNoteIterators.at(lane) == Lanes.at(lane).end())
        return false;
    ++CurrentNoteIterators.at(lane);
    return true;
}
void BaseChart::Reset() {
    for (size_t i = 0; i < CurrentNoteIterators.size(); i++) {
        CurrentNoteIterators.at(i) = Lanes.at(i).begin();
    }
}
void BaseChart::UpdateSections(const int tick) {
    if (!sections.empty()) {
        while (CurrentSection < sections.size() - 1) {
            if (tick < sections[CurrentSection + 1].tickStart) {
                break;
            }
            CurrentSection++;
        }
    }
}
void BaseChart::RecountPhrases() {
    if (!overdrive.empty()) {
        for (auto& phrase : overdrive) {
            phrase.NoteCount = 0;
        }
        for (auto& lane : Lanes) {
            auto curPhrase = overdrive.begin();
            for (auto& note : lane) {
                if (note.start.tick >= curPhrase->end.tick) {
                    ++curPhrase;
                }
                if (curPhrase == overdrive.end()) {
                    break;
                }
                if (note.start.tick >= curPhrase->start.tick) {
                    curPhrase->NoteCount++;
                }
            }
        }
    }
    if (!solos.empty()) {
        for (auto& phrase : solos) {
            phrase.NoteCount = 0;
        }
        for (auto& lane : Lanes) {
            auto curPhrase = solos.begin();
            for (auto& note : lane) {
                if (note.start.tick >= curPhrase->end.tick) {
                    ++curPhrase;
                }
                if (curPhrase == solos.end()) {
                    break;
                }
                if (note.start.tick >= curPhrase->start.tick) {
                    curPhrase->NoteCount++;
                }
            }
        }
    }

}
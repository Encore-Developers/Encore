//
// Created by maria on 01/06/2025.
//

#include "BaseStats.h"

template <size_t LaneCount>
void Encore::RhythmEngine::BaseStats<LaneCount>::HitNote(int chordSize) {
    Combo++;
    Score = (25 * chordSize) * multiplier();
    // PerfectHits = 0;
    NotesHit++;
    AttemptedNotes++;
    AudioMuted = false;
}

template <size_t LaneCount>
void Encore::RhythmEngine::BaseStats<LaneCount>::Overhit() {
    Combo = 0;
    AudioMuted = true;
}

template <size_t LaneCount>
int Encore::RhythmEngine::BaseStats<LaneCount>::multiplier() const {
    int od = OverdriveActive ? 2 : 1;
    // if (IsBassOrVox()) {
    //     if (Combo >= 50)
    //         return 6 * od;

    //} else {
    if (Combo >= 30)
        return 4 * od;
    //};
    return (Combo / 10) + 1 * od;
}

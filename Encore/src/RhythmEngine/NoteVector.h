//
// Created by maria on 17/05/2025.
//

#ifndef NOTEVECTOR_H
#define NOTEVECTOR_H
#include "REenums.h"
#include "Notes/EncNote.h"
#include "song/events/EncEventVects/EventVectors.h"

#include <array>
#include <queue>
#include <vector>

inline std::vector<std::vector<int> > pDiffNotes = { { 60, 61, 62, 63, 64 },
                                                     { 72, 73, 74, 75, 76 },
                                                     { 84, 85, 86, 87, 88 },
                                                     { 96, 97, 98, 99, 100 } };

inline std::vector<std::pair<int, int> > MinMaxDiff = {
    { 60, 64 }, { 72, 76 }, { 84, 88 }, { 96, 100 }
};

inline std::vector<std::pair<int, int> > LiftMinMaxDiff = {
    { 66, 69 }, { 78, 81 }, { 90, 93 }, { 102, 106 }
};

namespace Encore::RhythmEngine {
    // half of this shit is just to make nicer code
    // although i do run the issue of "i want to be able to reuse code for this lmao"
    // shouldnt be that bad once i get to the gritty part of the code
    // but holy fuck ive come so far

    template <typename NoteType>
    class NoteVector : public std::vector<NoteType> {
    public:
        NoteType &operator[](int i) { return this->at(i); }
    };

    template <typename NoteType, int size>
    class BaseChart : public std::array<NoteVector<NoteType>, size> {
    public:
        virtual ~BaseChart() = default;
        NoteVector<NoteType> &operator[](int i) { return this->at(i); }
        SoloEvents solos;
        ODEvents overdrive;
        SectionEvents sections;
    };

    class GuitarChart : public BaseChart<EncNote, 5> {};

    class PadChart : public BaseChart<EncNote, 5> {};
}

#endif // NOTEVECTOR_H

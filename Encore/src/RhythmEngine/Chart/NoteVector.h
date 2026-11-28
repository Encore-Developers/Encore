//
// Created by maria on 17/05/2025.
//

#ifndef NOTEVECTOR_H
#define NOTEVECTOR_H

#include "RhythmEngine/events/EncEventVects/EventVectors.h"
#include "../timingvalues.h"
#include <deque>
#include <vector>

#include "gameplay/enctime.h"

inline std::vector<std::vector<int> > pDiffNotes = { { 60, 61, 62, 63, 64 },
                                                     { 72, 73, 74, 75, 76 },
                                                     { 84, 85, 86, 87, 88 },
                                                     { 96, 97, 98, 99, 100 } };

inline std::vector<std::pair<int, int> > MinMaxDiff = {
    { 60, 64 }, { 72, 76 }, { 84, 88 }, { 96, 100 }
};

inline std::vector<std::pair<int, int> > GuitarMinMaxDiff = {
    { 59, 64 }, { 71, 76 }, { 83, 88 }, { 95, 100 }
};

inline std::vector<std::pair<int, int> > LiftMinMaxDiff = {
    { 66, 69 }, { 78, 81 }, { 90, 93 }, { 102, 106 }
};

namespace Encore::RhythmEngine {
    // half of this shit is just to make nicer code
    // although i do run the issue of "i want to be able to reuse code for this lmao"
    // shouldnt be that bad once i get to the gritty part of the code
    // but holy fuck ive come so far

    class NoteVector : public std::vector<NoteEvent> {
    public:
        NoteEvent &operator[](const int i) { return this->at(i); }
    };

    class BaseChart {
    public:
        explicit BaseChart(const size_t _size) : size(_size) {
            Lanes.resize(size);
            CurrentNoteIterators.resize(size);
        };
        BaseChart() : BaseChart(5) {};
        uint8_t size;
        NoteVector &operator[](const size_t i) { return this->Lanes.at(i); }
        NoteVector &at(const size_t i) { return this->Lanes.at(i); }
        void resize(size_t _size);
        std::vector<NoteVector> Lanes;
        std::vector<NoteVector::iterator> CurrentNoteIterators;
        std::vector<NoteEvent *> MissedNotePointers;
        std::vector<NoteEvent *> DroppedSustainPointers;
        /**
         * this DOES NOT CARE about timing or ANYTHING.
         * it simply just gets the next note in said lane
         */
        bool AdvanceCurrentNote(size_t lane);

        /**
         * reset notes
         */
        void Reset();

        void UpdateSections(int tick);

        void RecountPhrases();

        size_t CurrentSection = 0;
        std::vector<Section> sections;
        SoloEvents solos;
        ODEvents overdrive;
        TrillEvents trills;
        RollEvents rolls;
    };
}

#endif // NOTEVECTOR_H

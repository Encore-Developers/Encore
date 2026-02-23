//
// Created by maria on 17/05/2025.
//

#ifndef NOTEVECTOR_H
#define NOTEVECTOR_H
#include "REenums.h"
#include "Notes/EncNote.h"
#include "song/events/EncEventVects/EventVectors.h"

#include <array>
#include <memory>
#include <queue>
#include <vector>

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

    class NoteVector : public std::deque<EncNote> {
    public:
        EncNote &operator[](const int i) { return this->at(i); }
    };

    class BaseChart {
    public:
        explicit BaseChart(int _size) : size(_size) {
            Lanes.resize(_size);
            CurrentNoteIterators.resize(_size);
            HeldNotePointers.resize(_size);
        };
        BaseChart() : size(5) {
            Lanes.resize(5);
            CurrentNoteIterators.resize(5);
            HeldNotePointers.resize(5);
        };
        unsigned char size;
        NoteVector &operator[](const int i) { return this->Lanes.at(i); }
        NoteVector &at(const int i) { return this->Lanes.at(i); }
        std::vector<NoteVector> Lanes;
        std::vector<NoteVector::iterator> CurrentNoteIterators;
        // todo: please fix extended sustains, i really dont wanna do this right now
        // idea: use array instead of singular note
        std::vector<EncNote*> HeldNotePointers;
        std::vector<EncNote *> MissedNotePointers;
        double BaseScore = 0;
        /**
         * this DOES NOT CARE about timing or ANYTHING.
         * it simply just gets the next note in said lane
         */
        bool UpdateCurrentNote(int lane) {
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

        /**
         * sustains
         */
        void SetCurrentNoteAsHeldNote(int lane) {
            HeldNotePointers.at(lane) = &*CurrentNoteIterators.at(lane);
        }

        bool IsHeldNotePresent(int lane) {
            return HeldNotePointers.at(lane);
        }

        /**
         * reset notes
         */
        void Reset(int lane) {
            for (int i = 0; i < CurrentNoteIterators.size(); i++) {
                CurrentNoteIterators.at(i) = Lanes.at(i).begin();
            }
            for (int i = 0; i < HeldNotePointers.size(); i++) {
                HeldNotePointers.at(i) = nullptr;
            }
        }

        SoloEvents solos;
        ODEvents overdrive;
        SectionEvents sections;
        TrillEvents trills;
        RollEvents rolls;
    };
}

#endif // NOTEVECTOR_H

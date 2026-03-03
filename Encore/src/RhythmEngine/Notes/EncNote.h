//
// Created by maria on 06/05/2025.
//

#ifndef ENCNOTE_H
#define ENCNOTE_H
#include <cstdint>

namespace Encore::RhythmEngine {
    class EncNote {
    public:
        int StartTicks = 0;
        int LengthTicks = 0;
        double StartSeconds = 0;
        double LengthSeconds = 0;
        int NoteType = 0;
        uint8_t Lane = 0;
        bool NotePassed = false;
        bool operator==(const EncNote &value) const {
            if (&value == this)
                return true;
            return false;
        };

        bool operator!=(const EncNote &value) const {
            if (&value == this)
                return false;
            return true;
        };
    };
}

#endif // ENCNOTE_H

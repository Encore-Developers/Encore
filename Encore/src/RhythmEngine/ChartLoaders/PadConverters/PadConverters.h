#pragma once
#include "../../Chart/NoteVector.h"
#include "RhythmEngine/REenums.h"

#include <bit>

namespace Encore::RhythmEngine::PadConverters {
    enum NoteHand {
        CHORD,
        LEFT,
        RIGHT
    };

    NoteHand GetLaneHand(uint8_t note);

    NoteHand GetNoteHand(EncNote& note);

    BaseChart ConvertGuitarToPad(BaseChart& sourceChart);
}


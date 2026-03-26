//
// Created by maria on 06/05/2025.
//

#ifndef ENCNOTE_H
#define ENCNOTE_H
#include <cstdint>
#include <string>
#include <vector>

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
    };

    // for lyric display
    struct EncLyric {
        double StartSec = 0;
        std::string Lyric;
        bool talkie = false;
    };

    struct EncLyricPhrase {
        std::vector<EncLyric> lyrics;
        double StartSec = 0;
        double EndSec = 0;
    };
}

#endif // ENCNOTE_H

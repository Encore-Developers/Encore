//
// Created by maria on 01/04/2025.
//

#ifndef RHYTHMENGINE_H
#define RHYTHMENGINE_H
#include "song/notes/EncNote.h"

#include <queue>

class RhythmEngine {
    public:
        void Update();
        std::queue<Note> notes;

};



#endif //RHYTHMENGINE_H

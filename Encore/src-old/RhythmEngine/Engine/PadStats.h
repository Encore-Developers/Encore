//
// Created by maria on 13/06/2025.
//

#ifndef PADSTATS_H
#define PADSTATS_H
#include "BaseStats.h"


namespace Encore::RhythmEngine {
    class PadStats final : public BaseStats<5> {
        int Type = 0;
    public:
        explicit PadStats(int BaseScore) : BaseStats<5>(BaseScore) {}
    };
}



#endif //PADSTATS_H

//
// Created by maria on 01/06/2025.
//

#ifndef GUITARSTATS_H
#define GUITARSTATS_H
#include "../BaseStats.h"

namespace Encore::RhythmEngine {
    class GuitarStats final : public BaseStats {
    public:
        int Type = 0;
        explicit GuitarStats(int BaseScore) : BaseStats(BaseScore) {}
        [[nodiscard]] uint8_t HeldFretsArrayToMask() const;
    };
}

#endif // GUITARSTATS_H
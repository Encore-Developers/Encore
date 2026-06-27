//
// Created by maria on 13/06/2025.
//

#ifndef DRUMSSTATS_H
#define DRUMSSTATS_H
#include "../BaseStats.h"


namespace Encore::RhythmEngine {
    class DrumsStats final : public BaseStats {
        int Type = 0;
    public:
        explicit DrumsStats(int BaseScore) : BaseStats(BaseScore) {}
    };
}



#endif // DRUMSSTATS_H

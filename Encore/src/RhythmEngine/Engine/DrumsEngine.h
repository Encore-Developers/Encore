//
// Created by maria on 13/06/2025.
//

#ifndef DRUMSENGINE_H
#define DRUMSENGINE_H
#include "BaseEngine.h"
#include "DrumsStats.h"

namespace Encore::RhythmEngine {
    class DrumsEngine : public BaseEngine {
        bool ActivateOverdrive(InputChannel channel, Action action) override;

        /*
         *
         *
         */
        int RunHitStateCheck(InputChannel channel, Action action) override;
        bool PlayerIsPaused() override { return stats->Paused; };
        void TogglePause() override { stats->Paused = !stats->Paused; };

    public:
        void UpdateOnFrame(double CurrentTime) override;
        void SetStatsInputState(InputChannel channel, Action action) override;
        std::shared_ptr<BaseChart> chart;
        std::shared_ptr<DrumsStats> stats;
        DrumsEngine(auto _chart, auto _stats)
            : BaseEngine(_chart, _stats), chart(_chart), stats(_stats) {};
        ~DrumsEngine() override {};
    };
};

#endif // DRUMSENGINE_H

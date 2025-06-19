//
// Created by maria on 13/06/2025.
//

#ifndef PADENGINE_H
#define PADENGINE_H
#include "BaseEngine.h"
#include "PadStats.h"

namespace Encore::RhythmEngine {
    class PadEngine : public BaseEngine {
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
        void HitNote() override;
        void Overhit() override;
        std::shared_ptr<PadChart> chart;
        std::shared_ptr<PadStats> stats;
        PadEngine(auto _chart, auto _stats)
            : BaseEngine(_chart, _stats), chart(_chart), stats(_stats) {};
        ~PadEngine() override {};
    };
};



#endif //PADENGINE_H

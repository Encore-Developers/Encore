//
// Created by maria on 13/06/2025.
//

#ifndef DRUMSENGINE_H
#define DRUMSENGINE_H
#include "BaseEngine.h"
#include "DrumsStats.h"

namespace Encore::RhythmEngine {
    class DrumsEngine : public BaseEngine {
        bool ActivateOverdrive(ControllerEvent &event) override;

        /*
         *
         *
         */
        int RunHitStateCheck(ControllerEvent &event) override;
        bool PlayerIsPaused() override { return stats->Paused; };
        void TogglePause() override { stats->Paused = !stats->Paused; };

    public:
        void HitNote(int lane);
        void UpdateOnFrame(double CurrentTime) override;
        void SetStatsInputState(ControllerEvent &event) override;
        std::shared_ptr<BaseChart> chart;
        std::shared_ptr<DrumsStats> stats;
        DrumsEngine(auto _chart, auto _stats)
            : BaseEngine(_chart, _stats), chart(_chart), stats(_stats) {
            Timers = { {"debounce", RhythmTimer(0.01)} };
        };
        ~DrumsEngine() override {};
    };
};

#endif // DRUMSENGINE_H

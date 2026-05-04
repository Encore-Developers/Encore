//
// Created by maria on 13/06/2025.
//

#ifndef PADENGINE_H
#define PADENGINE_H
#include "BaseEngine.h"
#include "PadStats.h"

namespace Encore::RhythmEngine {
    class PadEngine : public BaseEngine {
        bool ActivateOverdrive(ControllerEvent &event) override;

        /*
         *
         *
         */
        int RunHitStateCheck(ControllerEvent &event) override;
        bool PlayerIsPaused() override { return stats->Paused; };
        void TogglePause() override { stats->Paused = !stats->Paused; };
        void HitNote(int lane);
    public:
        void UpdateOnFrame(double CurrentTime);
        void SetStatsInputState(ControllerEvent &event) override;
        std::shared_ptr<BaseChart> chart;
        std::shared_ptr<PadStats> stats;
        Player* player;
        PadEngine(auto _chart, auto _stats, Player* _player)
            : BaseEngine(_chart, _stats, _player), chart(_chart), stats(_stats), player(_player) {
            Timers = { { "LOP", RhythmTimer(liftLeniencyTime) } };
        };
        ~PadEngine() override {};
    };
};



#endif //PADENGINE_H

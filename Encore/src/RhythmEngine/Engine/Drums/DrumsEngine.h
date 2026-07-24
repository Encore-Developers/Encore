#pragma once
//
// Created by maria on 13/06/2025.
//

#include "../BaseEngine.h"
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
        void HitNote(size_t lane) override;
        void UpdateOnFrame(double CurrentTime) override;
        void SetStatsInputState(ControllerEvent &event) override;
        TimePoint NextNoteTime() override;
        TimePoint LastNoteTime() override;
        std::shared_ptr<BaseChart> chart;
        std::shared_ptr<DrumsStats> stats;
        Player* player;
        DrumsEngine(auto _chart, auto _stats, Player* _player)
            : BaseEngine(_chart, _stats, _player), chart(_chart), stats(_stats), player(_player) {
                Timers = {{"debounce", RhythmTimer(0.01)}};
                if (chart->size == 6) {
                    stats->HeldFrets.resize(6, false);
                };

        };
        ~DrumsEngine() override = default;
    };
};
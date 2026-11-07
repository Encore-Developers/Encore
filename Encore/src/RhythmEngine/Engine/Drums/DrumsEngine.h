#pragma once
//
// Created by maria on 13/06/2025.
//

#include "../BaseEngine.h"

namespace Encore::RhythmEngine {
    class DrumsEngine : public BaseEngine {
        void RunHitStateCheck(ControllerEvent &event) override;
    public:
        void HitNote(size_t lane) override;
        void Advance(double CurrentTime) override;
        DrumsEngine(auto _chart, Player* _player)
            : BaseEngine(_chart, _player) {
                Timers = {{"debounce", RhythmTimer(0.01)}};
                if (chart->size == 6) {
                    stats->HeldFrets.resize(6, false);
                };

        };
        ~DrumsEngine() override = default;
    };
};
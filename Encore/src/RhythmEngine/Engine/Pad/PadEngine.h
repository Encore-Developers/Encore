#pragma once
//
// Created by maria on 13/06/2025.
//

#include "../BaseEngine.h"

namespace Encore::RhythmEngine {
    class PadEngine : public BaseEngine {

        bool OverdriveLiftInput = false;

        void RunHitStateCheck(ControllerEvent &event) override;
        void LaneHitStateCheck(int lane, ControllerEvent &event);
        void TogglePause() override { stats->Paused = !stats->Paused; };
    public:
        void Advance(double CurrentTime) override;
        void SetStatsInputState(ControllerEvent &event) override;
        void OnOverdriveActivate(TimeSeconds time) override;
        PadEngine(auto _chart, Player* _player)
            : BaseEngine(_chart, _player) {
            Timers = { { "LOP", RhythmTimer(liftLeniencyTime) } };
        };
        ~PadEngine() override = default;
    };
};


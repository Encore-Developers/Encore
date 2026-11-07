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
    public:
        void Advance(double CurrentTime) override;
        void OnOverdriveActivate(TimeSeconds time) override;
        PadEngine(auto _chart, Player* _player)
            : BaseEngine(_chart, _player) {
            Timers = { { "LOP", RhythmTimer(liftLeniencyTime) } };
        };
        ~PadEngine() override = default;
    };
};


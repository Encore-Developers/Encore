#pragma once
#include <functional>
#include <string>
#include <utility>

#include "RhythmEngine/REenums.h"

namespace Encore {
    class ButtonAction
    {
        std::function<void(RhythmEngine::Action action, int slot)> Action;

        public:
        bool barVisible = true;
        std::string Name;
        ButtonAction(const std::string &name, std::function<void(RhythmEngine::Action action, int slot)> _action) {
            Name = name;
            Action = std::move(_action);
        };
        ButtonAction(const std::string &name, std::function<void(RhythmEngine::Action action, int slot)> _action, bool _barVisible) {
            Name = name;
            Action = std::move(_action);
            barVisible = _barVisible;
        };
        void RunAction(RhythmEngine::Action _action, int slot) {
            Action(_action, slot);
        }
    };
}

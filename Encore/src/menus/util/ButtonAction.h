#pragma once
#include <functional>
#include <string>
#include <utility>

#include "RhythmEngine/REenums.h"
#include "util/Input.h"

namespace Encore {
    class ButtonAction
    {
        std::function<void(Action action, int slot)> action;

        public:
        bool barVisible = true;
        std::string Name;
        ButtonAction(const std::string &name, std::function<void(Action action, int slot)> _action) {
            Name = name;
            action = std::move(_action);
        };
        ButtonAction(const std::string &name, std::function<void(Action action, int slot)> _action, bool _barVisible) {
            Name = name;
            action = std::move(_action);
            barVisible = _barVisible;
        };
        void RunAction(Action _action, int slot) {
            action(_action, slot);
        }
    };
}

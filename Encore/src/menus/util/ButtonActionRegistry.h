#pragma once
#include "ButtonAction.h"
#include "users/playerManager.h"

#include <map>

namespace Encore {
    class ButtonActionRegistry
    {
    public:
        std::map<InputChannel, ButtonAction> buttMap;

        void HandleInput(const ControllerEvent &event);

        // run after drawing GameMenu::BottomOvershell()
        void DrawPrompts(bool OvershellOpen, float top = -1, float left = -1);
    };
}

// write lambdas yourself
#define NEWBUTTONACTION(reg, lane, name, ...) reg.buttMap.emplace(Encore::InputChannel::lane, Encore::ButtonAction{name, __VA_ARGS__});

// autofills most lambda information, _action for action, slot for slot int
#define NEWBUTTONACTION2(reg, lane, name, ...) reg.buttMap.emplace(Encore::InputChannel::lane, Encore::ButtonAction{name, [this](Encore::Action _action, int slot)__VA_ARGS__});
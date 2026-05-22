#pragma once
#include "ButtonAction.h"
#include "users/playerManager.h"

#include <map>

namespace Encore {
    class ButtonActionRegistry
    {
    public:
        std::map<RhythmEngine::InputChannel, ButtonAction> buttMap;

        void CallbackAction(const RhythmEngine::ControllerEvent &event) {
            int curSlot = 0;
            if (ThePlayerManager.GetPlayerForJoystick(event.slot)) {
                curSlot = ThePlayerManager.GetPlayerForJoystick(event.slot)->ActiveSlot;
            }
            if (buttMap.contains(event.channel)) {
                buttMap.at(event.channel).RunAction(event.action, curSlot);
            }
        }

        // run after drawing GameMenu::BottomOvershell()
        void DrawPrompts(bool OvershellOpen, float top = -1, float left = -1);
    };
}

// write lambdas yourself
#define NEWBUTTONACTION(reg, lane, name, ...) reg.buttMap.emplace(Encore::RhythmEngine::InputChannel::lane, Encore::ButtonAction{name, __VA_ARGS__});

// autofills most lambda information
#define NEWBUTTONACTION2(reg, lane, name, ...) reg.buttMap.emplace(Encore::RhythmEngine::InputChannel::lane, Encore::ButtonAction{name, [this](Encore::RhythmEngine::Action _action, int slot)__VA_ARGS__});
#pragma once
#include "ButtonAction.h"

namespace Encore {
    class ButtonActionRegistry
    {
    public:
        std::unordered_map<RhythmEngine::InputChannel, ButtonAction> buttMap;

        void CallbackAction(RhythmEngine::InputChannel channel, RhythmEngine::Action action, int slot) {
            if (buttMap.contains(channel)) {
                buttMap.at(channel).RunAction(action, slot);
            }
        }

        // run after drawing GameMenu::BottomOvershell()
        void DrawPrompts(bool OvershellOpen, float top = -1, float left = -1);
    };
}

#define NEWBUTTONACTION(reg, lane, name, ...) reg.buttMap.emplace(Encore::RhythmEngine::InputChannel::lane, Encore::ButtonAction{name, __VA_ARGS__});
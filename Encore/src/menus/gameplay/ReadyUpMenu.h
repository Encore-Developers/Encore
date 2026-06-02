//
// Created by marie on 16/11/2024.
//

#ifndef READYUPMENU_H
#define READYUPMENU_H
#include "../overshell/OvershellMenu.h"
#include "menus/util/ButtonActionRegistry.h"
#include "users/player.h"

class ReadyUpMenu : public OvershellMenu {
    enum ReadyUpMenuState {
        INSTRUMENT,
        DIFFICULTY,
        READY
    };

    std::array<ReadyUpMenuState, MAX_PLAYERS> SlotState{ INSTRUMENT };
    std::array<bool, MAX_PLAYERS> ReadyState{ false };
    std::array<uint8_t, MAX_PLAYERS> ControllerDiffSlot{ 0 };
    std::array<uint8_t, MAX_PLAYERS> ControllerInstSlot{ 0 };
    Encore::ButtonActionRegistry buttReg;
    std::vector<int> PartsToDisplay = {};
public:
    ReadyUpMenu() = default;
    ~ReadyUpMenu() override = default;
    void KeyboardInputCallback(SDL_KeyboardEvent* event) override;
    void DrawDifficulties(float BottomOvershell,
                 int playerInt,
                 Player &player,
                 float xPosOfMenu);
    void ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event) override;
    void Draw() override;
    void Load() override;
};



#endif //READYUPMENU_H

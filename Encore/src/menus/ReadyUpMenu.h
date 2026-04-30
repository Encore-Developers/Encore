//
// Created by marie on 16/11/2024.
//

#ifndef READYUPMENU_H
#define READYUPMENU_H
#include "OvershellMenu.h"
#include "users/player.h"

class ReadyUpMenu : public OvershellMenu {
    enum ReadyUpMenuState {
        INSTRUMENT,
        DIFFICULTY,
        READY
    };

    std::array<ReadyUpMenuState, 4> SlotState{ INSTRUMENT, INSTRUMENT, INSTRUMENT, INSTRUMENT };
    std::array<bool, 4> ReadyState{ false, false, false, false };
    std::array<int, 4> ControllerDiffSlot{ 0, 0, 0, 0 };
    std::array<int, 4> ControllerInstSlot{ 0, 0, 0, 0 };

    std::array<std::vector<int>, 4> PartsToDisplay = {};
public:
    ReadyUpMenu() = default;
    ~ReadyUpMenu() override = default;
    void KeyboardInputCallback(int key, int scancode, int action, int mods) override;
    void DrawDifficulties(float BottomOvershell,
                 int playerInt,
                 Player &player,
                 float xPosOfMenu);
    void ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event) override;
    void Draw() override;
    void Load() override;
};



#endif //READYUPMENU_H

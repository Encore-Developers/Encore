//
// Created by marie on 16/11/2024.
//

#ifndef READYUPMENU_H
#define READYUPMENU_H
#include "../overshell/OvershellMenu.h"
#include "menus/util/ButtonActionRegistry.h"
#include "RhythmEngine/ChartLoaders/ChartLoader.h"
#include "menus/settings/util/SimpleMenu/SimpleMenu.h"
#include "users/player.h"

class ReadyUpMenu;

class PartOption : public SimpleMenu::Option {
public:
    ReadyUpMenu* menu;
    int slot;
    int part;

    PartOption(SimpleMenu::Instance*, ReadyUpMenu* menu, int slot, int part);

    void Draw(Encore::TextDisplay display, size_t index, SimpleMenu::MouseState mouseState) override;
    bool Input(Encore::ControllerEvent event) override;
};

class ReadyUpMenu : public OvershellMenu {
public:
    enum ReadyUpMenuState {
        INSTRUMENT,
        DIFFICULTY,
        READY
    };

    std::array<ReadyUpMenuState, MAX_PLAYERS> SlotState{ INSTRUMENT };
    std::array<SimpleMenu::Instance, MAX_PLAYERS> Menus{};
    std::array<uint8_t, MAX_PLAYERS> ControllerDiffSlot{ 0 };
    std::array<uint8_t, MAX_PLAYERS> ControllerInstSlot{ 0 };
    /// Functions to be ran at the end of the Draw
    std::deque<std::function<void(ReadyUpMenu*)>> defer;
    Encore::ButtonActionRegistry buttReg;
    std::vector<int> PartsToDisplay = {};
    void Back(int slot);

    Song* curSong;
    Encore::RhythmEngine::ChartLoader chartLoader;
    ReadyUpMenu(Song* song)
        : curSong(song), chartLoader(song) {}

    ~ReadyUpMenu() override = default;
    void KeyboardInputCallback(SDL_KeyboardEvent* event) override;
    void ControllerInputCallback(Encore::ControllerEvent event) override;
    void Draw() override;
    void Load() override;
    void LoadMenuState(int slot, ReadyUpMenuState state);

    bool AllowsTempPlayers() override { return true; };
};



#endif //READYUPMENU_H

//
// Created by maria on 29/12/2025.
//

#ifndef ENCORE_DEVELOPERS_KEYBINDS_H
#define ENCORE_DEVELOPERS_KEYBINDS_H
#include "SDL3/SDL_keycode.h"
#include <array>
#include <string>
#include <nlohmann/json.hpp>

namespace Encore {
    class Keybinds
    {
    public:
        std::array<SDL_Keycode, 4> keybinds4k = {SDLK_D, SDLK_F, SDLK_J, SDLK_K};
        std::array<SDL_Keycode, 5> keybinds5k = {SDLK_D, SDLK_F, SDLK_J, SDLK_K, SDLK_L};
        std::array<SDL_Keycode, 4> keybinds4kalt = {SDLK_1, SDLK_2, SDLK_3, SDLK_4};
        std::array<SDL_Keycode, 5> keybinds5kalt = {SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5};
        std::pair<SDL_Keycode, SDL_Keycode> strumBinds = {SDLK_RSHIFT, SDLK_RCTRL}; // first is up, second is down
        std::pair<SDL_Keycode, SDL_Keycode> overdriveBinds = {SDLK_SPACE, SDLK_TAB}; // first is main, second is alt

        void SaveToFile(const std::string& filename) const;
        void LoadFromFile(const std::string& filename);
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
        Keybinds,
        keybinds4k,
        keybinds5k,
        keybinds4kalt,
        keybinds5kalt,
        strumBinds,
        overdriveBinds
    );
}

extern Encore::Keybinds TheGameKeybinds;

#endif //ENCORE_DEVELOPERS_KEYBINDS_H
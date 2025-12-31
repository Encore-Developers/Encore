//
// Created by maria on 29/12/2025.
//

#ifndef ENCORE_DEVELOPERS_KEYBINDS_H
#define ENCORE_DEVELOPERS_KEYBINDS_H
#include <array>
#include <string>
#include <nlohmann/json.hpp>

namespace Encore {
    class Keybinds
    {
    public:
        std::array<int, 4> keybinds4k = {68, 70, 74, 75};
        std::array<int, 5> keybinds5k = {68, 70, 74, 75, 76};
        std::array<int, 4> keybinds4kalt = {-2, -2, -2, -2};
        std::array<int, 5> keybinds5kalt = {-2, -2, -2, -2, -2};
        std::pair<int, int> strumBinds = {344, 345}; // first is up, second is down
        std::pair<int, int> overdriveBinds = {32, -2}; // first is main, second is alt
        int pauseBind = 256;

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
        overdriveBinds,
        pauseBind
    );
}

extern Encore::Keybinds TheGameKeybinds;

#endif //ENCORE_DEVELOPERS_KEYBINDS_H
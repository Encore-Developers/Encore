#pragma once

#ifndef ENCORE_KEYBINDS_H
#define ENCORE_KEYBINDS_H

#include "raylib.h"
#include "GLFW/glfw3.h"
#include "game/settings.h"
#include <unordered_map>
#include <string>

class Keybinds {
    static std::unordered_map<int, std::string> keymap;

    static std::unordered_map<int, std::string> GenericNames;

    static std::unordered_map<int, std::string> XBOXNames;

    static std::unordered_map<int, std::string> PS1PS2Names;

    static std::unordered_map<int, std::string> PS3Names;

    static std::unordered_map<int, std::string> PS4Names;

    static std::unordered_map<int, std::string> PS5Names;

    static std::vector<std::vector<std::string>> controllerTypeNames;

public:
    static std::string getKeyStr(int keycode);

    static std::string getControllerStr(int jid, int input, int type, int direction);

    static int getControllerType(const std::string &searchString);
};
#endif
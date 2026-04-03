
#ifndef ENCORE_RAYLIBOPS_H
#define ENCORE_RAYLIBOPS_H

#include "raylib.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

json ColorToJson(Color color) {
    return { {"r", color.r}, {"g", color.g}, {"b", color.b}};
}

Color JsonToColor(json color) {
    return {color["r"], color["g"], color["b"], 255};
}

#endif // ENCORE_RAYLIBOPS_H

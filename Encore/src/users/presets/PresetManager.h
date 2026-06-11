#pragma once
#include "Preset.h"

#include <memory>
#include <string>
#include <unordered_map>

class PresetManager {
public:
    std::unordered_map<std::string, std::unordered_map<std::string, std::weak_ptr<Preset>>> presetRegistry;
};

extern PresetManager ThePresetManager;
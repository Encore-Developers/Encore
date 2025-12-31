//
// Created by maria on 29/12/2025.
//

#include "keybinds.h"

#include "settings-old.h"
#include <raylib.h>
#include <fstream>
#include <nlohmann/json.hpp>


namespace Encore {
    void Keybinds::SaveToFile(const std::string& filename) const {
        try {
            nlohmann::json j = *this;
            std::ofstream file(filename);
            if (!file.is_open()) {
                TraceLog(LOG_ERROR, "Failed to open keybinds file for writing: %s", filename.c_str());
                return;
            }
            file << j.dump(4);
            file.close();
            TraceLog(LOG_INFO, "Settings saved to %s", filename.c_str());
        } catch (const std::exception& e) {
            TraceLog(LOG_ERROR, "Error saving keybinds to %s: %s", filename.c_str(), e.what());
        }
    }

    void Keybinds::LoadFromFile(const std::string& filename) {
        try {
            std::ifstream file(filename);
            if (!file.is_open()) {
                TraceLog(LOG_WARNING, "Keybinds file not found, using defaults: %s", filename.c_str());
                return;
            }
            nlohmann::json j;
            file >> j;
            file.close();
            j.get_to(*this);
            TraceLog(LOG_INFO, "Keybinds loaded from %s", filename.c_str());
        } catch (const std::exception& e) {
            TraceLog(LOG_ERROR, "Error loading keybinds from %s: %s", filename.c_str(), e.what());
        }
    }
}
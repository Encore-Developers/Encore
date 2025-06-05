#include "settings.h"
#include "raylib.h"
#include <fstream>

namespace Encore {
    void Settings::SaveToFile(const std::string& filename) const {
        nlohmann::json j = *this;
        std::ofstream file(filename);
        if (file.is_open()) {
            file << j.dump(4);
            file.close();
            TraceLog(LOG_INFO, "Settings saved to %s", filename.c_str());
        } else {
            TraceLog(LOG_ERROR, "Failed to open %s for writing", filename.c_str());
        }
    }

    void Settings::LoadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (file.is_open()) {
            nlohmann::json j;
            file >> j;
            file.close();
            if (!j.is_null()) {
                j.get_to(*this);
                TraceLog(LOG_INFO, "Settings loaded from %s", filename.c_str());
            } else {
                TraceLog(LOG_ERROR, "Invalid JSON in %s", filename.c_str());
            }
        } else {
            TraceLog(LOG_ERROR, "Failed to open %s for reading", filename.c_str());
        }
    }

    Settings TheGameSettings;
}
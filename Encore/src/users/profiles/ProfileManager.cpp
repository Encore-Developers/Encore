//
// Created by maria on 06/05/2026.
//

#include "ProfileManager.h"

#include <fstream>

using namespace nlohmann;

void Encore::ProfileManager::LoadColorProfiles() {
    for (const auto& file : std::filesystem::directory_iterator(ColorProfilesPath)) {
        if (file.exists() && file.path().extension() == ".json") {
            std::ifstream f(file.path());
            json fileJson = json::parse(f);
            ColorProfile colorProfile = fileJson.get<ColorProfile>();
            for (size_t i = 0; i < SLOT_MAX; i++) {
                if (fileJson["colors"].is_array() && fileJson["colors"].size() > i) {
                    colorProfile.colors[i] = fileJson["colors"][i];
                }
            }
            colorProfile.builtin = false;
            ColorProfiles.emplace(colorProfile.Name, colorProfile);
            f.close();
        }
    }
}

void Encore::ProfileManager::SaveColorProfiles() {
    for (const auto& profile : ColorProfiles) {
        if (profile.second.builtin) {
            continue;
        }
        json outJson;
        outJson = profile.second;
        for (size_t i = 0; i < SLOT_MAX; i++) {
            outJson["colors"][i] = profile.second.colors[i];
        }
        std::ofstream f(ColorProfilesPath / (profile.second.Name + ".json"), std::ios::out | std::ios::trunc);
        f << outJson.dump(2, ' ', false, detail::error_handler_t::strict);
        f.close();
    }
}
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
            ColorProfiles.emplace_back(fileJson.get<ColorProfile>());
            f.close();
        }
    }
}

void Encore::ProfileManager::SaveColorProfiles() {
    for (const auto& profile : ColorProfiles) {
        if (profile.Name == defaultProfile.Name || profile.Name == transgender.Name) {
            continue;
        }
        json outJson;
        outJson = profile;
        std::ofstream f(ColorProfilesPath / (profile.Name + ".json"), std::ios::out | std::ios::trunc);
        f << outJson.dump(2, ' ', false, detail::error_handler_t::strict);
        f.close();
    }
}
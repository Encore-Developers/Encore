#pragma once
#include <vector>

#include "users/ColorProfile.h"

namespace Encore {
    class ProfileManager
    {
        std::filesystem::path ColorProfilesPath;
    public:
        std::vector<ColorProfile> ColorProfiles;
        void SetColorProfilesPath(const std::filesystem::path& path) {
            if (!exists(path)) {
                create_directory(path);
            }
            ColorProfilesPath = path;
        }
        void LoadColorProfiles();
        void CreateColorProfile() {
            ColorProfiles.emplace_back(ColorProfile());
        }
        void SaveColorProfiles();
        ProfileManager() {
            ColorProfiles.emplace_back(defaultProfile);
            ColorProfiles.emplace_back(transgender);
        };
    };
}

extern Encore::ProfileManager TheProfileManager;
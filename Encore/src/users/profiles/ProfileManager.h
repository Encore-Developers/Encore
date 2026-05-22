#pragma once
#include <vector>

#include "users/ColorProfile.h"

namespace Encore {
    class ProfileManager
    {
        std::filesystem::path ColorProfilesPath;
    public:
        std::map<std::string, ColorProfile> ColorProfiles;
        void SetColorProfilesPath(const std::filesystem::path& path) {
            if (!exists(path)) {
                create_directory(path);
            }
            ColorProfilesPath = path;
        }
        void LoadColorProfiles();
        void CreateColorProfile() {
            ColorProfiles.emplace("New Profile", ColorProfile("New Profile"));
        }
        void SaveColorProfiles();
        ProfileManager() {
            defaultProfile.builtin = true;
            transgender.builtin = true;
            ColorProfiles.emplace(defaultProfile.Name, defaultProfile);
            ColorProfiles.emplace(transgender.Name, transgender);
        };
    };
}

extern Encore::ProfileManager TheProfileManager;
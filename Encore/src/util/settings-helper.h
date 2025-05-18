//
// Created by maria on 14/02/2025.
//

#ifndef SETTINGSLOADER_H
#define SETTINGSLOADER_H
#include <filesystem>

#pragma once
#include <string>
#include <vector>

namespace Encore {
    class SettingsInit {
        std::filesystem::path directory;
        void ReadSettings();
        void CreateSettings();
        void MigrateSettings();
        void LegacyMigrateSettings();
    public:
        void InitSettings(std::filesystem::path directory);
    };
}
// Encore

#endif // SETTINGSLOADER_H

extern Encore::SettingsInit TheSettingsInitializer;
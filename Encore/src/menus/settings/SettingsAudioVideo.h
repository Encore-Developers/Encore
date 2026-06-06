//
// Created by Jaydenz on 04/29/2025.
//
#pragma once

#include "../menu.h"
#include "settings/settings.h"
#include "assets.h"
#include "../overshell/OvershellMenu.h"
#include "menus/util/ButtonActionRegistry.h"
#include "util/SettingRenderer.h"

#ifndef SETTINGSAUDIOVIDEO_H
#define SETTINGSAUDIOVIDEO_H

namespace Encore {
        class SettingsAudioVideo : public OvershellMenu {
#define OPTION(type, value, default) type value = default;
            SETTINGS_OPTIONS;
#undef OPTION
            ButtonActionRegistry buttReg;
        public:
            int calibrationMenuOffset;
            SettingsAudioVideo() = default;
            ~SettingsAudioVideo() override;
            SettingDoohickey settings;
            void Draw();
            void KeyboardInputCallback(SDL_KeyboardEvent* event);
            void ControllerInputCallback(ControllerEvent event);
            void Load();
            void Save();
        };
    }
extern Encore::SettingsAudioVideo TheAudioVideoSettings;

#endif //SETTINGSAUDIOVIDEO_H

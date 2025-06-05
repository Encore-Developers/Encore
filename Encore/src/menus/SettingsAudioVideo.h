//
// Created by Jaydenz on 04/29/2025.
//
#pragma once

#include "menu.h"
#include "settings.h"
#include "../../include/assets.h"
#include "OvershellMenu.h"

#ifndef SETTINGSAUDIOVIDEO_H
#define SETTINGSAUDIOVIDEO_H

namespace Encore {
    class SettingsAudioVideo {
    };
}

class SettingsAudioVideo : public OvershellMenu {
#define OPTION(type, value, default) type value = default;
    SETTINGS_OPTIONS;
#undef OPTION
public:
    int calibrationMenuOffset;
    int GUI_TEXT_ALIGN_CENTER;
    SettingsAudioVideo() = default;
    ~SettingsAudioVideo() override = default;
    void GuiSlider(Rectangle bounds,
        std::nullptr_t null,
        std::nullptr_t text_right,
        int value,
        float min_value,
        float max_value
    );
    void Draw();
    void KeyboardInputCallback(int key, int scancode, int action, int mods);
    void ControllerInputCallback(int joypadID, GLFWgamepadstate state);
    void Load();
    void Save();
};

extern Encore::SettingsAudioVideo TheAudioVideoSettings;

#endif //SETTINGSAUDIOVIDEO_H

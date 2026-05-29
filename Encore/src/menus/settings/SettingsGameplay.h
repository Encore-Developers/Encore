//
// Created by Jaydenz on 04/29/2025.
//

#ifndef SETTINGS_GAMEPLAY_H
#define SETTINGS_GAMEPLAY_H

#include <GLFW/glfw3.h>
#include "../overshell/OvershellMenu.h"
#include "menus/util/ButtonActionRegistry.h"
#include "menus/util/Jukebox.h"
#include "menus/util/SettingRenderer.h"
#include "song/songlist.h"


namespace Encore {
    class SettingsGameplay : public OvershellMenu {
        std::function<void()> scanSongsFunc;
#define OPTION(type, value, default) type value = default;
        SETTINGS_OPTIONS;
#undef OPTION
        bool ad = false;
        void ScanSongs() {
            TheGameJukebox.UnloadStreams();
            if (TheGameSettings.SongPaths.empty()) {
                TraceLog(LOG_ERROR, "SongPaths is empty. Cannot scan songs.");
            } else {
                for (const auto& path : TheGameSettings.SongPaths) {
                    TraceLog(LOG_INFO, "Scanning path: %s", path.string().c_str());
                }
                TheSongList.ScanSongs(TheGameSettings.SongPaths);
            }
        }
    public:
        void Draw();
        void KeyboardInputCallback(int key, int scancode, int action, int mods);
        void ControllerInputCallback(RhythmEngine::ControllerEvent event);
        void Load();
        void Save();
        SettingDoohickey settings;
        ButtonActionRegistry buttReg;
    };
}

#endif // SETTINGS_GAMEPLAY_H
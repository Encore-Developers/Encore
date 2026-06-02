//
// Created by Jaydenz on 04/29/2025.
//

#ifndef SETTINGS_GAMEPLAY_H
#define SETTINGS_GAMEPLAY_H

#include "../overshell/OvershellMenu.h"
#include "menus/util/ButtonActionRegistry.h"
#include "menus/util/Jukebox.h"
#include "menus/util/SettingRenderer.h"
#include "song/cacheload.h"
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
            SongCount = 0;
            BadSongCount = 0;
            FolderCount = 0;
            SongsHashed = 0;
            if (TheGameSettings.SongPaths.empty()) {
                TraceLog(LOG_ERROR, "SongPaths is empty. Cannot scan songs.");
            } else {
                for (const auto& path : TheGameSettings.SongPaths) {
                    TraceLog(LOG_INFO, "Scanning path: %s", path.string().c_str());
                }
                ScanSongsThread = std::thread([]() {
                    TheSongList.ScanSongs(TheGameSettings.SongPaths);
                    SongCount = 0;
                    BadSongCount = 0;
                    FolderCount = 0;
                    SongsHashed = 0;
                });
                ScanSongsThread.detach();
            }
        }
    public:
        std::thread ScanSongsThread;
        void Draw();
        void KeyboardInputCallback(SDL_KeyboardEvent* event);
        void ControllerInputCallback(RhythmEngine::ControllerEvent event);
        void Load();
        void Save();
        SettingDoohickey settings;
        ButtonActionRegistry buttReg;
    };
}

#endif // SETTINGS_GAMEPLAY_H

#ifndef ENCORE_DEBUG_H
#define ENCORE_DEBUG_H
#include "imgui.h"
#include "users/playerManager.h"

namespace EncoreDebug {

    class Indent {
    public:
        Indent() {
            ImGui::Indent();
        }

        ~Indent() {
            ImGui::Unindent();
        }
    };

    extern bool showDebug;
    extern bool reloadQueued;
    extern bool reloadFonts;
    extern bool showGameplayHud;
    extern bool showEngineWindow[MAX_PLAYERS];
    void DrawDebug();
    void MenuBar();
    void DrawAssetViewer();
    void DrawPlayerManager();
    void DrawSongList();
    void DrawQuickSettings();
    void DrawSongScrubber();
    void DrawPracticeSectionSelector();
    void DrawColorProfileSettings();
    void DrawJoystickTools();
    void DrawEngineWindow(int slot);
    void DrawLocaleDebug();
    void DrawLog();

    void StartReloadAssets();
}


#endif // ENCORE_DEBUG_H

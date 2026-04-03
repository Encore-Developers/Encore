
#ifndef ENCORE_DEBUG_H
#define ENCORE_DEBUG_H
#include "imgui.h"

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

    void DrawDebug();
    void MenuBar();
    void DrawAssetViewer();
    void DrawPlayerManager();
    void DrawSongList();
    void DrawQuickSettings();
    void DrawSongScrubber();
    void DrawPracticeSectionSelector();

    void StartReloadAssets();
}


#endif // ENCORE_DEBUG_H

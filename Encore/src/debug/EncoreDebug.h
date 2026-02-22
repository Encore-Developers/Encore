
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

    void DrawDebug();
    void MenuBar();
    void DrawAssetViewer();
}


#endif // ENCORE_DEBUG_H

#include "EncoreDebug.h"

#include "assets.h"
#include "imgui.h"
#include "misc/imgui_stdlib.h"

bool EncoreDebug::showDebug = false;

bool showDemoWindow = false;
bool showAssets = false;

void EncoreDebug::DrawDebug() {
    MenuBar();
    if (showAssets) {
        DrawAssetViewer();
    }

    if (showDemoWindow) {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }
}


void EncoreDebug::MenuBar() {
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("Windows")) {
        ImGui::MenuItem("Assets", 0, &showAssets);
        ImGui::MenuItem("ImGui Demo Window", 0, &showDemoWindow);
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
}

void EncoreDebug::DrawAssetViewer() {
    ImGui::SetNextWindowSize({200, 300}, ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Assets", &showAssets, 0)) {
        int i = 0;
        for (auto asset : TheAssets.assets) {
            ImGui::BeginChild(TextFormat("##%i", i), {0, 0}, ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeY);
            ImGui::Text(TextFormat("%s (%x)", asset->id.c_str(), (size_t)asset));
            ImGui::Text(typeid(*asset).name());
            ImGui::Text(AssetStateName(asset->state));

            switch (asset->state) {
            case UNLOADED:
                if (ImGui::Button("Load")) {
                    asset->StartLoad();
                }
                break;
            case PREFINALIZED:
                if (ImGui::Button("Finalize")) {
                    asset->CheckForFetch();
                }
                break;
            }

            ImGui::EndChild();
            i++;
        }
    }
    ImGui::End();

}
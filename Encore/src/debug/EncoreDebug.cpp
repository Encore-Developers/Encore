#include "EncoreDebug.h"

#include "assets.h"
#include "imgui.h"
#include "users/playerManager.h"
#include "misc/imgui_stdlib.h"
#include "settings/settings.h"
#include "util/frame-manager.h"

bool EncoreDebug::showDebug = false;

bool showDemoWindow = false;
bool showAssets = false;
bool showPlayerManager = false;

std::string debugVersionHash = "";

void ColorEdit(const char* label, Color* color, ImGuiColorEditFlags flags) {
    float floats[3] = {color->r/255.0f, color->g/255.0f, color->b/255.0f};

    ImGui::ColorEdit3(label, (float*)&floats, flags);

    color->r = floats[0]*255;
    color->g = floats[1]*255;
    color->b = floats[2]*255;
}

void EncoreDebug::DrawDebug() {
    if (debugVersionHash.empty()) {
        debugVersionHash = TextFormat(
            "Encore %s-%s:%s", ENCORE_VERSION, GIT_COMMIT_HASH, GIT_BRANCH
        );
    }
    MenuBar();
    if (showAssets) {
        DrawAssetViewer();
    }

    if (showDemoWindow) {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }
    if (showPlayerManager) {
        DrawPlayerManager();
    }
}


void EncoreDebug::MenuBar() {
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("Windows")) {
        ImGui::MenuItem("Assets", 0, &showAssets);
        ImGui::MenuItem("Player Manager", 0, &showPlayerManager);
        ImGui::MenuItem("ImGui Demo Window", 0, &showDemoWindow);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Framerate")) {
        ImGui::Text("%i FPS", GetFPS());
        ImGui::MenuItem("Uncap Framerate", 0, &TheFrameManager.removeFPSLimit);
        ImGui::MenuItem("VSync", 0, &TheGameSettings.VerticalSync);
        ImGui::SliderInt("Menu FPS", &TheFrameManager.menuFPS, 1, 300);
        ImGui::SliderInt("Gameplay FPS", &TheGameSettings.Framerate, 1, 1500);
        ImGui::EndMenu();
    }

    auto avail = ImGui::GetWindowWidth();
    auto size = ImGui::CalcTextSize(debugVersionHash.c_str()).x;

    ImGui::SetCursorPosX(avail - size - ImGui::GetStyle().FramePadding.x);
    ImGui::Text(debugVersionHash.c_str());

    ImGui::EndMainMenuBar();
}

void EncoreDebug::DrawPlayerManager() {
    if (ImGui::Begin("Player Manager", &showPlayerManager, 0)) {
        if (ImGui::Button("Save All")) {
            ThePlayerManager.SavePlayerList();
        }

        if (ImGui::BeginTabBar("Players")) {
            for (auto &player : ThePlayerManager.PlayerList) {
                if (ImGui::BeginTabItem((player.Name + TextFormat("###%x", &player)).c_str())) {

                    ImGui::InputText("Username", &player.Name);
                    ImGui::SeparatorText("Color Profile");
                    ColorEdit("Green", &player.GetColorProfile()->colors[Encore::SLOT_GREEN], 0);
                    ColorEdit("Red", &player.GetColorProfile()->colors[Encore::SLOT_RED], 0);
                    ColorEdit("Yellow", &player.GetColorProfile()->colors[Encore::SLOT_YELLOW], 0);
                    ColorEdit("Blue", &player.GetColorProfile()->colors[Encore::SLOT_BLUE], 0);
                    ColorEdit("Orange", &player.GetColorProfile()->colors[Encore::SLOT_ORANGE], 0);
                    ColorEdit("Open", &player.GetColorProfile()->colors[Encore::SLOT_OPEN], 0);

                    ImGui::SeparatorText(std::string("Player: " + player.Name).c_str());
                    ImGui::SliderFloat("Note Speed", &player.NoteSpeed, 0, 3);
                    ImGui::SliderFloat("Track Length", &player.HighwayLength, 0, 5);
                    int inputOffset = player.InputCalibration * 1000;
                    ImGui::DragInt("Input Calibration", &inputOffset, 1, -1000, 1000, "%dms");
                    player.InputCalibration = inputOffset / 1000.0;
                    ColorEdit("Accent Color", &player.AccentColor, 0);
                    ImGui::Checkbox("Bot", &player.Bot);
                    ImGui::Checkbox("Lefty Flip", &player.LeftyFlip);
                    ImGui::Checkbox("Brutal Mode", &player.BrutalMode);
                    ImGui::EndTabItem();

                    if (ImGui::Button("Delete Player")) {
                        ThePlayerManager.DeletePlayer(player);
                        ThePlayerManager.SavePlayerList();
                    }
                }
            }
            if (ImGui::TabItemButton("New", ImGuiTabItemFlags_Trailing)) {
                ThePlayerManager.CreatePlayer("New Player");
            }
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
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
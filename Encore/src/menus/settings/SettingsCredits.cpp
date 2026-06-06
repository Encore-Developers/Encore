//
// Created by Jaydenz on 04/29/2025.
//

#include "SettingsCredits.h"

#include "imgui.h"
#include "SettingsMenu.h"
#include "menus/MenuManager.h"
#include "../util/uiUnits.h"
#include "menus/main/MainMenu.h"
#include "menus/overshell/OvershellHelper.h"

using namespace Encore;



void SettingsCredits::Load() {
    std::ifstream cJsonFile(TheAssets.getDirectory() / "contributors.json");
    nlohmann::json cJson;
    cJsonFile >> cJson;
    cJsonFile.close();
    maintainer = cJson["maintainers"];
    founder = cJson["founder"];
    locale = cJson["localization"];
    contributor = cJson["contributors"];

    maintainerFunc = [this]() {
        if (ImGui::BeginTable("maintainers", maintainer.size())) {
            ImGui::TableNextRow();
            for (int m = 0; m < maintainer.size(); m++) {
                ImGui::TableSetColumnIndex(m);
                ImGui::Text("%s", maintainer[m].c_str());
            }
        }
        ImGui::EndTable();
    };

    founderFunc = [this]() {
        if (ImGui::BeginTable("founder", founder.size())) {
            ImGui::TableNextRow();
            for (int m = 0; m < founder.size(); m++) {
                ImGui::TableSetColumnIndex(m);
                ImGui::Text("%s", founder[m].c_str());
            }
        }
        ImGui::EndTable();
    };

    localeFunc = [this]() {
        if (ImGui::BeginTable("locale", locale.size())) {
            ImGui::TableNextRow();
            for (int m = 0; m < locale.size(); m++) {
                ImGui::TableSetColumnIndex(m);
                ImGui::Text("%s", locale[m].c_str());
            }
        }
        ImGui::EndTable();
    };

    contributorFunc = [this]() {
        Units &u = Units::getInstance();

        ImGui::SetNextItemWidth(u.winpct(0.7));
        if (ImGui::BeginTable("contributors", 4)) {
            ImGui::TableNextRow();
            for (int m = 0; m < contributor.size(); m++) {
                if (m % 4 == 0 && m > 3) {
                    ImGui::TableNextRow();
                }
                ImGui::TableNextColumn();
                ImGui::Text("%s", contributor[m].c_str());
            }
        }
        ImGui::EndTable();
    };
    settings.Add(new SettingDoohickey::separatorObject("settings.credits.maintainers"));
    settings.Add(new SettingDoohickey::goMyScarabs("Maintainers", &maintainerFunc));
    settings.Add(new SettingDoohickey::separatorObject("settings.credits.founder"));
    settings.Add(new SettingDoohickey::goMyScarabs("Founder", &founderFunc));
    settings.Add(new SettingDoohickey::separatorObject("settings.credits.locale"));
    settings.Add(new SettingDoohickey::goMyScarabs("Locale", &localeFunc));
    settings.Add(new SettingDoohickey::separatorObject("settings.credits.contributors"));
    settings.Add(new SettingDoohickey::goMyScarabs("Contributors", &contributorFunc, 4));

    NEWBUTTONACTION2(buttReg, LANE_2, "generic.back", {
        if (_action != Encore::Action::PRESS) return;
        TheMenuManager.CreateAndSwitchMenu<SettingsMenu>();
    });
    NEWBUTTONACTION2(buttReg, LANE_1, "generic.back", {
        if (_action != Encore::Action::PRESS) return;
        TheMenuManager.CreateAndSwitchMenu<SettingsMenu>();
    });
}

void SettingsCredits::Draw() {
    Units &u = Units::getInstance();
    Assets &assets = Assets::getInstance();

    GameMenu::DrawAlbumArtBackground();
    DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), Color { 0, 0, 0, 128 });

    encOS::DrawTopOvershell(0.15f);
    GameMenu::DrawVersion();
    GameMenu::DrawBottomOvershell();

    float TextPlacementTB = u.hpct(0.05f);
    Text::lDrawText(assets.rubik, "settings.header.credits", {u.LeftSide, u.hpct(0.027f)}, u.hinpct(0.042f), LIGHTGRAY, LEFT);
    Text::lDrawText(assets.redHatDisplayBlack, "settings.header.main", {u.LeftSide, TextPlacementTB}, u.hinpct(0.125f), WHITE, LEFT);

    settings.isOSOpen = isOSOpen();
    settings.Draw();

    GameMenu::DrawBottomOvershell();
    buttReg.DrawPrompts(isOSOpen());
    DrawOvershell();
}
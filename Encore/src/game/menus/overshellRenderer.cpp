//
// Created by marie on 03/08/2024.
//

#include "game/menus/overshellRenderer.h"
#include "raylib.h"
#include "raygui.h"
#include "game/menus/uiUnits.h"
#include "sol/sol.hpp"
#include "game/assets.h"

void OvershellRenderer::DrawOvershell() {
	//sol::state lua;
	//lua.open_libraries(sol::lib::base);
	Assets &assets = Assets::getInstance();
	Units &unit = Units::getInstance();
	//lua.set_function("hpct", &Units::hpct);
	//lua.set_function("hinpct", &Units::hinpct);
	//lua.set_function("winpct", &Units::winpct);
	//lua["leftside"] = unit.LeftSide;
	//lua.set_function("DrawRectangle", DrawRectangle);
	//lua.script_file("scripts/ui/overshell.lua");
	PlayerManager &playerManager = PlayerManager::getInstance();
	float LeftMin = unit.wpct(0.1);
	float LeftMax = unit.wpct(0.9);
	for (int i = 0; i < 4; i++) {
		bool EmptySlot = true;
		float OvershellTopLoc = unit.hpct(1.0f)-unit.winpct(0.05f);
		float OvershellLeftLoc = (unit.wpct(0.125) + (unit.winpct(0.25)*i))-unit.winpct(0.1);
		float OvershellCenterLoc = (unit.wpct(0.125) + (unit.winpct(0.25)*i));

		if (!playerManager.ActivePlayers.empty()) {
			if (i < playerManager.ActivePlayers.size()) {
				if (GuiButton({OvershellLeftLoc, OvershellTopLoc+unit.winpct(0.01f), unit.winpct(0.2f), unit.winpct(0.04f)}, "")) {
					playerManager.RemoveActivePlayer(i);
				} else {
					float playerNameSize = MeasureTextEx(assets.redHatDisplayBlack, playerManager.GetActivePlayer(i)->Name.c_str(), unit.winpct(0.03f), 0).x;

					DrawRectangle(OvershellLeftLoc, OvershellTopLoc, unit.winpct(0.2f), unit.winpct(0.05f), GRAY);
					DrawTextEx(assets.redHatDisplayBlack, playerManager.GetActivePlayer(i)->Name.c_str(), {OvershellCenterLoc-(playerNameSize/2), OvershellTopLoc+unit.winpct(0.01f)}, unit.winpct(0.03f), 0, WHITE);
				}
			} else {
				if (GuiButton({OvershellLeftLoc, OvershellTopLoc+unit.winpct(0.01f), unit.winpct(0.2f), unit.winpct(0.04f)}, "")) {
					playerManager.AddActivePlayer(playerManager.PlayersActive);
				} else {
					float playerNameSize = MeasureTextEx(assets.redHatDisplayBlack, "JOIN", unit.winpct(0.02f), 0).x;
					DrawRectangle(OvershellLeftLoc, OvershellTopLoc+unit.winpct(0.01f), unit.winpct(0.2f), unit.winpct(0.04f), DARKGRAY);
					DrawTextEx(assets.redHatDisplayBlack, "JOIN", {OvershellCenterLoc-(playerNameSize/2), OvershellTopLoc+unit.winpct(0.02f)}, unit.winpct(0.02f), 0, WHITE);
				}
			}
		} else {
			if (GuiButton({OvershellLeftLoc, OvershellTopLoc+unit.winpct(0.01f), unit.winpct(0.2f), unit.winpct(0.04f)}, "")) {
				playerManager.AddActivePlayer(playerManager.PlayersActive);
			} else {
				float playerNameSize = MeasureTextEx(assets.redHatDisplayBlack, "JOIN", unit.winpct(0.02f), 0).x;
				DrawRectangle(OvershellLeftLoc, OvershellTopLoc+unit.winpct(0.01f), unit.winpct(0.2f), unit.winpct(0.04f), DARKGRAY);
				DrawTextEx(assets.redHatDisplayBlack, "JOIN", {OvershellCenterLoc-(playerNameSize/2), OvershellTopLoc+unit.winpct(0.02f)}, unit.winpct(0.02f), 0, WHITE);
			}
		}


	}
};

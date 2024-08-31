//
// Created by marie on 03/08/2024.
//

#include "overshellRenderer.h"
#include "raylib.h"
#include "raygui.h"
#include "uiUnits.h"
#include "assets.h"

std::vector<bool> SlotSelectingState = {false, false, false, false};

void OvershellRenderer::DrawOvershell() {
	Assets &assets = Assets::getInstance();
	Units &unit = Units::getInstance();
	PlayerManager &playerManager = PlayerManager::getInstance();
	float LeftMin = unit.wpct(0.1);
	float LeftMax = unit.wpct(0.9);
	for (int i = 0; i < 4; i++) {
		bool EmptySlot = true;

		float OvershellTopLoc = unit.hpct(1.0f)-unit.winpct(0.05f);
		float OvershellLeftLoc = (unit.wpct(0.125) + (unit.winpct(0.25)*i))-unit.winpct(0.1);
		float OvershellCenterLoc = (unit.wpct(0.125) + (unit.winpct(0.25)*i));

		if (SlotSelectingState[i]) {
			if (GuiButton({OvershellLeftLoc, unit.hpct(1.0f)-(unit.winpct(0.03f)), unit.winpct(0.2f), unit.winpct(0.03f)}, "Cancel")) {
				SlotSelectingState[i] = false;
			}
			BeginBlendMode(BLEND_MULTIPLIED);
			DrawRectangleRec({OvershellLeftLoc, unit.hpct(1.0f)-(unit.winpct(0.03f)), unit.winpct(0.2f), unit.winpct(0.03f)}, Color{255,0,0,128});
			EndBlendMode();
			for (int x = 0; x < playerManager.PlayerList.size(); x++) {

				if (playerManager.ActivePlayers[i] == -1) {
					if (GuiButton({OvershellLeftLoc, unit.hpct(1.0f)-(unit.winpct(0.03f)*(x+2)), unit.winpct(0.2f), unit.winpct(0.03f)}, playerManager.PlayerList[x].Name.c_str())) {
						playerManager.AddActivePlayer(x, i);
						SlotSelectingState[i] = false;
					}
				}
			}
			float playerNameSize = MeasureTextEx(assets.redHatDisplayBlack, "Select a player", unit.winpct(0.03f), 0).x;
			float InfoLoc = unit.winpct(0.03f)*(playerManager.PlayerList.size()+1);

			DrawRectangle(OvershellLeftLoc, unit.hpct(1.0f)-InfoLoc-unit.winpct(0.05f), unit.winpct(0.2f), unit.winpct(0.05f), GRAY);
			DrawTextEx(assets.redHatDisplayBlack, "Select a player", {OvershellCenterLoc-(playerNameSize/2), unit.hpct(1.0f)-InfoLoc+unit.winpct(0.01f)-unit.winpct(0.05f)}, unit.winpct(0.03f), 0, WHITE);

		} else if (playerManager.ActivePlayers[i] != -1) { //player active
			if (GuiButton({OvershellLeftLoc, OvershellTopLoc+unit.winpct(0.01f), unit.winpct(0.2f), unit.winpct(0.04f)}, "")) {
				playerManager.RemoveActivePlayer(i);
			} else {
				float playerNameSize = MeasureTextEx(assets.redHatDisplayBlack, playerManager.GetActivePlayer(i)->Name.c_str(), unit.winpct(0.03f), 0).x;

				DrawRectangle(OvershellLeftLoc, OvershellTopLoc, unit.winpct(0.2f), unit.winpct(0.05f), GRAY);
				DrawTextEx(assets.redHatDisplayBlack, playerManager.GetActivePlayer(i)->Name.c_str(), {OvershellCenterLoc-(playerNameSize/2), OvershellTopLoc+unit.winpct(0.01f)}, unit.winpct(0.03f), 0, WHITE);
			}
		} else { // no active players
			if (GuiButton({OvershellLeftLoc, OvershellTopLoc+unit.winpct(0.01f), unit.winpct(0.2f), unit.winpct(0.04f)}, "")) {
				SlotSelectingState[i] = true;
				// playerManager.AddActivePlayer(i, i);
			} else {
				float playerNameSize = MeasureTextEx(assets.redHatDisplayBlack, "JOIN", unit.winpct(0.02f), 0).x;
				DrawRectangle(OvershellLeftLoc, OvershellTopLoc+unit.winpct(0.01f), unit.winpct(0.2f), unit.winpct(0.04f), DARKGRAY);
				DrawTextEx(assets.redHatDisplayBlack, "JOIN", {OvershellCenterLoc-(playerNameSize/2), OvershellTopLoc+unit.winpct(0.02f)}, unit.winpct(0.02f), 0, WHITE);
			}
		}
	}
};

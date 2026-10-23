//
// Created by marie on 16/11/2024.
//

#include "ReadyUpMenu.h"

#include "ChartLoadingMenu.h"
#include "../overshell/OvershellHelper.h"
#include "../MenuManager.h"
#include "menus/util/GameMenu.h"
#include "raygui.h"
#include "../util/styles.h"
#include "../util/uiUnits.h"
#include "song/ArtLoader.h"
#include "menus/util/locale/Locale.h"
#include "menus/main/SongSelectMenu.h"
#include "RhythmEngine/ChartLoaders/ChartLoader.h"
#include "menus/util/TextDisplay.h"

#include "users/playerManager.h"

void ReadyUpMenu::ControllerInputCallback(Encore::ControllerEvent event) {
    auto &Parts = chartLoader.GetSongParts();
    for (int i = 0; i < ThePlayerManager.ActivePlayers.size(); i++) {
        auto player = ThePlayerManager.ActivePlayers[i];
        if (!player)
            continue;
        if (player->joypadID != event.slot)
            continue;
        if (event.slot == std::numeric_limits<unsigned int>::max())
            return;

        buttReg.HandleInput(event);
        if (event.action == Encore::Action::PRESS) {
            int diffCount = 0;
            TrackInformation& part = Parts[player->Instrument];
            for (int d = 0; d < 4; d++) {
                if (part.ValidDiffs[d]) {
                    diffCount += 1;
                }
            }
            if (diffCount != 0)
                ControllerDiffSlot[i] = ControllerDiffSlot[i] % diffCount;
        }
    }
}

PartOption::PartOption(SimpleMenu::Instance * inst, ReadyUpMenu* menu, int slot, int part) : SimpleMenu::Option(inst, LOCALISE(songPartsList[part])), menu(menu), slot(slot), part(part) {

}
void PartOption::Draw(
    Encore::TextDisplay display, size_t index, SimpleMenu::MouseState mouseState
) {
    auto originalPadding = display.padding.x / 2.0f;
    display.Padding(display.padding.x + display.fontSize + display.padding.x * 1.5f, display.padding.y);
    Option::Draw(display, index, mouseState);
    int num = part > PartVocals
        ? part - PartVocals - 1
        : part;
    auto iconA = TheAssets.InstIcons.at(num);
    Rectangle dest = { display.pos.x + originalPadding,
                       display.pos.y + display.padding.y / 2.0f,
                       display.fontSize + display.padding.y,
                       display.fontSize + display.padding.y };
    DrawCircle(dest.x + dest.width/2.0f, dest.y + dest.height/2.0f, dest.width/2.0f, {0, 0, 0, 200});
    DrawTexturePro(
        iconA->Fetch(),
        { 0, 0, float(iconA->width), float(iconA->height) },
        dest,
        { 0, 0 },
        0,
        WHITE
    );

    auto diff = menu->curSong->Difficulties.at(part);
    if (diff > 0) {
        auto ringA = TheAssets.YargRings.at(diff);
        DrawTexturePro(
            ringA->Fetch(),
            { 0, 0, float(iconA->width), float(iconA->height) },
            dest,
            { 0, 0 },
            0,
            WHITE
        );
    }


}
bool PartOption::Input(Encore::ControllerEvent event) {
    if (event.action == Encore::Action::PRESS && event.channel == Encore::InputChannel::LANE_1) {
        auto tempSlot = slot;
        auto tempPart = part;
        menu->defer.push_back([tempSlot, tempPart](ReadyUpMenu* menu) {
            ThePlayerManager.GetActivePlayer(tempSlot).Instrument = tempPart;
            menu->LoadMenuState(tempSlot, ReadyUpMenu::DIFFICULTY);
        });
        return true;
    }
    return false;
}
void ReadyUpMenu::Back(int slot) {
    switch (SlotState[slot]) {
    case INSTRUMENT:
        TheMenuManager.CreateAndSwitchMenu<SongSelectMenu>();
        break;
    case DIFFICULTY:
        LoadMenuState(slot, INSTRUMENT);
        break;
    case READY:
        LoadMenuState(slot, DIFFICULTY);
        break;
    }
}

void ReadyUpMenu::KeyboardInputCallback(SDL_KeyboardEvent* event) {
}


void ReadyUpMenu::LoadMenuState(int slot, ReadyUpMenuState state) {
    auto& menu = Menus[slot];
    SlotState[slot] = state;
    menu.options.clear();
    menu.selectedIndex = 0;
    switch (state) {
    case INSTRUMENT: {
        for (auto part : PartsToDisplay) {
            menu.CreateOption<PartOption>(this, slot, part);
            if (ThePlayerManager.ActivePlayers[slot]) {
                auto& player = ThePlayerManager.GetActivePlayer(slot);
                if (player.Instrument == part) {
                    menu.selectedIndex = menu.options.size()-1;
                }
            }
        }
        break;
    }
    case DIFFICULTY: {
        if (!ThePlayerManager.ActivePlayers[slot]) break;
        auto& Parts = chartLoader.GetSongParts();
        auto& player = ThePlayerManager.GetActivePlayer(slot);
        TrackInformation &part = Parts[player.Instrument];
        for (int i = 0; i < 4; i++) {
            if (part.ValidDiffs[i]) {
                menu.CreateOption<SimpleMenu::FuncOption>(LOCALIZE(diffList[i]), [this, &player, i, slot] {
                    player.Difficulty = i;
                    defer.push_back([this, slot](ReadyUpMenu*) {
                        LoadMenuState(slot, READY);
                    });
                });
                if (player.Difficulty == i) {
                    menu.selectedIndex = menu.options.size()-1;
                }
            }
        }
        break;
    }
    case READY: {
        menu.CreateOption<SimpleMenu::FuncOption>(LOCALIZE("generic.back"), [this, slot] {
            defer.push_back([this, slot](ReadyUpMenu*) {
                LoadMenuState(slot, DIFFICULTY);
            });
        });
    }
    }

    if (!menu.options.empty()) {
        // Hack: reverse the options because that's how it was before
        auto options = menu.options;
        for (int i = 0; i < options.size(); i++) {
            menu.options[options.size()-1-i] = options[i];
        }

        menu.selectedIndex = options.size()-1-menu.selectedIndex;
    }

}

void ReadyUpMenu::Draw() {
    bool SwitchMenus = true;
    Assets &assets = Assets::getInstance();
    GameMenu::DrawAlbumArtBackground();
    float AlbumArtLeft = Units::LeftSide;
    float AlbumArtTop = Units::hpct(0.04f);
    float AlbumArtRight = Units::winpct(0.15f);

    Rectangle albumRect{ AlbumArtLeft, AlbumArtTop, AlbumArtRight, AlbumArtRight };
    NPatchInfo shadowOverlay;
    shadowOverlay.source = {0,0,128,128};
    shadowOverlay.top = AlbumArtRight*0.1;
    shadowOverlay.bottom = AlbumArtRight*0.1;
    shadowOverlay.left = AlbumArtRight*0.1;
    shadowOverlay.right = AlbumArtRight*0.1;
    shadowOverlay.layout = 0;
    DrawRectangle(
        0,
        0,
        (int)GetRenderWidth(),
        (int)GetRenderHeight(),
        GetColor(0x00000080)
    );

    GameMenu::DrawTopOvershell(0.2f);
    GameMenu::DrawTopBarText(true);

    // buttReg.DrawPrompts(false, Units::hpct(0.2f), AlbumArtLeft + AlbumArtRight + 24);
    if (TheArtLoader.loadedArt->GetTexture().id != 0) {
        DrawTexturePro(
            *TheArtLoader.loadedArt,
            Rectangle{ 0,
                       0,
                       (float)TheArtLoader.loadedArt->GetTexture().width,
                       (float)TheArtLoader.loadedArt->GetTexture().width },
            albumRect,
            { 0, 0 },
            0,
            WHITE
        );
    } else {
        DrawTexturePro(
            ASSET(missingAlbumArt),
            Rectangle{ 0,
                       0,
                       (float)ASSET(missingAlbumArt).width,
                       (float)ASSET(missingAlbumArt).width },
            albumRect,
            { 0, 0 },
            0,
            WHITE
        );
    }
    DrawTextureNPatch(ASSET(borderShadowLight), shadowOverlay, albumRect, {0}, 0, {255,255,255,128});

    float BottomOvershell = Units::hpct(1) - Units::hinpct(0.18f);
    float TextPlacementTB = AlbumArtTop;
    float TextPlacementLR = AlbumArtRight + AlbumArtLeft + 32;
    DrawTextEx(
        assets.redHatDisplayItalic,
        curSong->title.c_str(),
        { TextPlacementLR, TextPlacementTB },
        Units::hinpct(0.05f),
        0,
        WHITE
    );
    DrawTextEx(
        assets.rubikItalic,
        curSong->artist.c_str(),
        { TextPlacementLR, TextPlacementTB + Units::hinpct(0.05125f) },
        Units::hinpct(0.04f),
        0,
        WHITE
    );
    if (!curSong->charters.empty()) {
        DrawTextEx(
            assets.rubikItalic,
            curSong->charters[0].c_str(),
            { TextPlacementLR, TextPlacementTB + Units::hinpct(0.095f) },
            Units::hinpct(0.04f),
            0,
            WHITE
        );
    }
    // GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_TOP);
    // GuiSetStyle(DEFAULT, TEXT_WRAP_MODE, TEXT_WRAP_WORD);
    // GuiSetStyle(DEFAULT, TEXT_LINE_SPACING, Units::hinpct(0.03f));
    // GuiSetStyle(DEFAULT, TEXT_SIZE, Units::hinpct(0.0275));
    //
    // GuiLabel(
    //     { albumRect.x + albumRect.width,
    //       Units::hpct(0.2f),
    //       Units::wpct(1.0f) - (albumRect.x + albumRect.width),
    //       (albumRect.y + albumRect.width) - Units::hpct(0.2f) },
    //       curSong->loadingPhrase.data()
    // );
    //
    // GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_CENTER);
    // GuiSetStyle(DEFAULT, TEXT_WRAP_MODE, TEXT_WRAP_NONE);
    // Encore::Text::DrawText(ASSET(josefinSansNormal), curSong->loadingPhrase, {albumRect.x + albumRect.width, Units::hpct(0.2f)}, Units::hinpct(0.0275), LIGHTGRAY, LEFT);
    // todo: allow this to be run per player
    // load midi
    GameMenu::DrawBottomOvershell();
    auto &Parts = chartLoader.GetSongParts();
    for (int playerInt = 0; playerInt < MAX_PLAYERS; playerInt++) {
        if (!ThePlayerManager.ActivePlayers[playerInt])
            continue;
        Player &player = ThePlayerManager.GetActivePlayer(playerInt);

        float LeftOfMenu = Units::wpct(0.025);
        float xPosOfMenu = LeftOfMenu + ((playerInt) * Units::winpct(0.25f));

        auto& menu = Menus[playerInt];
        menu.bgColor = {0x18, 0x18, 0x27, 0xff};
        menu.displayParams.Fnt(ASSET(rubik)).Size(Units::hinpct(SlotState[playerInt] == INSTRUMENT ? 0.04f : 0.03f));
        menu.displayParams.Padding(Units::hinpct(0.01f), Units::hinpct(0.01f));
        menu.displayParams.Pos(xPosOfMenu, BottomOvershell-menu.ItemHeight()*menu.options.size()).Bounds(GetOvershellSlotWidth(), menu.ItemHeight()*menu.options.size());
        menu.Draw();

        if (SlotState[playerInt] != READY) {
            SwitchMenus = false;
        }
    }
    if (SwitchMenus && ThePlayerManager.PlayersActive > 0) {
        ThePlayerManager.SavePlayerList();
        TheMenuManager.CreateAndSwitchMenu<ChartLoadingMenu>(curSong);
    }
    if (GuiButton({ 0, 0, 60, 60 }, "<")) {
        TheMenuManager.CreateAndSwitchMenu<SongSelectMenu>();
    }


    buttReg.DrawPrompts(isOSOpen());
    DrawOvershell();

    for (auto& func : defer) {
        func(this);
    }
    defer.clear();
}

void ReadyUpMenu::Load() {
    curSong->LoadInfoINI(curSong->songInfoPath);
    buttReg.buttMap.clear();
    NEWBUTTONACTION2(buttReg, LANE_1, "generic.select", {
        if (_action != Encore::Action::PRESS) return;
        Encore::ControllerEvent ev;
        ev.action = Encore::Action::PRESS;
        ev.channel = Encore::InputChannel::LANE_1;
        if (slot == -1) {
            for (int playerInt = 0; playerInt < MAX_PLAYERS; playerInt++) {
                if (!ThePlayerManager.ActivePlayers[playerInt])
                    continue;
                Menus[playerInt].Input(ev);
            }
        } else {
            Menus[slot].Input(ev);
        }
    })
    NEWBUTTONACTION2(buttReg, LANE_2, "generic.back", {
        if (_action != Encore::Action::PRESS) return;
        if (slot == -1) {
            for (int playerInt = 0; playerInt < MAX_PLAYERS; playerInt++) {
                if (!ThePlayerManager.ActivePlayers[playerInt])
                    continue;
                Back(playerInt);
            }
            return;
        }
        Back(slot);
    })
    NEWBUTTONACTION2(buttReg, STRUM_UP, "Up", {
        if (_action != Encore::Action::PRESS) return;
        Encore::ControllerEvent ev;
        ev.action = Encore::Action::PRESS;
        ev.channel = Encore::InputChannel::STRUM_UP;
        Menus[slot].Input(ev);
    }, false)
    NEWBUTTONACTION2(buttReg, STRUM_DOWN, "Down", {
        if (_action != Encore::Action::PRESS) return;
        Encore::ControllerEvent ev;
        ev.action = Encore::Action::PRESS;
        ev.channel = Encore::InputChannel::STRUM_DOWN;
        Menus[slot].Input(ev);
    }, false)
    SlotState = { INSTRUMENT, INSTRUMENT, INSTRUMENT, INSTRUMENT };
    auto& Parts = chartLoader.GetSongParts();
    for (size_t i = 0; i < Parts.size(); i++) {
        if (Parts[i].Valid) {
            PartsToDisplay.push_back(i);
        }
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
        LoadMenuState(i, SlotState[i]);
        if (!ThePlayerManager.ActivePlayers[i]) continue;
        auto& player = ThePlayerManager.GetActivePlayer(i);
        for (int g = 0; g < PartsToDisplay.size(); g++) {
            if (player.Instrument == PartsToDisplay[g]) {
                ControllerInstSlot[i] = g;
                break;
            }
        }
        ControllerDiffSlot[i] = player.Difficulty;
        if (!PartsToDisplay.empty() && !Parts[PartsToDisplay[ControllerInstSlot[i]]].Valid) {
            ControllerInstSlot[i] = 0;
        }
        if (!Parts[player.Instrument].ValidDiffs[player.Difficulty]) {
            ControllerDiffSlot[i] = 0;
        }
        if (player.ReplayPlayer) {
            LoadMenuState(i, READY);
        }
    }

    curSong->LoadAlbumArt();
}
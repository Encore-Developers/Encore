//
// Created by marie on 16/11/2024.
//

#include "ReadyUpMenu.h"

#include "ChartLoadingMenu.h"
#include "../overshell/OvershellHelper.h"
#include "../MenuManager.h"
#include "../main/MainMenu.h"
#include "raygui.h"
#include "../styles.h"
#include "../uiUnits.h"
#include "song/ArtLoader.h"
#include "menus/locale/Locale.h"
#include "menus/main/SongSelectMenu.h"

#include "users/playerManager.h"

void ReadyUpMenu::ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        auto playerId = ThePlayerManager.ActivePlayers[i];
        if (playerId == -1)
            continue;
        auto &player = ThePlayerManager.GetActivePlayer(i);
        if (player.joypadID != event.slot)
            continue;
        if (event.slot == std::numeric_limits<unsigned int>::max())
            return;

        buttReg.HandleInput(event);
        if (event.action == Encore::RhythmEngine::Action::PRESS) {
            int diffCount = 0;
            SongPart& part = curSong->parts[player.Instrument];
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

void ReadyUpMenu::Back(int slot) {
    switch (SlotState[slot]) {
    case INSTRUMENT:
        curSong->midiParsed = false;
        TheMenuManager.CreateAndSwitchMenu<SongSelectMenu>();
        break;
    case DIFFICULTY:
        SlotState[slot] = INSTRUMENT;
        break;
    case READY:
        curSong->midiParsed = false;
        TheMenuManager.CreateAndSwitchMenu<SongSelectMenu>();
        ReadyState[slot] = false;
        break;
    }
}

void ReadyUpMenu::Select(int slot) {
    switch (SlotState[slot]) {
    case INSTRUMENT:
        if (!PartsToDisplay.empty()) {
            SlotState[slot] = DIFFICULTY;
            ThePlayerManager.GetActivePlayer(slot).Instrument = PartsToDisplay[ControllerInstSlot[slot]];
        }
        break;
    case DIFFICULTY:
        SlotState[slot] = READY;
        ThePlayerManager.GetActivePlayer(slot).Difficulty = ControllerDiffSlot[slot];
        ReadyState[slot] = true;
        break;
    default:
        break;
    }
}

void ReadyUpMenu::KeyboardInputCallback(SDL_KeyboardEvent* event) {
}

void ReadyUpMenu::DrawDifficulties(float BottomOvershell,
                                   int playerInt,
                                   Player &player,
                                   float xPosOfMenu) {
    Units &u = Units::getInstance();
    for (int i = 0; i < 4; i++) {
        SongPart &part = curSong->parts[player.Instrument];
        if (part.ValidDiffs[i]) {
            Color ButtonColor = backgroundColor;
            if (ControllerDiffSlot[playerInt] == i) {
                ButtonColor = ColorBrightness(AccentColor, -0.25);
            }
            Rectangle pos{
                xPosOfMenu,
                BottomOvershell - u.hinpct(0.05f)
                - (u.hinpct(0.05f) * (float)i),
                u.winpct(0.2f),
                u.hinpct(0.05f)
            };

            if (GuiButton(pos, "") && !isOSOpen()) {
                ControllerDiffSlot[playerInt] = i;
                SlotState[playerInt] = READY;
                ReadyState[playerInt] = true;
                player.Difficulty = i;
                return;
            }

            DrawRectangleRec(pos, ButtonColor);

            Encore::Text::lDrawText(
                ASSET(rubik),
                diffList[i],
                { pos.x + u.hinpct(0.01f), pos.y + u.hinpct(0.01f) },
                u.hinpct(0.03f),
                WHITE,
                LEFT
            );
        } else {
            GuiButton(
                { xPosOfMenu,
                  BottomOvershell - u.hinpct(0.05f)
                  - (u.hinpct(0.05f) * part.diff),
                  u.winpct(0.2f),
                  u.hinpct(0.05f) },
                ""
            );
            DrawRectangle(
                xPosOfMenu + 2,
                BottomOvershell + 2 - u.hinpct(0.05f)
                - (u.hinpct(0.05f) * part.diff),
                u.winpct(0.2f) - 4,
                u.hinpct(0.05f) - 4,
                Color{ 0, 0, 0, 128 }
            );
        }
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
    }
}

void ReadyUpMenu::Draw() {
    bool SwitchMenus = true;
    Assets &assets = Assets::getInstance();
    Units &u = Units::getInstance();
    GameMenu::DrawAlbumArtBackground();
    float AlbumArtLeft = u.LeftSide;
    float AlbumArtTop = u.hpct(0.05f);
    float AlbumArtRight = u.winpct(0.15f);
    float AlbumArtBottom = u.winpct(0.15f);
    DrawRectangle(
        0,
        0,
        (int)GetRenderWidth(),
        (int)GetRenderHeight(),
        GetColor(0x00000080)
    );

    encOS::DrawTopOvershell(0.2f);
    GameMenu::DrawVersion();

    DrawRectangle(
        (int)u.LeftSide,
        (int)AlbumArtTop,
        (int)AlbumArtRight + 12,
        (int)AlbumArtBottom + 12,
        WHITE
    );
    DrawRectangle(
        (int)u.LeftSide + 6,
        (int)AlbumArtTop + 6,
        (int)AlbumArtRight,
        (int)AlbumArtBottom,
        BLACK
    );
    DrawTexturePro(
        TheArtLoader.loadedArt->GetTexture(),
        Rectangle{ 0,
                   0,
                   (float)TheArtLoader.loadedArt->GetTexture().width,
                   (float)TheArtLoader.loadedArt->GetTexture().width },
        Rectangle{ u.LeftSide + 6, AlbumArtTop + 6, AlbumArtRight, AlbumArtBottom },
        { 0, 0 },
        0,
        WHITE
    );

    float BottomOvershell = u.hpct(1) - u.hinpct(0.15f);
    float TextPlacementTB = AlbumArtTop;
    float TextPlacementLR = AlbumArtRight + AlbumArtLeft + 32;
    DrawTextEx(
        assets.redHatDisplayItalic,
        curSong->title.c_str(),
        { TextPlacementLR, TextPlacementTB },
        u.hinpct(0.05f),
        0,
        WHITE
    );
    DrawTextEx(
        assets.rubikItalic,
        curSong->artist.c_str(),
        { TextPlacementLR, TextPlacementTB + u.hinpct(0.05125f) },
        u.hinpct(0.04f),
        0,
        WHITE
    );
    if (!curSong->charters.empty()) {
        DrawTextEx(
            assets.rubikItalic,
            curSong->charters[0].c_str(),
            { TextPlacementLR, TextPlacementTB + u.hinpct(0.095f) },
            u.hinpct(0.04f),
            0,
            WHITE
        );
    }
    // todo: allow this to be run per player
    // load midi
    GameMenu::DrawBottomOvershell();

    for (int playerInt = 0; playerInt < MAX_PLAYERS; playerInt++) {
        if (ThePlayerManager.ActivePlayers[playerInt] == -1)
            continue;
        Player &player = ThePlayerManager.GetActivePlayer(playerInt);

        float LeftOfMenu = u.wpct(0.025);
        float xPosOfMenu = LeftOfMenu + ((playerInt) * u.winpct(0.25f));
        switch (SlotState[playerInt]) {
        case INSTRUMENT: {
            if (PartsToDisplay.empty()) {
                break;
            }

            for (size_t i = 0; i < PartsToDisplay.size(); i++) {
                if (ControllerInstSlot[playerInt] >= PartsToDisplay.size()) {
                    ControllerInstSlot[playerInt] = PartsToDisplay.size() - 1;
                }
                Rectangle pos{ xPosOfMenu,
                               BottomOvershell - u.hinpct(0.05f)
                               - (u.hinpct(0.05f) * (float)i),
                               u.winpct(0.2f),
                               u.hinpct(0.05f) };
                std::string PartAndDiff = std::to_string(
                        curSong->parts[PartsToDisplay[i]].diff + 1) + "/7   ";
                PartAndDiff += LOCALIZE(songPartsList[PartsToDisplay[i]]).toString();
                if (GuiButton(pos, "") && !isOSOpen()) {
                    ControllerInstSlot[playerInt] = i;
                    SlotState[playerInt] = DIFFICULTY;
                    player.Instrument = PartsToDisplay[i];
                }
                GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0xcbcbcbFF);
                GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
                Color ButtonColor = backgroundColor;
                if (ControllerInstSlot[playerInt] == i) {
                    ButtonColor = ColorBrightness(AccentColor, 0.5);
                }
                DrawRectangleRec(pos, ButtonColor);
                Encore::Text::DrawText(
                    assets.rubik,
                    PartAndDiff,
                    { pos.x + u.hinpct(0.01f), pos.y + u.hinpct(0.01f) },
                    u.hinpct(0.03f),
                    WHITE,
                    LEFT
                );
            }
            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
            break;
        }
        case DIFFICULTY: {
            DrawDifficulties(BottomOvershell, playerInt, player, xPosOfMenu);
            break;
        }
        case READY: {
            if (GuiButton(
                { xPosOfMenu,
                  BottomOvershell - u.hinpct(0.05f),
                  u.winpct(0.2f),
                  u.hinpct(0.05f) },
                ""
            ) && !isOSOpen()) {
                SlotState[playerInt] = DIFFICULTY;
                ReadyState[playerInt] = false;
            }
            Encore::Text::lDrawText(
                assets.rubik,
                "generic.difficulty",
                { xPosOfMenu, BottomOvershell - u.hinpct(0.04f) },
                u.hinpct(0.03f),
                WHITE,
                LEFT
            );
            auto difficultyLocalized = LOCALIZE(diffList[player.Difficulty]);
            DrawTextEx(
                assets.rubikBold,
                difficultyLocalized,
                { xPosOfMenu + u.winpct(0.19f)
                  - MeasureTextEx(
                      assets.rubikBold, difficultyLocalized,
                      u.hinpct(0.03f),
                      0
                  )
                  .x,
                  BottomOvershell - u.hinpct(0.04f) },
                u.hinpct(0.03f),
                0,
                WHITE
            );
            if (GuiButton(
                { xPosOfMenu,
                  BottomOvershell - u.hinpct(0.10f),
                  u.winpct(0.2f),
                  u.hinpct(0.05f) },
                ""
            ) && !isOSOpen()) {
                SlotState[playerInt] = INSTRUMENT;
                ReadyState[playerInt] = false;
            }
            Encore::Text::lDrawText(
                assets.rubik,
                "generic.instrument",
                { xPosOfMenu, BottomOvershell - u.hinpct(0.09f) },
                u.hinpct(0.03f),
                WHITE,
                LEFT
            );
            auto text = LOCALIZE(songPartsList[player.Instrument]);
            DrawTextEx(
                assets.rubikBold,
                text,
                { xPosOfMenu + u.winpct(0.19f)
                  - MeasureTextEx(
                      assets.rubikBold,
                      text,
                      u.hinpct(0.03f),
                      0
                  )
                  .x,
                  BottomOvershell - u.hinpct(0.09f) },
                u.hinpct(0.03f),
                0,
                WHITE
            );
            GuiSetStyle(
                BUTTON,
                TEXT_COLOR_NORMAL,
                ColorToInt(Color{ 255, 255, 255, 255 })
            );
            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x751D4AFF);
            GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, 0xA82A6BFF);
            //if (GuiButton(
            //    { xPosOfMenu,
            //      BottomOvershell,
            //      u.winpct(0.2f),
            //      u.hinpct(0.05f) },
            //    "Go Back"
            //)) {
            //    SlotState[playerInt] = INSTRUMENT;
            //    ReadyState[playerInt] = false;
            //}
            GuiSetStyle(
                BUTTON,
                BASE_COLOR_FOCUSED,
                ColorToInt(ColorBrightness(player.AccentColor, -0.5))
            );
            GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0xcbcbcbFF);
            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
        }
        break;
        }
        if (ReadyState[playerInt] == false) {
            SwitchMenus = false;
        }
    }
    if (SwitchMenus && ThePlayerManager.PlayersActive > 0) {
        TheMenuManager.CreateAndSwitchMenu<ChartLoadingMenu>(curSong);
    }
    if (GuiButton({ 0, 0, 60, 60 }, "<")) {
        curSong->midiParsed = false;
        TheMenuManager.CreateAndSwitchMenu<SongSelectMenu>();
    }


    buttReg.DrawPrompts(isOSOpen());
    DrawOvershell();
}

void ReadyUpMenu::Load() {
    buttReg.buttMap.clear();
    NEWBUTTONACTION2(buttReg, LANE_1, "generic.select", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        if (slot == -1) {
            for (int playerInt = 0; playerInt < MAX_PLAYERS; playerInt++) {
                if (ThePlayerManager.ActivePlayers[playerInt] == -1)
                    continue;
                Select(playerInt);
            }
            return;
        }
        Select(slot);
    })
    NEWBUTTONACTION2(buttReg, LANE_2, "generic.back", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        if (slot == -1) {
            for (int playerInt = 0; playerInt < MAX_PLAYERS; playerInt++) {
                if (ThePlayerManager.ActivePlayers[playerInt] == -1)
                    continue;
                Back(playerInt);
            }
            return;
        }
        Back(slot);
    })
    NEWBUTTONACTION2(buttReg, STRUM_UP, "Up", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        switch (SlotState[slot]) {
                case INSTRUMENT:
            ControllerInstSlot[slot]++;
            if (ControllerInstSlot[slot] < 0)
                ControllerInstSlot[slot] = 0;
            break;
        case DIFFICULTY:
            ControllerDiffSlot[slot]++;
            if (ControllerDiffSlot[slot] < 0)
                ControllerDiffSlot[slot] = 0;

            if (ControllerDiffSlot[slot] > 4)
                ControllerDiffSlot[slot] = 4;
            break;
        default:
            break;
        }
    }, false)
    NEWBUTTONACTION2(buttReg, STRUM_DOWN, "Down", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        switch (SlotState[slot]) {
                case INSTRUMENT:
            ControllerInstSlot[slot]--;
            if (ControllerInstSlot[slot] < 0)
                ControllerInstSlot[slot] = 0;
            break;
        case DIFFICULTY:
            ControllerDiffSlot[slot]--;
            if (ControllerDiffSlot[slot] < 0)
                ControllerDiffSlot[slot] = 0;
            break;
        default:
            break;
        }
    }, false)
    SlotState = { INSTRUMENT, INSTRUMENT, INSTRUMENT, INSTRUMENT };
    ReadyState = { false, false, false, false };
    smf::MidiFile midiFile;
    curSong->LoadAlbumArt();
    midiFile.read(curSong->midiPath.string());
    for (int track = 0; track < midiFile.getTrackCount(); track++) {
        SongParts songPart = curSong->GetSongPart(midiFile[track]);
        curSong->IsPartValid(midiFile[track], songPart, track);
        if (songPart == BeatLines) {
            curSong->BeatTrackID = track;
        }
        if (songPart == Events) {
            curSong->getStartEnd(midiFile, track, midiFile[track]);
        }
    }
    if (!curSong->parts[PartGuitar].Valid && curSong->parts[
        PlasticGuitar].Valid) {
        curSong->parts[PartGuitar] = curSong->parts[
            PlasticGuitar];
        curSong->parts[PartGuitar].AutoToPad = true;
    }
    if (!curSong->parts[PartBass].Valid && curSong->parts[
        PlasticBass].Valid) {
        curSong->parts[PartBass] = curSong->parts[PlasticBass];
        curSong->parts[PartBass].AutoToPad = true;
    }
    if (!curSong->parts[PartKeys].Valid && curSong->parts[
        PlasticKeys].Valid) {
        curSong->parts[PartKeys] = curSong->parts[PlasticKeys];
        curSong->parts[PartKeys].AutoToPad = true;
    }

    for (size_t i = 0; i < curSong->parts.size(); i++) {
        if (curSong->parts[i].Valid) {
            PartsToDisplay.push_back(i);
        }
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (ThePlayerManager.ActivePlayers[i] == -1) continue;
        auto& player = ThePlayerManager.GetActivePlayer(i);
        for (int g = 0; g < PartsToDisplay.size(); g++) {
            if (player.Instrument == PartsToDisplay[g]) {
                ControllerInstSlot[i] = g;
                break;
            }
        }
        ControllerDiffSlot[i] = player.Difficulty;
        if (!PartsToDisplay.empty() && !curSong->parts[PartsToDisplay[ControllerInstSlot[i]]].Valid) {
            ControllerInstSlot[i] = 0;
        }
        if (!curSong->parts[player.Instrument].ValidDiffs[player.Difficulty]) {
            ControllerDiffSlot[i] = 0;
        }
    }
}
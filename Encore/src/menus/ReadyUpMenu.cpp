//
// Created by marie on 16/11/2024.
//

#include "ReadyUpMenu.h"

#include "OvershellHelper.h"
#include "MenuManager.h"
#include "gameMenu.h"
#include "raygui.h"
#include "styles.h"
#include "uiUnits.h"

#include "users/playerManager.h"

void ReadyUpMenu::ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event) {

    for (int i = 0; i < 4; i++) {
        auto playerId = ThePlayerManager.ActivePlayers[i];
        if (playerId == -1) continue;
        auto &player = ThePlayerManager.GetActivePlayer(i);
        if (player.joypadID != event.slot) continue;
        if (event.slot == -1) return;
        if (event.action == Encore::RhythmEngine::Action::PRESS) {
            switch (event.channel) {
            case Encore::RhythmEngine::InputChannel::STRUM_UP: {
                switch (SlotState[i]) {
                case INSTRUMENT:
                    ControllerInstSlot[i]++;
                    if (ControllerInstSlot[i] < 0) ControllerInstSlot[i] = 0;
                    break;
                case DIFFICULTY:
                    ControllerDiffSlot[i]++;
                    if (ControllerDiffSlot[i] < 0) ControllerDiffSlot[i] = 0;

                    if (ControllerDiffSlot[i] > 4) ControllerDiffSlot[i] = 4;
                    break;
                }
                break;
            }
            case Encore::RhythmEngine::InputChannel::STRUM_DOWN: {
                switch (SlotState[i]) {
                case INSTRUMENT:
                    ControllerInstSlot[i]--;
                    if (ControllerInstSlot[i] < 0) ControllerInstSlot[i] = 0;
                    break;
                case DIFFICULTY:
                    ControllerDiffSlot[i]--;
                    if (ControllerDiffSlot[i] < 0) ControllerDiffSlot[i] = 0;
                    break;
                }
                break;
            }
            case Encore::RhythmEngine::InputChannel::LANE_1: {
                switch (SlotState[i]) {
                case INSTRUMENT:
                    SlotState[i] = DIFFICULTY;
                    player.Instrument = PartsToDisplay[i][ControllerInstSlot[i]];
                    break;
                case DIFFICULTY:
                    SlotState[i] = READY;
                    player.Difficulty = ControllerDiffSlot[i];
                    ReadyState[i] = true;
                    break;
                }
                break;
            }
            case Encore::RhythmEngine::InputChannel::LANE_2: {
                switch (SlotState[i]) {
                case INSTRUMENT:
                    TheSongList.curSong->midiParsed = false;
                    TheMenuManager.SwitchScreen(SONG_SELECT);
                    break;
                case DIFFICULTY:
                    SlotState[i] = INSTRUMENT;
                    break;
                case READY:
                    TheSongList.curSong->midiParsed = false;
                    TheMenuManager.SwitchScreen(SONG_SELECT);
                    ReadyState[i] = false;
                    break;
                }
            }
            }
        }
    }
}

void ReadyUpMenu::KeyboardInputCallback(int key, int scancode, int action, int mods) {
}

bool ReadyUpMenu::DrawDifficulties(float BottomOvershell,
                                   int playerInt,
                                   Player &player,
                                   float xPosOfMenu) {
    Units &u = Units::getInstance();
    for (int i = 0; i < 4; i++) {
        SongPart *chart = TheSongList.curSong->parts[player.Instrument];
        if (chart->ValidDiffs[i]) {
            Color ButtonColor = backgroundColor;
            if (ControllerDiffSlot[playerInt] == i) {
                ButtonColor = ColorBrightness(AccentColor, -0.25);
            }
            Rectangle pos {
                xPosOfMenu,
                     BottomOvershell - u.hinpct(0.05f)
                     - (u.hinpct(0.05f) * (float)i),
                     u.winpct(0.2f),
                     u.hinpct(0.05f)
            };

            if (GuiButton(pos, "")) {

                ControllerDiffSlot[playerInt] = i;
                SlotState[playerInt] = READY;
                ReadyState[playerInt] = true;
                player.Difficulty = i;
                return true;
            }

            DrawRectangleRec(pos, ButtonColor);

            GameMenu::mhDrawText(
                    ASSET(rubik),
                    diffList[i],
                    { pos.x, pos.y },
                    u.hinpct(0.03f),
                    WHITE,
                    ASSET(sdfShader),
                    LEFT
                );
        } else {
            GuiButton(
                { xPosOfMenu,
                  BottomOvershell - u.hinpct(0.05f)
                  - (u.hinpct(0.05f) * chart->diff),
                  u.winpct(0.2f),
                  u.hinpct(0.05f) },
                ""
            );
            DrawRectangle(
                xPosOfMenu + 2,
                BottomOvershell + 2 - u.hinpct(0.05f)
                - (u.hinpct(0.05f) * chart->diff),
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
    GameMenu::DrawAlbumArtBackground(TheSongList.curSong->albumArtBlur);

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
        TheSongList.curSong->albumArt,
        Rectangle{ 0,
                   0,
                   (float)TheSongList.curSong->albumArt.width,
                   (float)TheSongList.curSong->albumArt.width },
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
        TheSongList.curSong->title.c_str(),
        { TextPlacementLR, TextPlacementTB },
        u.hinpct(0.05f),
        0,
        WHITE
    );
    DrawTextEx(
        assets.rubikItalic,
        TheSongList.curSong->artist.c_str(),
        { TextPlacementLR, TextPlacementTB + u.hinpct(0.05125f) },
        u.hinpct(0.04f),
        0,
        WHITE
    );
    if (!TheSongList.curSong->charters.empty()) {
        DrawTextEx(
            assets.rubikItalic,
            TheSongList.curSong->charters[0].c_str(),
            { TextPlacementLR, TextPlacementTB + u.hinpct(0.095f) },
            u.hinpct(0.04f),
            0,
            WHITE
        );
    }
    // todo: allow this to be run per player
    // load midi
    GameMenu::DrawBottomOvershell();

    for (int playerInt = 0; playerInt < 4; playerInt++) {
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

            for (int i = 0; i < PartsToDisplay[playerInt].size(); i++) {
                if (ControllerInstSlot[playerInt] >= PartsToDisplay[playerInt].size()) {
                    ControllerInstSlot[playerInt] = PartsToDisplay[playerInt].size() - 1;
                }
                Rectangle pos { xPosOfMenu,
                      BottomOvershell - u.hinpct(0.05f)
                      - (u.hinpct(0.05f) * (float)i),
                      u.winpct(0.2f),
                      u.hinpct(0.05f) };
                std::string PartName =
                    TextFormat("  %s", songPartsList[PartsToDisplay[playerInt][i]].c_str());
                std::string PartAndDiff = std::to_string(TheSongList.curSong->parts[PartsToDisplay[playerInt][i]]->diff + 1) + "/7" + PartName;
                if (GuiButton(pos, "")) {
                    ControllerInstSlot[playerInt] = i;
                    SlotState[playerInt] = DIFFICULTY;
                    player.Instrument = PartsToDisplay[playerInt][i];
                }
                GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0xcbcbcbFF);
                GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
                Color ButtonColor = backgroundColor;
                if (ControllerInstSlot[playerInt] == i) {
                    ButtonColor = ColorBrightness(AccentColor, 0.5);
                }
                DrawRectangleRec(pos, ButtonColor);
                GameMenu::mhDrawText(
                    assets.rubik,
                    PartAndDiff,
                    { pos.x, pos.y },
                    u.hinpct(0.03f),
                    WHITE,
                    assets.sdfShader,
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
            )) {
                SlotState[playerInt] = DIFFICULTY;
                ReadyState[playerInt] = false;
            }
            GameMenu::mhDrawText(
                assets.rubik,
                "  Difficulty",
                { xPosOfMenu, BottomOvershell - u.hinpct(0.04f) },
                u.hinpct(0.03f),
                WHITE,
                assets.sdfShader,
                LEFT
            );
            DrawTextEx(
                assets.rubikBold,
                diffList[player.Difficulty].c_str(),
                { xPosOfMenu + u.winpct(0.19f)
                  - MeasureTextEx(
                      assets.rubikBold,
                      diffList[player.Difficulty].c_str(),
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
            )) {
                SlotState[playerInt] = INSTRUMENT;
                ReadyState[playerInt] = false;
            }
            GameMenu::mhDrawText(
                assets.rubik,
                "  Instrument",
                { xPosOfMenu, BottomOvershell - u.hinpct(0.09f) },
                u.hinpct(0.03f),
                WHITE,
                assets.sdfShader,
                LEFT
            );
            DrawTextEx(
                assets.rubikBold,
                songPartsList[player.Instrument].c_str(),
                { xPosOfMenu + u.winpct(0.19f)
                  - MeasureTextEx(
                      assets.rubikBold,
                      songPartsList[player.Instrument].c_str(),
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
            if (GuiButton(
                { xPosOfMenu,
                  BottomOvershell,
                  u.winpct(0.2f),
                  u.hinpct(0.05f) },
                "Go Back"
            )) {
                SlotState[playerInt] = INSTRUMENT;
                ReadyState[playerInt] = false;
            }
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
    if (SwitchMenus) {
        TheMenuManager.SwitchScreen(CHART_LOADING_SCREEN);
    }
    if (GuiButton({ 0, 0, 60, 60 }, "<")) {
        TheSongList.curSong->midiParsed = false;
        TheMenuManager.SwitchScreen(SONG_SELECT);
    }

    DrawOvershell();
}

void ReadyUpMenu::Load() {
    SlotState = { INSTRUMENT, INSTRUMENT, INSTRUMENT, INSTRUMENT };
    ReadyState = { false, false, false, false };
    smf::MidiFile midiFile;
    midiFile.read(TheSongList.curSong->midiPath.string());
    for (int track = 0; track < midiFile.getTrackCount(); track++) {
        SongParts songPart = TheSongList.curSong->GetSongPart(midiFile[track]);
        TheSongList.curSong->IsPartValid(midiFile[track], songPart, track);
        if (songPart == BeatLines) {
            TheSongList.curSong->BeatTrackID = track;
        }
        if (songPart == Events) {
            TheSongList.curSong->getStartEnd(midiFile, track, midiFile[track]);
        }
    }
    for (int playerInt = 0; playerInt < 4; playerInt++) {
        if (ThePlayerManager.ActivePlayers[playerInt] == -1)
            continue;
        for (int i = 0; i < TheSongList.curSong->parts.size(); i++) {
            if (TheSongList.curSong->parts[i]->Valid) {
                PartsToDisplay[playerInt].push_back(i);
            }
        }
    }
}
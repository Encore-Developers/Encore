//
// Created by marie on 16/11/2024.
//

#include "ReadyUpMenu.h"

#include "MenuManager.h"
#include "gameMenu.h"
#include "raygui.h"
#include "uiUnits.h"
#include "gameplay/gameplayRenderer.h"
#include "users/playerManager.h"
void ReadyUpMenu::ControllerInputCallback(int joypadID, GLFWgamepadstate state) {
}
void ReadyUpMenu::KeyboardInputCallback(int key, int scancode, int action, int mods) {

}

SongParts GetSongPart(smf::MidiEventList track) {
    for (int events = 0; events < track.getSize(); events++) {
        std::string trackName;
        if (!track[events].isMeta())
            continue;
        if ((int)track[events][1] == 3) {
            for (int k = 3; k < track[events].getSize(); k++) {
                trackName += track[events][k];
            }
            if (TheSongList.curSong->ini)
                return TheSongList.curSong->partFromStringINI(trackName);
            else
                return TheSongList.curSong->partFromString(trackName);
            break;
        }
    }
}

std::vector<std::vector<int> > pDiffRangeNotes = {
    { 60, 64 }, { 72, 76 }, { 84, 88 }, { 96, 100 }
};

void IsPartValid(smf::MidiEventList track, SongParts songPart, int trackNumber) {
    if (songPart != SongParts::Invalid && songPart != PitchedVocals
        && songPart != BeatLines) {
        for (int diff = 0; diff < 4; diff++) {
            bool StopSearching = false;
            Chart newChart;
            for (int i = 0; i < track.getSize(); i++) {
                if (track[i].isNoteOn() && !track[i].isMeta()
                    && (int)track[i][1] >= pDiffRangeNotes[diff][0]
                    && (int)track[i][1] <= pDiffRangeNotes[diff][1] && !StopSearching) {
                    newChart.valid = true;
                    newChart.diff = diff;
                    newChart.track = trackNumber;
                    TheSongList.curSong->parts[(int)songPart]->hasPart = true;
                    StopSearching = true;
                }
            }
            if (songPart > PartVocals && songPart < PlasticVocals)
                TheSongList.curSong->parts[songPart]->plastic = true;
            if (songPart < PlasticVocals)
                TheSongList.curSong->parts[(int)songPart]->charts.push_back(newChart);
        }
    }
}

void ReadyUpMenu::Draw() {
    Assets &assets = Assets::getInstance();
    Units &u = Units::getInstance();
    GameMenu::DrawAlbumArtBackground(TheSongList.curSong->albumArtBlur);

    float AlbumArtLeft = u.LeftSide;
    float AlbumArtTop = u.hpct(0.05f);
    float AlbumArtRight = u.winpct(0.15f);
    float AlbumArtBottom = u.winpct(0.15f);
    DrawRectangle(
        0, 0, (int)GetScreenWidth(), (int)GetScreenHeight(), GetColor(0x00000080)
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
        Rectangle { 0,
                    0,
                    (float)TheSongList.curSong->albumArt.width,
                    (float)TheSongList.curSong->albumArt.width },
        Rectangle { u.LeftSide + 6, AlbumArtTop + 6, AlbumArtRight, AlbumArtBottom },
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
    DrawTextEx(
        assets.rubikItalic,
        TheSongList.curSong->charters[0].c_str(),
        { TextPlacementLR, TextPlacementTB + u.hinpct(0.095f) },
        u.hinpct(0.04f),
        0,
        WHITE
    );
    // todo: allow this to be run per player
    // load midi
    GameMenu::DrawBottomOvershell();
    for (int playerInt = 0; playerInt < 4; playerInt++) {
        if (ThePlayerManager.ActivePlayers[playerInt] == -1)
            continue;
        Player &player = ThePlayerManager.GetActivePlayer(playerInt);
        if (!TheGameRenderer.midiLoaded && !TheSongList.curSong->midiParsed) {
            smf::MidiFile midiFile;
            midiFile.read(TheSongList.curSong->midiPath.string());
            for (int track = 0; track < midiFile.getTrackCount(); track++) {
                SongParts songPart = GetSongPart(midiFile[track]);
                IsPartValid(midiFile[track], songPart, track);
                if (songPart == BeatLines) {
                    TheSongList.curSong->BeatTrackID = track;
                }
            }

            TheSongList.curSong->midiParsed = true;
            TheGameRenderer.midiLoaded = true;

            if (!player.ReadiedUpBefore
                || !TheSongList.curSong->parts[player.Instrument]->hasPart) {
                player.ReadyUpMenuState = Player::INSTRUMENT;
            } else if (!TheSongList.curSong->parts[player.Instrument]
                            ->charts[player.Difficulty]
                            .valid) {
                player.ReadyUpMenuState = Player::DIFFICULTY;
            } else if (player.ReadiedUpBefore) {
                player.ReadyUpMenuState = Player::PREVIEW;
            }
        } else if (TheGameRenderer.midiLoaded) {
            float LeftOfMenu = u.wpct(0.025);
            float xPosOfMenu = LeftOfMenu + ((playerInt)*u.winpct(0.25f));
            switch (player.ReadyUpMenuState) {
            case Player::INSTRUMENT: {
                if (GuiButton({ 0, 0, 60, 60 }, "<")) {
                    if (!player.ReadiedUpBefore
                        || !TheSongList.curSong->parts[player.Instrument]->hasPart) {
                        player.ReadyUpMenuState = Player::INSTRUMENT;
                        TheGameRenderer.midiLoaded = false;
                        TheMenuManager.SwitchScreen(SONG_SELECT);
                    } else {
                        player.ReadyUpMenuState = Player::PREVIEW;
                    }
                }
                // DrawTextRHDI(TextFormat("%s - %s",
                // TheSongList.curSong->title.c_str(),
                // TheSongList.curSong->artist.c_str()), 70,7, WHITE);
                std::vector<int> PartsToDisplay = {};
                for (int i = 0; i < TheSongList.curSong->parts.size(); i++) {
                    if (TheSongList.curSong->parts[i]->hasPart) {
                        PartsToDisplay.push_back(i);
                    }
                }
                if (PartsToDisplay.empty()) {
                    break;
                }

                for (int i = 0; i < PartsToDisplay.size(); i++) {
                    GuiSetStyle(
                        BUTTON,
                        BASE_COLOR_NORMAL,
                        PartsToDisplay[i] == player.Instrument
                            ? ColorToInt(ColorBrightness(player.AccentColor, -0.25))
                            : 0x181827FF
                    );
                    GuiSetStyle(
                        BUTTON,
                        TEXT_COLOR_NORMAL,
                        ColorToInt(Color { 255, 255, 255, 255 })
                    );
                    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
                    if (GuiButton(
                            { xPosOfMenu,
                              BottomOvershell - u.hinpct(0.05f)
                                  - (u.hinpct(0.05f) * (float)i),
                              u.winpct(0.2f),
                              u.hinpct(0.05f) },
                            TextFormat("  %s", songPartsList[PartsToDisplay[i]].c_str())
                        )) {
                        player.instSelected = true;
                        player.Instrument = PartsToDisplay[i];
                        int isBassOrVocal = 0;
                        if (PartsToDisplay[i] > PartVocals)
                            player.ClassicMode = true;
                        else
                            player.ClassicMode = false;
                        if (player.Instrument == PartBass
                            || player.Instrument == PartVocals
                            || player.Instrument == PlasticBass) {
                            isBassOrVocal = 1;
                        }
                        SetShaderValue(
                            assets.odMultShader,
                            assets.isBassOrVocalLoc,
                            &isBassOrVocal,
                            SHADER_UNIFORM_INT
                        );
                    }
                    GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0xcbcbcbFF);
                    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
                    GameMenu::mhDrawText(
                        assets.rubik,
                        (std::to_string(
                             TheSongList.curSong->parts[PartsToDisplay[i]]->diff + 1
                         )
                         + "/7")
                            .c_str(),
                        { xPosOfMenu + u.winpct(0.165f),
                          BottomOvershell - u.hinpct(0.04f)
                              - (u.hinpct(0.05f) * (float)i) },
                        u.hinpct(0.03f),
                        WHITE,
                        assets.sdfShader,
                        LEFT
                    );
                }
                GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
                if (player.instSelected) {
                    GuiSetStyle(
                        BUTTON,
                        TEXT_COLOR_NORMAL,
                        ColorToInt(Color { 255, 255, 255, 255 })
                    );
                    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x1D754AFF);
                    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, 0x2AA86BFF);
                    if (GuiButton(
                            { xPosOfMenu,
                              BottomOvershell,
                              u.winpct(0.2f),
                              u.hinpct(0.05f) },
                            "Done"
                        )) {
                        if (TheSongList.curSong->parts[player.Instrument]
                                ->charts[player.Difficulty]
                                .notes.empty()) {
                            player.ReadyUpMenuState = Player::DIFFICULTY;
                        } else {
                            player.ReadyUpMenuState = Player::PREVIEW;
                        }
                        break;
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
            case Player::DIFFICULTY: {
                {
                    for (auto &chartDiff :
                         TheSongList.curSong->parts[player.Instrument]->charts) {
                        if (chartDiff.valid) {
                            GuiSetStyle(
                                BUTTON,
                                BASE_COLOR_NORMAL,
                                chartDiff.diff == player.Difficulty && player.diffSelected
                                    ? ColorToInt(
                                          ColorBrightness(player.AccentColor, -0.25)
                                      )
                                    : 0x181827FF
                            );
                            if (GuiButton(
                                    { xPosOfMenu,
                                      BottomOvershell - u.hinpct(0.05f)
                                          - (u.hinpct(0.05f) * chartDiff.diff),
                                      u.winpct(0.2f),
                                      u.hinpct(0.05f) },
                                    diffList[chartDiff.diff].c_str()
                                )) {
                                player.Difficulty = chartDiff.diff;
                                player.diffSelected = true;
                            }
                        } else {
                            GuiButton(
                                { xPosOfMenu,
                                  BottomOvershell - u.hinpct(0.05f)
                                      - (u.hinpct(0.05f) * chartDiff.diff),
                                  u.winpct(0.2f),
                                  u.hinpct(0.05f) },
                                ""
                            );
                            DrawRectangle(
                                xPosOfMenu + 2,
                                BottomOvershell + 2 - u.hinpct(0.05f)
                                    - (u.hinpct(0.05f) * chartDiff.diff),
                                u.winpct(0.2f) - 4,
                                u.hinpct(0.05f) - 4,
                                Color { 0, 0, 0, 128 }
                            );
                        }
                        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
                        if (player.diffSelected) {
                            GuiSetStyle(
                                BUTTON,
                                TEXT_COLOR_NORMAL,
                                ColorToInt(Color { 255, 255, 255, 255 })
                            );
                            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x1D754AFF);
                            GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, 0x2AA86BFF);
                            if (GuiButton(
                                    { xPosOfMenu,
                                      BottomOvershell - u.hinpct(0.25f),
                                      u.winpct(0.2f),
                                      u.hinpct(0.05f) },
                                    "Done"
                                )) {
                                player.ReadyUpMenuState = Player::PREVIEW;
                                player.ReadiedUpBefore = true;
                            }
                            GuiSetStyle(
                                BUTTON,
                                BASE_COLOR_FOCUSED,
                                ColorToInt(ColorBrightness(player.AccentColor, -0.5))
                            );
                            GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0xcbcbcbFF);
                            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
                        }
                        if (GuiButton({ 0, 0, 60, 60 }, "<")) {
                            if (player.ReadiedUpBefore
                                || !TheSongList.curSong->parts[player.Instrument]
                                        ->hasPart) {
                                player.ReadyUpMenuState = Player::INSTRUMENT;
                                player.instSelected = false;
                                player.diffSelected = false;
                            } else {
                                player.ReadyUpMenuState = Player::PREVIEW;
                                player.instSelected = false;
                                player.diffSelected = false;
                            }
                        }
                    }
                }
                break;
            }
            case Player::PREVIEW: {
                {
                    if (GuiButton(
                            { xPosOfMenu,
                              BottomOvershell - u.hinpct(0.05f),
                              u.winpct(0.2f),
                              u.hinpct(0.05f) },
                            ""
                        )) {
                        player.ReadyUpMenuState = Player::DIFFICULTY;
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
                        player.ReadyUpMenuState = Player::INSTRUMENT;
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
                        ColorToInt(Color { 255, 255, 255, 255 })
                    );
                    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x1D754AFF);
                    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, 0x2AA86BFF);
                    if (GuiButton(
                            { xPosOfMenu,
                              BottomOvershell,
                              u.winpct(0.2f),
                              u.hinpct(0.05f) },
                            "Ready Up!"
                        )) {
                        player.ReadyUpMenuState = Player::PREVIEW;
                        // TheGameRenderer.highwayInAnimation = false;
                        // TheGameRenderer.songStartTime = GetTime();
                        TheMenuManager.SwitchScreen(CHART_LOADING_SCREEN);
                    }
                    GuiSetStyle(
                        BUTTON,
                        BASE_COLOR_FOCUSED,
                        ColorToInt(ColorBrightness(player.AccentColor, -0.5))
                    );
                    GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0xcbcbcbFF);
                    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
                }
            } break;
            }
        }

        if (GuiButton({ 0, 0, 60, 60 }, "<")) {
            TheGameRenderer.midiLoaded = false;
            TheSongList.curSong->midiParsed = false;
            TheMenuManager.SwitchScreen(SONG_SELECT);
        }
    }
    DrawOvershell();
}

void ReadyUpMenu::Load() {}
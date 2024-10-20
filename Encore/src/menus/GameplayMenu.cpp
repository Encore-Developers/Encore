//
// Created by marie on 20/10/2024.
//

#include "GameplayMenu.h"

#include "gameMenu.h"
#include "overshellRenderer.h"
#include "uiUnits.h"
#include "song/audio.h"
#include "song/songList.h"
#include "raymath.h"
#include "raygui.h"
#include "settings-old.h"
#include "gameplay/enctime.h"
#include "styles.h"
#include "easing/easing.h"
#include "gameplay/gameplayRenderer.h"
#include "users/playerManager.h"

#include <raylib.h>

GameplayMenu::GameplayMenu() {}
GameplayMenu::~GameplayMenu() {}

void GameplayMenu::Draw() {
    AudioManager &audioManager = AudioManager::getInstance();
    Units &u = Units::getInstance();
    Assets &assets = Assets::getInstance();
    SettingsOld &settings = SettingsOld::getInstance();
    
    OvershellRenderer osr;
    double curTime = GetTime();

    // IMAGE BACKGROUNDS??????
    ClearBackground(BLACK);
    int width = float(GetScreenWidth());
    int height = float(GetScreenHeight());

    float scorePos = u.RightSide - u.hinpct(0.01f);
    float scoreY = u.hpct(0.15f);
    float starY = scoreY + u.hinpct(0.065f);
    float comboY = starY + u.hinpct(0.055f);

    GameMenu::DrawAlbumArtBackground(TheSongList.curSong->albumArtBlur);
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color { 0, 0, 0, 128 });
    // DrawTextureEx(assets.songBackground, {0,0},0,
    // (float)GetScreenHeight()/assets.songBackground.height,WHITE);

    int starsval = ThePlayerManager.BandStats.Stars();
    float starPercent =
        (float)ThePlayerManager.BandStats.Score / (float)ThePlayerManager.BandStats.BaseScore;
    for (int i = 0; i < 5; i++) {
        bool firstStar = (i == 0);
        float starX = scorePos - u.hinpct(0.26) + (i * u.hinpct(0.0525));
        float starWH = u.hinpct(0.05);
        Rectangle emptyStarWH = {
            0, 0, (float)assets.emptyStar.width, (float)assets.emptyStar.height
        };
        Rectangle starRect = { starX, starY, starWH, starWH };
        DrawTexturePro(assets.emptyStar, emptyStarWH, starRect, { 0, 0 }, 0, WHITE);
        float yMaskPos = Remap(
            starPercent,
            firstStar ? 0 : ThePlayerManager.BandStats.xStarThreshold[i - 1],
            ThePlayerManager.BandStats.xStarThreshold[i],
            0,
            u.hinpct(0.05)
        );
        BeginScissorMode(starX, (starY + starWH) - yMaskPos, starWH, yMaskPos);
        DrawTexturePro(
            assets.star,
            emptyStarWH,
            starRect,
            { 0, 0 },
            0,
            i == starsval ? Color { 192, 192, 192, 128 } : WHITE
        );
        EndScissorMode();
    }
    if (starPercent >= ThePlayerManager.BandStats.xStarThreshold[4]
        && ThePlayerManager.BandStats.EligibleForGoldStars) {
        float starWH = u.hinpct(0.05);
        Rectangle emptyStarWH = {
            0, 0, (float)assets.goldStar.width, (float)assets.goldStar.height
        };
        float yMaskPos = Remap(
            starPercent,
            ThePlayerManager.BandStats.xStarThreshold[4],
            ThePlayerManager.BandStats.xStarThreshold[5],
            0,
            u.hinpct(0.05)
        );
        BeginScissorMode(
            scorePos - (starWH * 6), (starY + starWH) - yMaskPos, scorePos, yMaskPos
        );
        for (int i = 0; i < 5; i++) {
            float starX = scorePos - u.hinpct(0.26) + (i * u.hinpct(0.0525));
            Rectangle starRect = { starX, starY, starWH, starWH };
            DrawTexturePro(
                ThePlayerManager.BandStats.GoldStars ? assets.goldStar
                                                  : assets.goldStarUnfilled,
                emptyStarWH,
                starRect,
                { 0, 0 },
                0,
                WHITE
            );
        }
        EndScissorMode();
    }
    // int totalScore =
    // 		player.score + player.sustainScoreBuffer[0] +
    // player.sustainScoreBuffer[ 			1] + player.sustainScoreBuffer[2] +
    // player.sustainScoreBuffer[3]
    // 		+ player.sustainScoreBuffer[4];

    int Score = 0;
    int Combo = 0;
    /*
    for (int player = 0; player < ThePlayerManager.PlayersActive; player++) {
        Score += ThePlayerManager.GetActivePlayer(player)->stats->Score;
        Combo += ThePlayerManager.GetActivePlayer(player)->stats->Combo;
    }
    ThePlayerManager.BandStats.Score = Score;
    ThePlayerManager.BandStats.Combo = Combo;
*/

    Rectangle scoreboxSrc {
        0, 0, float(assets.Scorebox.width), float(assets.Scorebox.height)
    };
    float WidthOfScorebox = u.hinpct(0.28);
    // float scoreY = u.hpct(0.15f);
    float ScoreboxX = u.RightSide;
    float ScoreboxY = u.hpct(0.1425f);
    float HeightOfScorebox = WidthOfScorebox / 4;
    Rectangle scoreboxDraw { ScoreboxX, ScoreboxY, WidthOfScorebox, HeightOfScorebox };
    DrawTexturePro(
        assets.Scorebox, scoreboxSrc, scoreboxDraw, { WidthOfScorebox, 0 }, 0, WHITE
    );
    Rectangle TimerboxSrc {
        0, 0, float(assets.Timerbox.width), float(assets.Timerbox.height)
    };
    float WidthOfTimerbox = u.hinpct(0.14);
    // float scoreY = u.hpct(0.15f);
    float TimerboxX = u.RightSide;
    float TimerboxY = u.hpct(0.1425f);
    float HeightOfTimerbox = WidthOfTimerbox / 4;
    Rectangle TimerboxDraw { TimerboxX, TimerboxY, WidthOfTimerbox, HeightOfTimerbox };
    DrawTexturePro(
        assets.Timerbox,
        TimerboxSrc,
        TimerboxDraw,
        { WidthOfTimerbox, HeightOfTimerbox },
        0,
        WHITE
    );
    int played = TheSongTime.GetSongTime();
    int length = TheSongTime.GetSongLength();
    float Width = Remap(played, 0, length, 0, WidthOfTimerbox);
    BeginScissorMode(
        TimerboxX - WidthOfTimerbox,
        TimerboxY - HeightOfTimerbox,
        Width + 1,
        HeightOfTimerbox + 1
    );
    DrawTexturePro(
        assets.TimerboxOutline,
        TimerboxSrc,
        TimerboxDraw,
        { WidthOfTimerbox, HeightOfTimerbox },
        0,
        WHITE
    );
    EndScissorMode();
    GameMenu::mhDrawText(
        assets.redHatDisplayItalicLarge,
        GameMenu::scoreCommaFormatter(ThePlayerManager.BandStats.Score),
        { u.RightSide - u.winpct(0.015f), scoreY },
        u.hinpct(0.053),
        Color { 107, 161, 222, 255 },
        assets.sdfShader,
        RIGHT
    );
    float bandMult = u.RightSide - WidthOfScorebox;
    GameMenu::mhDrawText(
        assets.redHatDisplayItalicLarge,
        TextFormat(
            "%01ix",
            ThePlayerManager.BandStats
                .OverdriveMultiplier[ThePlayerManager.BandStats.PlayersInOverdrive]
        ),
        { bandMult, scoreY },
        u.hinpct(0.05),
        RAYWHITE,
        assets.sdfShader,
        RIGHT
    );

    int playedMinutes = played / 60;
    int playedSeconds = played % 60;
    int songMinutes = length / 60;
    int songSeconds = length % 60;
    const char *textTime = TextFormat(
        "%i:%02i / %i:%02i", playedMinutes, playedSeconds, songMinutes, songSeconds
    );
    GameMenu::mhDrawText(
        assets.rubik,
        textTime,
        { u.RightSide - u.winpct(0.013f), scoreY - u.hinpct(SmallHeader) },
        u.hinpct(SmallHeader * 0.66),
        WHITE,
        assets.sdfShader,
        RIGHT
    );
    if (!TheGameRenderer.streamsLoaded) {
        audioManager.loadStreams(TheSongList.curSong->stemsPath);
        for (int i = 0; i < ThePlayerManager.PlayersActive; i++) {
            for (auto &stream : audioManager.loadedStreams) {
                Player *player = ThePlayerManager.GetActivePlayer(i);
                if ((player->ClassicMode ? player->Instrument - 5 : player->Instrument)
                    == stream.instrument) {
                    audioManager.SetAudioStreamVolume(
                        stream.handle,
                        player->stats->Mute ? settings.MainVolume * settings.MissVolume
                                            : settings.MainVolume * settings.PlayerVolume
                    );
                } else {
                    audioManager.SetAudioStreamVolume(
                        stream.handle, settings.MainVolume * settings.BandVolume
                    );
                }
            }
        }
        TheGameRenderer.streamsLoaded = true;

        // player.resetPlayerStats();
    } else {
        for (int i = 0; i < ThePlayerManager.PlayersActive; i++) {
            for (auto &stream : audioManager.loadedStreams) {
                Player *player = ThePlayerManager.GetActivePlayer(i);
                if ((player->ClassicMode ? player->Instrument - 5 : player->Instrument)
                    == stream.instrument) {
                    audioManager.SetAudioStreamVolume(
                        stream.handle,
                        player->stats->Mute ? settings.MainVolume * settings.MissVolume
                                            : settings.MainVolume * settings.PlayerVolume
                    );
                } else {
                    audioManager.SetAudioStreamVolume(
                        stream.handle, settings.MainVolume * settings.BandVolume
                    );
                }
            }
        }

        float songPlayed = audioManager.GetMusicTimePlayed();

        if (TheSongTime.SongComplete()) {
            TheGameRenderer.LowerHighway();
        }
        if (TheSongTime.SongComplete()) {
            TheSongList.curSong->LoadAlbumArt();
            TheGameRenderer.midiLoaded = false;
            TheGameRenderer.highwayInAnimation = false;
            TheGameRenderer.songPlaying = false;
            TheGameRenderer.highwayLevel = 0;
            TheSongTime.Stop();
            if (TheGameRenderer.streamsLoaded) {
                audioManager.unloadStreams();
                TheGameRenderer.streamsLoaded = false;
            }
            TheGameMenu.SwitchScreen(RESULTS);
            Encore::EncoreLog(LOG_INFO, TextFormat("Song ended at at %f", songPlayed));
            return;
        }
    }

    for (int pnum = 0; pnum < ThePlayerManager.PlayersActive; pnum++) {
        switch (ThePlayerManager.PlayersActive) {
        case (1): {
            TheGameRenderer.cameraSel = 0;
            TheGameRenderer.renderPos = 0;
            break;
        }
        case (2): {
            if (pnum == 0) {
                TheGameRenderer.cameraSel = 1;
                TheGameRenderer.renderPos = GetScreenWidth() / 8;
            } else {
                TheGameRenderer.cameraSel = 0;
                TheGameRenderer.renderPos = -GetScreenWidth() / 8;
            }
            break;
        }
        case (3): {
            if (pnum == 0) {
                TheGameRenderer.cameraSel = 2;
                TheGameRenderer.renderPos = GetScreenWidth() / 4;
            } else if (pnum == 1) {
                TheGameRenderer.cameraSel = 0;
                TheGameRenderer.renderPos = 0;
            } else if (pnum == 2) {
                TheGameRenderer.cameraSel = 1;
                TheGameRenderer.renderPos = -GetScreenWidth() / 4;
            }
            break;
        }
        case (4): {
            if (pnum == 0) {
                TheGameRenderer.cameraSel = 2;
                TheGameRenderer.renderPos = GetScreenWidth() / 4;
            } else if (pnum == 1) {
                TheGameRenderer.cameraSel = 3;
                TheGameRenderer.renderPos = GetScreenWidth() / 12;
            } else if (pnum == 2) {
                TheGameRenderer.cameraSel = 0;
                TheGameRenderer.renderPos = -GetScreenWidth() / 12;
            } else if (pnum == 3) {
                TheGameRenderer.cameraSel = 1;
                TheGameRenderer.renderPos = -GetScreenWidth() / 4;
            }
            break;
        }
        }
        TheGameRenderer.RenderGameplay(
            ThePlayerManager.GetActivePlayer(pnum), TheSongTime.GetSongTime(), *TheSongList.curSong
        );
        float CenterPosForText =
            GetWorldToScreen(
                { 0, 0, 0 },
                TheGameRenderer.cameraVectors[ThePlayerManager.PlayersActive - 1][TheGameRenderer.cameraSel]
            )
                .x;
        float fontSize = u.hinpct(0.025);
        float textWidth = MeasureTextEx(
                              assets.rubikBold,
                              ThePlayerManager.GetActivePlayer(pnum)->Name.c_str(),
                              fontSize,
                              0
        )
                              .x;
        DrawTextEx(
            assets.rubikBold,
            ThePlayerManager.GetActivePlayer(pnum)->Name.c_str(),
            { (CenterPosForText - (textWidth / 2)) - (TheGameRenderer.renderPos),
              GetScreenHeight() - u.hinpct(0.03) },
            fontSize,
            0,
            WHITE
        );
    }

    float SongNameWidth = MeasureTextEx(
                              assets.rubikBoldItalic,
                              TheSongList.curSong->title.c_str(),
                              u.hinpct(MediumHeader),
                              0
    )
                              .x;
    std::string SongArtistString =
        TheSongList.curSong->artist + ", " + std::to_string(TheSongList.curSong->releaseYear);
    float SongArtistWidth =
        MeasureTextEx(
            assets.rubikBoldItalic, SongArtistString.c_str(), u.hinpct(SmallHeader), 0
        )
            .x;

    float SongExtrasWidth = MeasureTextEx(
                                assets.rubikBoldItalic,
                                TheSongList.curSong->charters[0].c_str(),
                                u.hinpct(SmallHeader),
                                0
    )
                                .x;

    double SongNameDuration = 0.75f;
    unsigned char SongNameAlpha = 255;
    float SongNamePosition = 35;
    unsigned char SongArtistAlpha = 255;
    float SongArtistPosition = 35;
    unsigned char SongExtrasAlpha = 255;
    float SongExtrasPosition = 35;
    float SongNameBackgroundWidth =
        SongNameWidth >= SongArtistWidth ? SongNameWidth : SongArtistWidth;
    float SongBackgroundWidth = SongNameBackgroundWidth;
    if (curTime > TheSongTime.GetStartTime() + 7.5
        && curTime < TheSongTime.GetStartTime() + 7.5 + SongNameDuration) {
        double timeSinceStart = GetTime() - (TheSongTime.GetStartTime() + 7.5);
        SongNameAlpha = static_cast<unsigned char>(Remap(
            static_cast<float>(
                getEasingFunction(EaseOutCirc)(timeSinceStart / SongNameDuration)
            ),
            0,
            1.0,
            255,
            0
        ));
        SongNamePosition = Remap(
            static_cast<float>(
                getEasingFunction(EaseInOutBack)(timeSinceStart / SongNameDuration)
            ),
            0,
            1.0,
            35,
            -SongNameWidth
        );
    } else if (curTime > TheSongTime.GetStartTime() + 7.5 + SongNameDuration)
        SongNameAlpha = 0;

    if (curTime > TheSongTime.GetStartTime() + 7.75
        && curTime < TheSongTime.GetStartTime() + 7.75 + SongNameDuration) {
        double timeSinceStart = GetTime() - (TheSongTime.GetStartTime() + 7.75);
        SongArtistAlpha = static_cast<unsigned char>(Remap(
            static_cast<float>(
                getEasingFunction(EaseOutCirc)(timeSinceStart / SongNameDuration)
            ),
            0,
            1.0,
            255,
            0
        ));

        SongArtistPosition = Remap(
            static_cast<float>(
                getEasingFunction(EaseInOutBack)(timeSinceStart / SongNameDuration)
            ),
            0,
            1.0,
            35,
            -SongArtistWidth
        );
    }
    if (curTime > TheSongTime.GetStartTime() + 8
        && curTime < TheSongTime.GetStartTime() + 8 + SongNameDuration) {
        double timeSinceStart = GetTime() - (TheSongTime.GetStartTime() + 8);
        SongExtrasAlpha = static_cast<unsigned char>(Remap(
            static_cast<float>(
                getEasingFunction(EaseOutCirc)(timeSinceStart / SongNameDuration)
            ),
            0,
            1.0,
            255,
            0
        ));
        SongBackgroundWidth = Remap(
            static_cast<float>(
                getEasingFunction(EaseInOutCirc)(timeSinceStart / SongNameDuration)
            ),
            0,
            1.0,
            SongNameBackgroundWidth,
            0
        );

        SongExtrasPosition = Remap(
            static_cast<float>(
                getEasingFunction(EaseInOutBack)(timeSinceStart / SongNameDuration)
            ),
            0,
            1.0,
            35,
            -SongExtrasWidth
        );
    }
    if (curTime < TheSongTime.GetStartTime() + 7.75 + SongNameDuration) {
        DrawRectangleGradientH(
            0,
            u.hpct(0.19f),
            1.25 * SongBackgroundWidth,
            u.hinpct(0.02f + MediumHeader + SmallHeader + SmallHeader),
            Color { 0, 0, 0, 128 },
            Color { 0, 0, 0, 0 }
        );
        DrawTextEx(
            assets.rubikBoldItalic,
            TheSongList.curSong->title.c_str(),
            { SongNamePosition, u.hpct(0.2f) },
            u.hinpct(MediumHeader),
            0,
            Color { 255, 255, 255, SongNameAlpha }
        );
        DrawTextEx(
            assets.rubikItalic,
            SongArtistString.c_str(),
            { SongArtistPosition, u.hpct(0.2f + MediumHeader) },
            u.hinpct(SmallHeader),
            0,
            Color { 200, 200, 200, SongArtistAlpha }
        );
        DrawTextEx(
            assets.rubikItalic,
            TheSongList.curSong->charters[0].c_str(),
            { SongExtrasPosition, u.hpct(0.2f + MediumHeader + SmallHeader) },
            u.hinpct(SmallHeader),
            0,
            Color { 200, 200, 200, SongExtrasAlpha }
        );
    }

    int songLength;
    if (TheSongList.curSong->end == 0)
        songLength = static_cast<int>(audioManager.GetMusicTimeLength());
    else
        songLength = static_cast<int>(TheSongList.curSong->end);

    GuiSetStyle(PROGRESSBAR, BORDER_WIDTH, 0);
    // GuiSetStyle(PROGRESSBAR, BASE_COLOR_NORMAL,
    //			ColorToInt(player.FC ? GOLD : AccentColor));
    // GuiSetStyle(PROGRESSBAR, BASE_COLOR_FOCUSED,
    //			ColorToInt(player.FC ? GOLD : AccentColor));
    // GuiSetStyle(PROGRESSBAR, BASE_COLOR_DISABLED,
    //			ColorToInt(player.FC ? GOLD : AccentColor));
    // GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED,
    //			ColorToInt(player.FC ? GOLD : AccentColor));
    GuiSetStyle(DEFAULT, TEXT_SIZE, static_cast<int>(u.hinpct(0.03f)));
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    GuiSetFont(assets.rubik);

    float floatSongLength = audioManager.GetMusicTimePlayed();

    if (!ThePlayerManager.BandStats.Multiplayer) {
        Player *player = ThePlayerManager.GetActivePlayer(0);
        PlayerGameplayStats *stats = player->stats;
        if (player->stats->Paused) {
            DrawRectangle(
                0, 0, GetScreenWidth(), GetScreenHeight(), Color { 0, 0, 0, 80 }
            );
            osr.DrawTopOvershell(0.2f);
            GuiSetStyle(DEFAULT, TEXT_SIZE, (int)u.hinpct(0.08f));
            GuiSetFont(assets.redHatDisplayBlack);
            GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
            GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0xaaaaaaFF);
            GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, 0xFFFFFFFF);
            GuiSetStyle(BUTTON, BORDER_WIDTH, 0);
            GuiSetStyle(BUTTON, BACKGROUND_COLOR, 0);
            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x00000000);
            GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, 0x00000000);
            GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, 0x00000000);
            GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, 0x00000000);
            GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, 0x00000000);
            GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, 0x00000000);
            GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0x00000000);

            if (GuiButton(
                    { u.wpct(0.02f), u.hpct(0.3f), u.winpct(0.2f), u.hinpct(0.08f) },
                    "Resume"
                )) {
                audioManager.unpauseStreams();
                TheSongTime.Resume();
                stats->Paused = false;
            }

            if (GuiButton(
                    { u.wpct(0.02f), u.hpct(0.39f), u.winpct(0.2f), u.hinpct(0.08f) },
                    "Restart"
                )) {
                TheSongTime.Reset();
                TheSongList.curSong->parts[player->Instrument]
                    ->charts[player->Difficulty]
                    .restartNotes();

                stats->Overdrive = false;
                stats->overdriveFill = 0.0f;
                stats->overdriveActiveFill = 0.0f;
                stats->overdriveActiveTime = 0.0;
                stats->overdriveActivateTime = 0.0f;
                TheGameRenderer.highwayInAnimation = false;
                TheGameRenderer.highwayInEndAnim = false;
                TheGameRenderer.songPlaying = false;
                TheGameRenderer.Restart = true;
                stats->curODPhrase = 0;
                stats->curNoteInt = 0;
                stats->curSolo = 0;
                stats->curNoteIdx = { 0, 0, 0, 0, 0 };
                stats->curBeatLine = 0;
                player->ResetGameplayStats();
                assets.expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture =
                    assets.highwayTexture;
                assets.emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture =
                    assets.highwayTexture;
                assets.multBar.materials[0].maps[MATERIAL_MAP_EMISSION].texture =
                    assets.odMultFill;
                assets.multCtr3.materials[0].maps[MATERIAL_MAP_EMISSION].texture =
                    assets.odMultFill;
                assets.multCtr5.materials[0].maps[MATERIAL_MAP_EMISSION].texture =
                    assets.odMultFill;
                assets.expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color =
                    DARKGRAY;
                assets.emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = DARKGRAY;
                stats->Paused = false;
            }
            if (GuiButton(
                    { u.wpct(0.02f), u.hpct(0.48f), u.winpct(0.2f), u.hinpct(0.08f) },
                    "Drop Out"
                )) {
                // notes =
                // TheSongList.curSong->parts[instrument]->charts[diff].notes.size();
                // notes = TheSongList.curSong->parts[instrument]->charts[diff];
                TheGameMenu.SwitchScreen(RESULTS);
                TheSongList.curSong->LoadAlbumArt();
                stats->Overdrive = false;
                stats->overdriveFill = 0.0f;
                stats->overdriveActiveFill = 0.0f;
                stats->overdriveActiveTime = 0.0;
                stats->overdriveActivateTime = 0.0f;
                stats->curNoteInt = 0;
                stats->curODPhrase = 0;
                stats->curSolo = 0;
                stats->Paused = false;
                TheGameRenderer.midiLoaded = false;
                TheSongTime.Reset();
                TheSongList.curSong->parts[player->Instrument]
                    ->charts[player->Difficulty]
                    .resetNotes();
                audioManager.unloadStreams();
                player->stats->Quit = true;
                TheGameRenderer.highwayInAnimation = false;
                TheGameRenderer.highwayInEndAnim = false;
                TheGameRenderer.songPlaying = false;
                GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, 0xFFFFFFFF);
                GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, 0xFFFFFFFF);
                GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, 0xFFFFFFFF);
                GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0x505050ff);
                GuiSetStyle(BUTTON, BORDER_WIDTH, 2);
                GuiSetFont(assets.rubik);
                GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
                return;
            }
            GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, 0xFFFFFFFF);
            GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, 0xFFFFFFFF);
            GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, 0xFFFFFFFF);
            GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0x505050ff);
            GuiSetStyle(BUTTON, BORDER_WIDTH, 2);
            GuiSetFont(assets.rubik);
            GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
            GuiSetStyle(
                BUTTON,
                BASE_COLOR_FOCUSED,
                ColorToInt(ColorBrightness(Color { 255, 0, 255, 255 }, -0.5))
            );
            GuiSetStyle(
                BUTTON,
                BASE_COLOR_PRESSED,
                ColorToInt(ColorBrightness(Color { 255, 0, 255, 255 }, -0.3))
            );

            DrawTextEx(
                assets.rubikBoldItalic,
                "PAUSED",
                { u.wpct(0.02f), u.hpct(0.05f) },
                u.hinpct(0.1f),
                0,
                WHITE
            );

            float SongFontSize = u.hinpct(0.03f);

            const char *instDiffText = TextFormat(
                "%s %s",
                diffList[player->Difficulty].c_str(),
                songPartsList[player->Instrument].c_str()
            );

            float TitleHeight = MeasureTextEx(
                                    assets.rubikBoldItalic,
                                    TheSongList.curSong->title.c_str(),
                                    SongFontSize,
                                    0
            )
                                    .y;
            float TitleWidth = MeasureTextEx(
                                   assets.rubikBoldItalic,
                                   TheSongList.curSong->title.c_str(),
                                   SongFontSize,
                                   0
            )
                                   .x;
            float ArtistHeight =
                MeasureTextEx(
                    assets.rubikItalic, TheSongList.curSong->artist.c_str(), SongFontSize, 0
                )
                    .y;
            float ArtistWidth =
                MeasureTextEx(
                    assets.rubikItalic, TheSongList.curSong->artist.c_str(), SongFontSize, 0
                )
                    .x;
            float InstDiffHeight =
                MeasureTextEx(assets.rubikBold, instDiffText, SongFontSize, 0).y;
            float InstDiffWidth =
                MeasureTextEx(assets.rubikBold, instDiffText, SongFontSize, 0).x;

            Vector2 SongTitleBox = { u.RightSide - TitleWidth - u.winpct(0.01f),
                                     u.hpct(0.1f) - (ArtistHeight / 2)
                                         - (TitleHeight * 1.1f) };
            Vector2 SongArtistBox = { u.RightSide - ArtistWidth - u.winpct(0.01f),
                                      u.hpct(0.1f) - (ArtistHeight / 2) };
            Vector2 SongInstDiffBox = { u.RightSide - InstDiffWidth - u.winpct(0.01f),
                                        u.hpct(0.1f) + (ArtistHeight / 2)
                                            + (InstDiffHeight * 0.1f) };

            DrawTextEx(
                assets.rubikBoldItalic,
                TheSongList.curSong->title.c_str(),
                SongTitleBox,
                SongFontSize,
                0,
                WHITE
            );
            DrawTextEx(
                assets.rubikItalic,
                TheSongList.curSong->artist.c_str(),
                SongArtistBox,
                SongFontSize,
                0,
                WHITE
            );
            DrawTextEx(
                assets.rubikBold, instDiffText, SongInstDiffBox, SongFontSize, 0, WHITE
            );
        }
    }

    GameMenu::DrawFPS(u.LeftSide, u.hpct(0.0025f) + u.hinpct(0.025f));
    GameMenu::DrawVersion();

    // if (!TheGameRenderer.bot)
    //	DrawTextEx(assets.rubikBold, TextFormat("%s", player.FC ? "FC" : ""),
    //				{5, GetScreenHeight() - u.hinpct(0.05f)}, u.hinpct(0.04), 0,
    //				GOLD);
    // if (TheGameRenderer.bot)
    //	DrawTextEx(assets.rubikBold, "BOT",
    //				{5, GetScreenHeight() - u.hinpct(0.05f)}, u.hinpct(0.04), 0,
    //				SKYBLUE);
    // if (!TheGameRenderer.bot)
    GuiProgressBar(
        Rectangle { 0,
                    (float)GetScreenHeight() - u.hinpct(0.005f),
                    (float)GetScreenWidth(),
                    u.hinpct(0.01f) },
        "",
        "",
        &floatSongLength,
        0,
        (float)songLength
    );
    /*
    std::string ScriptDisplayString = "";
    lua.script_file("scripts/testing.lua");
    ScriptDisplayString = lua["TextDisplay"];
    DrawTextEx(assets.rubikBold, ScriptDisplayString.c_str(),
                                    {5, GetScreenHeight() - u.hinpct(0.1f)},
    u.hinpct(0.04), 0, GOLD);

    DrawRectangle(u.wpct(0.5f) - (u.winpct(0.12f) / 2), u.hpct(0.02f) -
    u.winpct(0.01f), u.winpct(0.12f), u.winpct(0.065f),DARKGRAY);

    for (int fretBox = 0; fretBox < TheGameRenderer.heldFrets.size(); fretBox++) {
        float leftInputBoxSize = (5 * u.winpct(0.02f)) / 2;

        Color fretColor;
        switch (fretBox) {
            default:
                fretColor = BROWN;
                break;
            case (0):
                fretColor = GREEN;
                break;
            case (1):
                fretColor = RED;
                break;
            case (2):
                fretColor = YELLOW;
                break;
            case (3):
                fretColor = BLUE;
                break;
            case (4):
                fretColor = ORANGE;
                break;
        }

        DrawRectangle(u.wpct(0.5f) - leftInputBoxSize + (fretBox *
    u.winpct(0.02f)), u.hpct(0.02f), u.winpct(0.02f), u.winpct(0.02f),
                    TheGameRenderer.heldFrets[fretBox] ? fretColor : GRAY);
    }
    DrawRectangle(u.wpct(0.5f) - ((5 * u.winpct(0.02f)) / 2),
                u.hpct(0.02f) + u.winpct(0.025f), u.winpct(0.1f), u.winpct(0.01f),
                TheGameRenderer.upStrum ? WHITE : GRAY);
    DrawRectangle(u.wpct(0.5f) - ((5 * u.winpct(0.02f)) / 2),
                u.hpct(0.02f) + u.winpct(0.035f), u.winpct(0.1f), u.winpct(0.01f),
                TheGameRenderer.downStrum ? WHITE : GRAY);
    */
    DrawTextEx(
        assets.rubik,
        TextFormat("song time: %f", TheSongTime.GetSongTime()),
        { 0, u.hpct(0.5f) },
        u.hinpct(SmallHeader),
        0,
        WHITE
    );
    DrawTextEx(
        assets.rubik,
        TextFormat("audio time: %f", audioManager.GetMusicTimePlayed()),
        { 0, u.hpct(0.5f + SmallHeader) },
        u.hinpct(SmallHeader),
        0,
        WHITE
    );
}

void GameplayMenu::Load() {
    TheSongList.curSong->LoadAlbumArt();
    if (ThePlayerManager.PlayersActive > 1) {
        ThePlayerManager.BandStats.Multiplayer = true;
        for (int player = 0; player < ThePlayerManager.PlayersActive; player++) {
            ThePlayerManager.GetActivePlayer(player)->stats->Multiplayer = true;
        }
    } else {
        ThePlayerManager.BandStats.Multiplayer = false;
        for (int player = 0; player < ThePlayerManager.PlayersActive; player++) {
            ThePlayerManager.GetActivePlayer(player)->stats->Multiplayer = false;
        }
    }

    for (int i = 0; i < ThePlayerManager.PlayersActive; i++) {
        if (i == 0) {
            ThePlayerManager.BandStats.BaseScore =
                TheSongList.curSong->parts[ThePlayerManager.GetActivePlayer(i)->Instrument]
                    ->charts[ThePlayerManager.GetActivePlayer(i)->Difficulty]
                    .baseScore;
        } else {
            ThePlayerManager.BandStats.BaseScore +=
                TheSongList.curSong->parts[ThePlayerManager.GetActivePlayer(i)->Instrument]
                    ->charts[ThePlayerManager.GetActivePlayer(i)->Difficulty]
                    .baseScore;
        }
    }
}
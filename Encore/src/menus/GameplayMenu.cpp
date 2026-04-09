//
// Created by marie on 20/10/2024.
//

#include "GameplayMenu.h"

#include "gameMenu.h"
#include "overshellRenderer.h"
#include "uiUnits.h"
#include "song/audio.h"
#include "song/songlist.h"
#include "raymath.h"
#include "raygui.h"
#include "gameplay/enctime.h"
#include "styles.h"
#include "easing/easing.h"

#include "users/playerManager.h"
#include "MenuManager.h"
#include "OvershellHelper.h"
#include "../settings/settings.h"
#include "timingvalues.h"

#include <raylib.h>

#include "settings/keybinds.h"
#include "tracy/Tracy.hpp"

GameplayMenu::GameplayMenu() {
    hasOvershell = false;
}

GameplayMenu::~GameplayMenu() {
}

void GameplayMenu::KeyboardInputCallback(int key, int scancode, int action, int mods) {
    for (auto track : tracks) {
        if (!track) {
            continue;
        }
        Player &player = track->player;
        if (player.joypadID != -1 && player.joypadID != -2) {
            continue;
        }
        Encore::RhythmEngine::BaseEngine *engine = player.engine.get();

        if (action == 2)
            return;
        Encore::RhythmEngine::ControllerEvent event;
        if (action == GLFW_PRESS) {
            event.action = Encore::RhythmEngine::Action::PRESS;
        } else if (action == GLFW_RELEASE) {
            event.action = Encore::RhythmEngine::Action::RELEASE;
        }
        if (key == TheGameKeybinds.overdriveBinds.first || key == TheGameKeybinds.
            overdriveBinds.second) {
            event.channel = Encore::RhythmEngine::InputChannel::OVERDRIVE;
            }
        if (player.bindingType != PAD) {
            if (key == TheGameKeybinds.strumBinds.first) {
                event.channel = Encore::RhythmEngine::InputChannel::STRUM_UP;
            } else if (key == TheGameKeybinds.strumBinds.second) {
                event.channel = Encore::RhythmEngine::InputChannel::STRUM_DOWN;
            }
        }
        int DiffMax = (player.Difficulty == 3 || player.Instrument > PartVocals) ? 5 : 4;
        for (int i = 0; i < DiffMax; i++) {
            if (key == TheGameKeybinds.keybinds5k[i] || key == TheGameKeybinds.keybinds5kalt[
                i]) {
                event.channel = Encore::RhythmEngine::IntIC(i);
                }
        }
        if (key == KEY_ESCAPE && action == GLFW_PRESS) {
            event.channel = Encore::RhythmEngine::InputChannel::PAUSE;
        }
        event.timestamp = TheSongTime.GetElapsedTime();
        if (event.channel != Encore::RhythmEngine::InputChannel::INVALID)
            engine->ProcessInput(event);
    }
};

void GameplayMenu::ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event) {
    for (auto track : tracks) {
        if (!track) {
            continue;
        }
        Player &player = track->player;
        if (player.joypadID != -2 && player.joypadID != event.slot) {
            continue;
        }
        Encore::RhythmEngine::BaseEngine *engine = player.engine.get();

        if (engine->allowTimestampedInputs) {
            if (engine->IsWithinPracticeSection(event.timestamp) || !engine->practice)
                engine->UpdateOnFrame(event.timestamp);
        }
        engine->ProcessInput(event);
    }
};

void GameplayMenu::DrawScorebox(Units &u, Assets &assets, float scoreY) {
    Rectangle scoreboxSrc{
        0, 0, float(assets.Scorebox.width), float(assets.Scorebox.height)
    };
    float WidthOfScorebox = u.hinpct(0.28);
    // float scoreY = u.hpct(0.15f);
    float ScoreboxX = u.RightSide;
    float ScoreboxY = u.hpct(0.1425f);
    float HeightOfScorebox = WidthOfScorebox / 4;
    Rectangle scoreboxDraw{ ScoreboxX, ScoreboxY, WidthOfScorebox, HeightOfScorebox };
    DrawTexturePro(
        assets.Scorebox,
        scoreboxSrc,
        scoreboxDraw,
        { WidthOfScorebox, 0 },
        0,
        WHITE
    );

    GameMenu::mhDrawText(
        assets.redHatMono,
        GameMenu::scoreCommaFormatter(
            ThePlayerManager.GetActivePlayer(0).engine->stats->Score
        ),
        { u.RightSide - u.winpct(0.0145f), scoreY + u.hinpct(0.0025) },
        u.hinpct(0.05),
        Color{ 107, 161, 222, 255 },
        assets.sdfShader,
        RIGHT
    );
}

void GameplayMenu::DrawTimerbox(Units &u, Assets &assets, float scoreY) {
    Rectangle TimerboxSrc{
        0, 0, float(assets.Timerbox.width), float(assets.Timerbox.height)
    };
    float WidthOfTimerbox = u.hinpct(0.14);
    // float scoreY = u.hpct(0.15f);
    float TimerboxX = u.RightSide;
    float TimerboxY = u.hpct(0.1425f);
    float HeightOfTimerbox = WidthOfTimerbox / 4;
    Rectangle TimerboxDraw{ TimerboxX, TimerboxY, WidthOfTimerbox, HeightOfTimerbox };
    DrawTexturePro(
        assets.Timerbox,
        TimerboxSrc,
        TimerboxDraw,
        { WidthOfTimerbox, HeightOfTimerbox },
        0,
        WHITE
    );
    int played = TheSongTime.GetElapsedTime();
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
    int playedMinutes = played / 60;
    int playedSeconds = played % 60;
    int songMinutes = length / 60;
    int songSeconds = length % 60;
    const char *textTime = TextFormat(
        "%i:%02i / %i:%02i",
        playedMinutes,
        playedSeconds,
        songMinutes,
        songSeconds
    );
    GameMenu::mhDrawText(
        assets.rubik,
        textTime,
        { u.RightSide - (WidthOfTimerbox / 2), scoreY - u.hinpct(SmallHeader) },
        u.hinpct(SmallHeader * 0.66),
        WHITE,
        assets.sdfShader,
        CENTER
    );
}

void GameplayMenu::DrawGameplayStars(
    Units &u,
    Assets &assets,
    float scorePos,
    float starY
) {
    // todo: redo for band
    auto &player = ThePlayerManager.GetActivePlayer(0);
    int inst = player.Instrument % 5;
    int diff = player.Difficulty;
    double starPercent = player.engine->stats->StarThresholdValue;
    int starsVal = player.engine->stats->Stars;
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
            firstStar ? 0 : STAR_THRESHOLDS[inst][i - 1],
            STAR_THRESHOLDS[inst][i],
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
            i == starsVal ? Color{ 192, 192, 192, 128 } : WHITE
        );
        EndScissorMode();
    }
    if (starPercent >= STAR_THRESHOLDS[inst][4]) {
        float starWH = u.hinpct(0.05);
        Rectangle emptyStarWH = {
            0, 0, (float)assets.goldStar.width, (float)assets.goldStar.height
        };
        float yMaskPos = Remap(
            starPercent,
            STAR_THRESHOLDS[inst][4],
            STAR_THRESHOLDS[inst][5],
            0,
            u.hinpct(0.05)
        );
        BeginScissorMode(
            scorePos - (starWH * 6),
            (starY + starWH) - yMaskPos,
            scorePos,
            yMaskPos
        );
        for (int i = 0; i < 5; i++) {
            float starX = scorePos - u.hinpct(0.26) + (i * u.hinpct(0.0525));
            Rectangle starRect = { starX, starY, starWH, starWH };
            DrawTexturePro(
                starPercent >= STAR_THRESHOLDS[inst][5]
                ? assets.goldStar
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
}

unsigned char BeatToCharViaTickThing(
    int tick,
    int MinBrightness,
    int MaxBrightness,
    int QuarterNoteLength
) {
    float TickModulo = tick % QuarterNoteLength;
    return Remap(
        TickModulo / float(QuarterNoteLength),
        0,
        1.0f,
        MaxBrightness,
        MinBrightness
    );
}

bool songPlaying = false;

double GetNotePos(double noteTime, double songTime, float length, float end) {
    return ((noteTime - songTime) * (length * 2.5)) - end;
}

void GameplayMenu::Draw() {
    Units &u = Units::getInstance();
    Assets &assets = Assets::getInstance();
    TheSongTime.UpdateTick();
    TheSongTime.UpdateOverdriveTick();
    ClearBackground(BLACK);
    unsigned char BackgroundColor = 0;
    if (!songPlaying) {
        TheSongTime.Reset();
        double songEnd = floor(TheAudioManager.GetMusicTimeLength());
        TheSongTime.Start(songEnd);
        TheAudioManager.BeginPlayback(TheAudioManager.loadedStreams[0].handle);
        songPlaying = true;
    }
    std::array<Color, 5> grybo = { GREEN, RED, YELLOW, BLUE, ORANGE };
    std::array<Color, 5> orybg = { ORANGE, RED, YELLOW, BLUE, GREEN };
    GameMenu::DrawAlbumArtBackground(TheSongList.curSong->albumArtBlur);
    DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), Color{ 0, 0, 0, 128 });
    DrawRectangle(
        0,
        0,
        GetRenderWidth(),
        GetRenderHeight(),
        Color{ 255, 255, 255, BackgroundColor }
    );
    double EndTime = TheSongList.curSong->end == 0.0 ? TheSongTime.GetSongLength() : TheSongList.curSong->end;
    if (TheSongTime.GetElapsedTime() > EndTime - 1) {
        // TODO: endgame
        TheSongTime.Reset();
        TheAudioManager.unloadStreams();
        songPlaying = false;
        TheSongTime.Beatlines.erase(
            TheSongTime.Beatlines.begin(),
            TheSongTime.Beatlines.end()
        );
        TheSongTime.OverdriveTicks.erase(
            TheSongTime.OverdriveTicks.begin(),
            TheSongTime.OverdriveTicks.end()
        );
        TheSongTime.TimeSigChanges.erase(
            TheSongTime.TimeSigChanges.begin(),
            TheSongTime.TimeSigChanges.end()
        );
        TheSongTime.BPMChanges.erase(
            TheSongTime.BPMChanges.begin(),
            TheSongTime.BPMChanges.end()
        );
        TheSongTime.Lyrics.erase(
            TheSongTime.Lyrics.begin(),
            TheSongTime.Lyrics.end()
        );
        TheSongTime.LastTick = 0;
        TheSongTime.CurrentTick = 0;
        TheSongTime.LastODTick = 0;
        TheSongTime.CurrentODTick = 0;
        TheSongTime.CurrentBPM = 0;
        TheSongTime.CurrentODTickItr = 0;
        TheSongTime.CurrentTimeSig = 0;
        TheSongTime.CurrentBeatline = 0;
        TheSongTime.CurrentLyricPhrase = 0;
        TheMenuManager.SwitchScreen(RESULTS);
        return;
    }

    int denom = TheSongTime.TimeSigChanges.at(TheSongTime.CurrentTimeSig).denom;
    int numer = TheSongTime.TimeSigChanges.at(TheSongTime.CurrentTimeSig).numer;
    int flashInterval = (numer * 480) / denom;

    if (!TheSongTime.Lyrics.empty()) {
        ZoneScopedN("Lyrics Display")
        float baselineVox = u.hpct(0.0025f) + u.hinpct(0.05f);
        float voxHeight = u.hinpct(0.06f);
        float FontSize = u.hinpct(0.0425f);
        float padding = (voxHeight - FontSize) / 2;
        // god forbid i have a proper ui library
        float LyricLeft = 0;
        std::string playedText = "";
        std::string unplayedText = "";
        Color lyricColor = WHITE;
        auto font = ASSETPTR(rubikBold);
        for (const auto& lyric : TheSongTime.Lyrics.at(TheSongTime.CurrentLyricPhrase).lyrics) {
            if (lyric.StartSec < TheSongTime.GetElapsedTime()) {
                playedText += lyric.Lyric;
            } else {
                unplayedText += lyric.Lyric;
            }
            // also this needs to be fixed (talkies dont display properly)
            //if (lyric.talkie)
            //    font = ASSETPTR(rubikBoldItalic);
        }
        // before i even CONSIDER going further with things like animations or fade or ANYTHING. this should be put somewhere but here.
        Rectangle imageRec {0, 0, float(ASSET(mainLyricBar).width), float(ASSET(mainLyricBar).height)};
        Rectangle dest {0, baselineVox, float(GetScreenWidth()), voxHeight};
        LyricLeft += (GetScreenWidth() / 2) - (MeasureTextEx(*font, (playedText + unplayedText).c_str(), FontSize, 0).x / 2);
        DrawTexturePro(ASSET(mainLyricBar), imageRec, dest, {0, 0}, 0, WHITE);
        GameMenu::mhDrawText(*font, playedText, {LyricLeft, baselineVox + padding}, FontSize, {119,183,255,255}, ASSET(sdfShader), LEFT);
        LyricLeft += MeasureTextEx(*font, playedText.c_str(), FontSize, 0).x;
        GameMenu::mhDrawText(*font, unplayedText, {LyricLeft, baselineVox + padding}, FontSize, WHITE, ASSET(sdfShader), LEFT);

        if (TheSongTime.CurrentLyricPhrase < TheSongTime.Lyrics.size() - 1) {
            ZoneScopedN("Secondary Lyrics Display")
            float topBelow = baselineVox + voxHeight;
            float SecFontSize = FontSize * 0.75f;
            float SecVoxSize = voxHeight * 0.75f;
            float secPadding = (SecVoxSize - SecFontSize) / 2;
            std::string lyricStr = "";
            font = ASSETPTR(rubik);
            for (const auto& lyric : TheSongTime.Lyrics.at(TheSongTime.CurrentLyricPhrase+1).lyrics) {
                lyricStr += lyric.Lyric;
            }
            imageRec = {0, 0, float(ASSET(secLyricBar).width), float(ASSET(secLyricBar).height)};
            dest = {0, topBelow, float(GetScreenWidth()), SecVoxSize};
            LyricLeft = (GetScreenWidth() / 2) - (MeasureTextEx(*font, (lyricStr).c_str(), SecFontSize, 0).x / 2);
            DrawTexturePro(ASSET(secLyricBar), imageRec, dest, {0, 0}, 0, WHITE);
            GameMenu::mhDrawText(*font, lyricStr, {LyricLeft, topBelow + secPadding}, SecFontSize, LIGHTGRAY, ASSET(sdfShader), LEFT);
        }

        if (TheSongTime.Lyrics.at(TheSongTime.CurrentLyricPhrase).EndSec < TheSongTime.GetElapsedTime()) {
            if (TheSongTime.CurrentLyricPhrase < TheSongTime.Lyrics.size() - 1) {
                TheSongTime.CurrentLyricPhrase++;
            }
        }
    }


    for (int i = 0; i < ThePlayerManager.PlayersActive; i++) {
        Player &player = ThePlayerManager.GetActivePlayer(i);

        {
            ZoneScopedN("Engine Update")
            if (player.engine->IsWithinPracticeSection(TheSongTime.GetElapsedTime()) || !player.engine->practice) {}
                player.engine->UpdateOnFrame(TheSongTime.GetElapsedTime());
            player.engine->UpdateStats(player.Instrument, player.Difficulty);
        }

        tracks.at(i)->Draw();
        int TopOfScreen = GetRenderHeight(); // width
        int FakeStrikeline = (TopOfScreen / 5) * 4;
        constexpr int NoteXWidth = 150;
        constexpr int NoteHeight = 25;

        int mospos =
            ((GetRenderWidth() + (ThePlayerManager.PlayersActive * NoteXWidth * 5))
                / (1 + ThePlayerManager.PlayersActive))
            - ((ThePlayerManager.PlayersActive * NoteXWidth * 5) / 2);
        int MiddleOfScreen = mospos + (mospos * i); // height
        int TrackLeft = MiddleOfScreen - (NoteXWidth / 2) - NoteXWidth - NoteXWidth;
        auto chart = player.engine->chart;
        int SidesWidth = 20;
        int RailWidth = SidesWidth / 2;

        if (player.engine.get()->stats.get()->AudioMuted) {
            int InstrumentNum =
                player.Instrument % 5;
            if (TheAudioManager.GetAudioStreamByInstrument(InstrumentNum) == nullptr)
                break;
            TheAudioManager.GetAudioStreamByInstrument(InstrumentNum)->volume =
                TheGameSettings.avMainVolume * TheGameSettings.avMuteVolume;
        } else {
            int InstrumentNum =
                player.Instrument % 5;
            if (TheAudioManager.GetAudioStreamByInstrument(InstrumentNum) == nullptr)
                break;
            TheAudioManager.GetAudioStreamByInstrument(InstrumentNum)->volume =
                TheGameSettings.avMainVolume * TheGameSettings.avActiveInstrumentVolume;
        }
    }
    TheAudioManager.UpdateAudioStreamVolumes();

    float scorePos = u.RightSide - u.hinpct(0.01f);
    float scoreY = u.hpct(0.15f);
    float starY = scoreY + u.hinpct(0.065f);
    // Draw Stars
    DrawGameplayStars(u, assets, scorePos, starY);

    // Draw Timerbox
    DrawTimerbox(u, assets, scoreY);

    // Score Drawing
    DrawScorebox(u, assets, scoreY);

    float SongNameWidth = MeasureTextEx(
            assets.rubikBoldItalic,
            TheSongList.curSong->title.c_str(),
            u.hinpct(MediumHeader),
            0
        )
        .x;
    std::string SongArtistString =
        TheSongList.curSong->artist + ", " + TheSongList.curSong->releaseYear;
    float SongArtistWidth =
        MeasureTextEx(
            assets.rubikBoldItalic,
            SongArtistString.c_str(),
            u.hinpct(SmallHeader),
            0
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

    // please God smite this code. flip a few bits in my hard drive. please get rid of this shit somehow
    // there's better ways. forgive me for I have sinned

    if (TheSongTime.GetElapsedTime() > TheSongTime.GetStartTime() + 7.5
        && TheSongTime.GetElapsedTime() < TheSongTime.GetStartTime() + 7.5 + SongNameDuration) {
        double timeSinceStart = TheSongTime.GetElapsedTime() - (TheSongTime.GetStartTime() + 7.5);
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
    } else if (TheSongTime.GetElapsedTime() > TheSongTime.GetStartTime() + 7.5 + SongNameDuration)
        SongNameAlpha = 0;

    if (TheSongTime.GetElapsedTime() > TheSongTime.GetStartTime() + 7.75
        && TheSongTime.GetElapsedTime() < TheSongTime.GetStartTime() + 7.75 + SongNameDuration) {
        double timeSinceStart = TheSongTime.GetElapsedTime() - (TheSongTime.GetStartTime() + 7.75);
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


    if (TheSongTime.GetElapsedTime() > TheSongTime.GetStartTime() + 8
        && TheSongTime.GetElapsedTime() < TheSongTime.GetStartTime() + 8 + SongNameDuration) {
        double timeSinceStart = TheSongTime.GetElapsedTime() - (TheSongTime.GetStartTime() + 8);
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
    if (TheSongTime.GetElapsedTime() < TheSongTime.GetStartTime() + 7.75 + SongNameDuration) {
        DrawRectangleGradientH(
            0,
            u.hpct(0.19f),
            1.25 * SongBackgroundWidth,
            u.hinpct(0.02f + MediumHeader + SmallHeader + SmallHeader),
            Color{ 0, 0, 0, 128 },
            Color{ 0, 0, 0, 0 }
        );
        DrawTextEx(
            assets.rubikBoldItalic,
            TheSongList.curSong->title.c_str(),
            { SongNamePosition, u.hpct(0.2f) },
            u.hinpct(MediumHeader),
            0,
            Color{ 255, 255, 255, SongNameAlpha }
        );
        DrawTextEx(
            assets.rubikItalic,
            SongArtistString.c_str(),
            { SongArtistPosition, u.hpct(0.2f + MediumHeader) },
            u.hinpct(SmallHeader),
            0,
            Color{ 200, 200, 200, SongArtistAlpha }
        );
        DrawTextEx(
            assets.rubikItalic,
            TheSongList.curSong->charters[0].c_str(),
            { SongExtrasPosition, u.hpct(0.2f + MediumHeader + SmallHeader) },
            u.hinpct(SmallHeader),
            0,
            Color{ 200, 200, 200, SongExtrasAlpha }
        );
    }

    GuiSetStyle(PROGRESSBAR, BORDER_WIDTH, 0);
    GuiSetStyle(DEFAULT, TEXT_SIZE, static_cast<int>(u.hinpct(0.03f)));
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    GuiSetFont(assets.rubik);

    GameMenu::DrawFPS(u.LeftSide, u.hpct(0.0025f) + u.hinpct(0.025f));
    GameMenu::DrawVersion();
}

void GameplayMenu::Load() {
    ZoneScoped;
    TheSongList.curSong->LoadAlbumArt();
    TheAudioManager.loadStreams(TheSongList.curSong->stemsPath);
    TheSongTime.SetOffset(TheGameSettings.AudioOffset / 1000.0);

    // i dont like the game stuttering when you active or get a streak
    float widthPerPlayer = 2.0f / ThePlayerManager.PlayersActive;

    for (int i = 0; i < ThePlayerManager.PlayersActive; i++) {
        ZoneScopedN("Player Init")
        Player &player = ThePlayerManager.GetActivePlayer(i);
        tracks.at(i) = std::make_shared<Encore::Track>(player);
        tracks.at(i)->Load();
        tracks.at(i)->ColumnLeft = -1 + widthPerPlayer * i;
        tracks.at(i)->ColumnRight = -1 + widthPerPlayer * (i + 1);
        switch (player.Instrument) {
        case PlasticGuitar:
        case PlasticBass:
        case PlasticKeys:
            tracks.at(i)->Configure5Lane();
            break;
        case PlasticDrums:
            tracks.at(i)->ConfigureDrums();
            break;
        default:
            if (player.Difficulty == 3) {
                tracks.at(i)->Configure5Lane();
            } else {
                tracks.at(i)->Configure4Lane();
            }
        }
        if (player.Instrument == PlasticBass || player.Instrument == PartVocals
            || player.Instrument == PartBass) {
            player.engine->stats->SixMultiplier = true;
        }
        if (player.Bot)
            player.engine->stats->Bot = true;
    }
    for (auto &stream : TheAudioManager.loadedStreams) {
        stream.volume =
            TheGameSettings.avMainVolume * TheGameSettings.avInactiveInstrumentVolume;
    }
    /*
    if (ThePlayerManager.PlayersActive > 1) {
        ThePlayerManager.BandStats->Multiplayer = true;
        for (int player = 0; player < ThePlayerManager.PlayersActive; player++) {
            ThePlayerManager.GetActivePlayer(player).stats->Multiplayer = true;
        }
    } else {
        ThePlayerManager.BandStats->Multiplayer = false;
        for (int player = 0; player < ThePlayerManager.PlayersActive; player++) {
            ThePlayerManager.GetActivePlayer(player).stats->Multiplayer = false;
        }
    }

    for (int i = 0; i < ThePlayerManager.PlayersActive; i++) {
        Player &player = ThePlayerManager.GetActivePlayer(i);
        // player.stats->BaseScore = TheSongList.curSong->parts[player.Instrument]
        //                               ->charts[player.Difficulty]
        //                               .baseScore;
        if (i == 0) {
            ThePlayerManager.BandStats->BaseScore = player.stats->BaseScore;
        } else {
            ThePlayerManager.BandStats->BaseScore += player.stats->BaseScore;
        }
    }
    */
}
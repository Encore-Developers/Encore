//
// Created by marie on 17/11/2024.
//

#include "ChartLoadingMenu.h"

#include "GameplayMenu.h"
#include "PracticeMenu.h"
#include "../MenuManager.h"
#include "menus/util/GameMenu.h"
#include "../util/uiUnits.h"
#include "RhythmEngine/engines.h"
#include "gameplay/enctime.h"
#include "tracy/Tracy.hpp"
#include "users/playerManager.h"
#include "../overshell/OvershellHelper.h"

#include <thread>

#include "menus/util/locale/Locale.h"
#include "../../RhythmEngine/ChartLoaders/MidiLoaders/MidiLyricLoader.h"
#include "RhythmEngine/ChartLoaders/MidiLoaders/Drums/MidiDrumsLoader.h"
#include "RhythmEngine/ChartLoaders/MidiLoaders/Guitar/MidiGuitarLoader.h"
#include "RhythmEngine/ChartLoaders/MidiLoaders/Pad/MidiPadLoader.h"
#include "RhythmEngine/ChartLoaders/PadConverters/PadConverters.h"

bool StartLoading = true;
std::atomic_bool FinishedLoading = false;

void ChartLoadingMenu::LoadCharts() {
    ZoneScoped;
    {
        ZoneScopedN("Loading Beatmap")
        TheSongTime.BPMChanges = chartLoader.GetBPMChanges();
        TheSongTime.TimeSigChanges = chartLoader.GetTimeSigChanges();
        TheSongTime.songPPQN = chartLoader.GetResolution();
        TheSongTime.GenerateBeatmap(chartLoader.GetEndEvent().first);
    }
    chartLoader.LoadCharts();
    TheSongTime.Sections = chartLoader.GetSections();
    chartLoader.PopulateSubsongs();

    for (int playerNum = 0; playerNum < ThePlayerManager.ActivePlayers.size(); playerNum++) {
        if (!ThePlayerManager.ActivePlayers[playerNum])
            continue;
        ZoneScopedN("RhythmEngine ctors")
        Player &player = ThePlayerManager.GetActivePlayer(playerNum);
        player.engine = chartLoader.CreateEngine(&player);
    }
    TheSongTime.Lyrics = chartLoader.GetLyricPhrases();
    FinishedLoading = true;
}

/**
 * @brief Load chart, create new player
 */
void ChartLoadingMenu::Load() {
    ZoneScoped;
    // for (int playerNum = 0; playerNum < ThePlayerManager.PlayersActive; playerNum++) {
    //    ThePlayerManager.GetActivePlayer(playerNum).stats = new PlayerGameplayStats(
    //        ThePlayerManager.GetActivePlayer(playerNum).Difficulty,
    //        ThePlayerManager.GetActivePlayer(playerNum).Instrument
    //    );
    // }
    // ThePlayerManager.BandStats = new BandGameplayStats;
    TheSongTime.FullReset();
    curSong->LoadAlbumArt();
    TheAssets.loadingPool.SubmitTask([this]() {
        TheAudioManager.unloadStreams();
        TheAudioManager.loadStreams(curSong->LoadAudioINI());
#ifndef NO_VIDEO
        auto videoPath = curSong->GetVideoPath();
        if (!videoPath.empty()) {
            videoBackground = std::make_shared<VideoBackground>(videoPath, curSong->videoStartTime);
            videoBackground->selfPtr = videoBackground;
        }
#endif
        LoadCharts();
    });
    // std::thread ChartLoader(LoadCharts);
    // ChartLoader.detach();
    gameplaySet.StartLoad();
}

void ChartLoadingMenu::Draw() {

    Assets &assets = Assets::getInstance();

    ClearBackground(BLACK);
    GameMenu::DrawAlbumArtBackground();
    GameMenu::DrawTopOvershell(0.15f);
    Encore::Text::lDrawText(
        assets.redHatDisplayBlack,
        "chartLoading.header",
        { Units::LeftSide, Units::hpct(0.05f) },
        Units::hinpct(0.125f),
        WHITE,
        LEFT
    );
    float AfterLoadingTextPos =
        MeasureTextEx(assets.redHatDisplayBlack, "LOADING...  ", Units::hinpct(0.125f), 0).x;

    std::string LoadingPhrase = curSong->loadingPhrase.empty()
        ? LOCALIZE("chartLoading.loadingPhrase").toString()
        : curSong->loadingPhrase;

    Encore::Text::DrawText(
        assets.rubikBold,
        LoadingPhrase.c_str(),
        { Units::LeftSide + AfterLoadingTextPos + Units::winpct(0.02f), Units::hpct(0.09f) },
        Units::hinpct(0.05f),
        LIGHTGRAY,
        LEFT
    );

    GameMenu::DrawBottomOvershell();
    //DrawOvershell();

    if (FinishedLoading) {
        // TheGameRenderer.LoadGameplayAssets();
        FinishedLoading = false;
        StartLoading = true;
        switch (gamemode) {
        case GAMEPLAY:
            TheMenuManager.CreateAndSwitchMenu<GameplayMenu>(curSong, videoBackground);
            break;
        case PRACTICE:
            TheMenuManager.CreateAndSwitchMenu<PracticeMenu>(curSong);
            break;
        }
    }
}

//
// Created by marie on 17/11/2024.
//

#include "ChartLoadingMenu.h"

#include "GameplayMenu.h"
#include "PracticeMenu.h"
#include "../MenuManager.h"
#include "../main/MainMenu.h"
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
    // TheSongList.curSong->getTiming(midiFile, 0, midiFile[0]);
    // TheSongList.curSong->parseBeatLines(midiFile, TheSongList.curSong->BeatTrackID);
    {
        ZoneScopedN("Loading Beatmap")
        TheSongTime.BPMChanges = chartLoader.GetBPMChanges();
        TheSongTime.TimeSigChanges = chartLoader.GetTimeSigChanges();
        TheSongTime.songPPQN = chartLoader.GetResolution();
        TheSongTime.GenerateBeatmap(chartLoader.GetEndEvent().first);
    }
    chartLoader.LoadCharts();
    TheSongTime.Sections = chartLoader.GetSections();

    for (int playerNum = 0; playerNum < MAX_PLAYERS; playerNum++) {
        if (!ThePlayerManager.ActivePlayers[playerNum])
            continue;
        ZoneScopedN("RhythmEngine ctors")
        Player &player = ThePlayerManager.GetActivePlayer(playerNum);
        int diff = player.Difficulty;
        int inst = player.Instrument;

        // i hate this shit i dunno why it didnt work

        while (!chartLoader.IsLoaded()) {
            if (chartLoader.IsLoaded()) break;
        }
        std::string trackName;
        if (inst < PitchedVocals && inst != PlasticDrums && inst > PartVocals) {
            player.engine =
                std::make_shared<Encore::RhythmEngine::GuitarEngine>(
                    std::make_shared<Encore::RhythmEngine::BaseChart>(
                        chartLoader.GetChart(SongPart(inst), diff)),
                    std::make_shared<Encore::RhythmEngine::GuitarStats>(0),
                    &player
                );
            player.engine->stats->Type = Encore::RhythmEngine::Guitar;
        } else if (inst == PlasticDrums) {
            player.engine = std::make_shared<Encore::RhythmEngine::DrumsEngine>(
                std::make_shared<Encore::RhythmEngine::BaseChart>(
                    chartLoader.GetChart(SongPart(inst), diff)),
                std::make_shared<Encore::RhythmEngine::DrumsStats>(0),
                &player
            );
            player.engine->stats->Type = Encore::RhythmEngine::Drums;
        } else if (inst < PlasticDrums) {
            player.engine = std::make_shared<Encore::RhythmEngine::PadEngine>(
                    std::make_shared<Encore::RhythmEngine::BaseChart>(
                        chartLoader.GetChart(SongPart(inst), diff)),
                    std::make_shared<Encore::RhythmEngine::PadStats>(0),
                    &player
                );
            player.engine->stats->Type = Encore::RhythmEngine::Pad;
            if (diff < 3)
                player.engine->chart->Lanes.
                                 resize(4);
        }
        for (int i = 0; i < player.engine->chart->Lanes.size(); i++) {

            player.engine->chart->at(i).shrink_to_fit();
            player.engine->chart->CurrentNoteIterators.at(i) =
                player.engine->chart->at(i).begin();
        }
        player.engine->stats->overdrive.ticks = chartLoader.GenerateOverdriveTicks();
        player.engine->chart->sections = chartLoader.GetSections();
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
        auto videoPath = curSong->GetVideoPath();
        if (!videoPath.empty()) {
            videoBackground = std::make_shared<VideoBackground>(videoPath, curSong->videoStartTime);
            videoBackground->selfPtr = videoBackground;
        }
        LoadCharts();
    });
    // std::thread ChartLoader(LoadCharts);
    // ChartLoader.detach();
    gameplaySet.StartLoad();
}

void ChartLoadingMenu::Draw() {
    Units u = Units::getInstance();
    Assets &assets = Assets::getInstance();

    ClearBackground(BLACK);
    GameMenu::DrawAlbumArtBackground();
    GameMenu::DrawTopOvershell(0.15f);
    Encore::Text::lDrawText(
        assets.redHatDisplayBlack,
        "chartLoading.header",
        { u.LeftSide, u.hpct(0.05f) },
        u.hinpct(0.125f),
        WHITE,
        LEFT
    );
    float AfterLoadingTextPos =
        MeasureTextEx(assets.redHatDisplayBlack, "LOADING...  ", u.hinpct(0.125f), 0).x;

    std::string LoadingPhrase = curSong->loadingPhrase.empty()
        ? LOCALIZE("chartLoading.loadingPhrase").toString()
        : curSong->loadingPhrase;

    Encore::Text::DrawText(
        assets.rubikBold,
        LoadingPhrase.c_str(),
        { u.LeftSide + AfterLoadingTextPos + u.winpct(0.02f), u.hpct(0.09f) },
        u.hinpct(0.05f),
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

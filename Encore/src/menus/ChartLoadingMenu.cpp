//
// Created by marie on 17/11/2024.
//

#include "ChartLoadingMenu.h"

#include "MenuManager.h"
#include "gameMenu.h"
#include "uiUnits.h"
#include "RhythmEngine/engines.h"
#include "gameplay/enctime.h"
#include "gameplay/gameplayRenderer.h"
#include "users/playerManager.h"

#include <thread>


bool StartLoading = true;
bool FinishedLoading = false;

void LoadCharts() {
    TheSongList.curSong->midiFile.read(TheSongList.curSong->midiPath.string());
    TheSongList.curSong->midiFile.doTimeAnalysis();
    TheSongTime.BeatmapFromMidiTrack(
        TheSongList.curSong->midiFile, TheSongList.curSong->endTick
    );
    TheSongTime.GenerateOverdriveTicks(
        TheSongList.curSong->midiFile, TheSongList.curSong->BeatTrackID
    );
    // TheSongList.curSong->getTiming(midiFile, 0, midiFile[0]);
    // TheSongList.curSong->parseBeatLines(midiFile, TheSongList.curSong->BeatTrackID);
    for (int playerNum = 0; playerNum < ThePlayerManager.PlayersActive; playerNum++) {
        Player &player = ThePlayerManager.GetActivePlayer(playerNum);
        int diff = player.Difficulty;
        int inst = player.Instrument;

        int track = TheSongList.curSong->parts[inst]->TrackInt;
        std::string trackName;

        if (TheSongList.curSong->parts[inst]->Valid) {
            trackName = songPartsList[inst];
            Encore::EncoreLog(
                LOG_DEBUG,
                TextFormat("Loading part %s, diff %01i", trackName.c_str(), diff)
            );
            // LoadingState = NOTE_PARSING;
            // if plastic
            if (inst < PitchedVocals && inst != PlasticDrums && inst > PartVocals) {
                TheSongList.curSong->midiFile[track].linkNotePairs();
                Encore::EncoreLog(
                    LOG_DEBUG,
                    TextFormat("Hopo threshold: %01i", TheSongList.curSong->hopoThreshold)
                );
                Encore::RhythmEngine::GuitarLoader chartLoader(
                    diff, TheSongList.curSong->hopoThreshold
                );
                chartLoader.LoadChart(TheSongList.curSong->midiFile[track]);

                player.engine =
                    std::make_shared<Encore::RhythmEngine::GuitarEngine>(
                    std::make_shared<Encore::RhythmEngine::BaseChart>(chartLoader.chart),
                        std::make_shared<Encore::RhythmEngine::GuitarStats>(0)
                    );
                player.engine->stats->Type = Encore::RhythmEngine::Guitar;

            } else if (inst == PlasticDrums) {
                TheSongList.curSong->midiFile[track].linkNotePairs();
                Encore::RhythmEngine::DrumsLoader chartLoader(diff);
                chartLoader.LoadChart(TheSongList.curSong->midiFile[track]);

                ThePlayerManager.GetActivePlayer(playerNum)
                    .engine = std::make_shared<Encore::RhythmEngine::DrumsEngine>(
                    std::make_shared<Encore::RhythmEngine::BaseChart>(chartLoader.chart),
                    std::make_shared<Encore::RhythmEngine::DrumsStats>(0)
                );
                ThePlayerManager.GetActivePlayer(playerNum).engine->stats->Type =
                    Encore::RhythmEngine::Drums;
            } else if (inst < PlasticDrums) {
                TheSongList.curSong->midiFile[track].linkNotePairs();
                Encore::RhythmEngine::PadLoader chartLoader(diff);
                chartLoader.LoadChart(TheSongList.curSong->midiFile[track]);

                ThePlayerManager.GetActivePlayer(playerNum).engine =
                    std::make_shared<Encore::RhythmEngine::PadEngine>(
                        std::make_shared<Encore::RhythmEngine::BaseChart>(chartLoader.chart),
                    std::make_shared<Encore::RhythmEngine::PadStats>(0)
                );
                ThePlayerManager.GetActivePlayer(playerNum).engine->stats->Type =
                    Encore::RhythmEngine::Pad;
                // todo: make pad engine shit how did u forget
            }
            for (int i = 0; i
                 < ThePlayerManager.GetActivePlayer(playerNum).engine->chart->Lanes.size(
                 );
                 i++) {
                ThePlayerManager.GetActivePlayer(playerNum)
                    .engine->chart->Lanes.at(i)
                    .shrink_to_fit();
                ThePlayerManager.GetActivePlayer(playerNum)
                    .engine->chart->CurrentNoteIterators.at(i) =
                    ThePlayerManager.GetActivePlayer(playerNum)
                        .engine->chart->Lanes.at(i)
                        .begin();
            }

            // if (!chart.plastic) {
            //     LoadingState = EXTRA_PROCESSING;
            //     int noteIdx = 0;
            //     for (Note &note : chart.notes) {
            //         chart.notes_perlane[note.lane].push_back(noteIdx);
            //         noteIdx++;
            //     }
            // }
            // chart.Loaded = true;
        }
        //}
        //}
        //}
    }

    TheSongList.curSong->getCodas(TheSongList.curSong->midiFile);

    // LoadingState = READY;

    // std::this_thread::sleep_for(std::chrono::seconds(1));
    FinishedLoading = true;
}
/**
 * @brief Load chart, create new player
 */
void ChartLoadingMenu::Load() {
    // for (int playerNum = 0; playerNum < ThePlayerManager.PlayersActive; playerNum++) {
    //    ThePlayerManager.GetActivePlayer(playerNum).stats = new PlayerGameplayStats(
    //        ThePlayerManager.GetActivePlayer(playerNum).Difficulty,
    //        ThePlayerManager.GetActivePlayer(playerNum).Instrument
    //    );
    // }
    // ThePlayerManager.BandStats = new BandGameplayStats;
    TheSongList.curSong->LoadAlbumArt();
    LoadCharts();
    // std::thread ChartLoader(LoadCharts);
    // ChartLoader.detach();
}

void ChartLoadingMenu::Draw() {
    Units u = Units::getInstance();
    Assets &assets = Assets::getInstance();

    ClearBackground(BLACK);
    GameMenu::DrawAlbumArtBackground(TheSongList.curSong->albumArtBlur);
    encOS::DrawTopOvershell(0.15f);
    DrawTextEx(
        assets.redHatDisplayBlack,
        "LOADING...  ",
        { u.LeftSide, u.hpct(0.05f) },
        u.hinpct(0.125f),
        0,
        WHITE
    );
    float AfterLoadingTextPos =
        MeasureTextEx(assets.redHatDisplayBlack, "LOADING...  ", u.hinpct(0.125f), 0).x;

    std::string LoadingPhrase = TheSongList.curSong->loadingPhrase.empty()
        ? "Loading Song..."
        : TheSongList.curSong->loadingPhrase;

    DrawTextEx(
        assets.rubikBold,
        LoadingPhrase.c_str(),
        { u.LeftSide + AfterLoadingTextPos + u.winpct(0.02f), u.hpct(0.09f) },
        u.hinpct(0.05f),
        0,
        LIGHTGRAY
    );

    GameMenu::DrawBottomOvershell();
    DrawOvershell();

    if (FinishedLoading) {
        // TheGameRenderer.LoadGameplayAssets();
        FinishedLoading = false;
        StartLoading = true;
        TheMenuManager.SwitchScreen(GAMEPLAY);
    }
}

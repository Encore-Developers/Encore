//
// Created by marie on 17/11/2024.
//

#include "ChartLoadingMenu.h"

#include "GameplayMenu.h"
#include "../MenuManager.h"
#include "../main/MainMenu.h"
#include "../uiUnits.h"
#include "RhythmEngine/engines.h"
#include "gameplay/enctime.h"
#include "tracy/Tracy.hpp"
#include "users/playerManager.h"
#include "../overshell/OvershellHelper.h"

#include <thread>

#include "menus/locale/Locale.h"
#include "RhythmEngine/ChartLoaders/LyricLoader.h"
#include "RhythmEngine/ChartLoaders/PadConverters/PadConverters.h"

bool StartLoading = true;
bool FinishedLoading = false;

void ChartLoadingMenu::LoadCharts() {
    ZoneScoped;
    auto &midiFile = curSong->midiFile;
    {
        ZoneScopedN("MIDI Read")
        midiFile.read(curSong->midiPath.string());
        midiFile.doTimeAnalysis();
        TheSongTime.BeatmapFromMidiTrack(
            curSong, midiFile, curSong->endTick
        );
        // TheSongTime.GenerateOverdriveTicks(
        //    midiFile, TheSongList.curSong->BeatTrackID
        //);
        TheSongTime.ParseSections(curSong, midiFile);
        for (int track = 0; track < midiFile.getTrackCount(); track++) {
            SongParts songPart = curSong->GetSongPart(midiFile[track]);
            curSong->IsPartValid(midiFile[track], songPart, track);
            if (songPart == BeatLines) {
                curSong->BeatTrackID = track;
            }
            else if (songPart == Events) {
                curSong->getStartEnd(midiFile, track, midiFile[track]);
            }
            else if (songPart == PitchedVocals) {
                Encore::RhythmEngine::LyricLoader lyricLoader(&midiFile, track);
                lyricLoader.LoadLyrics();
                TheSongTime.Lyrics = lyricLoader.lyrics;
            }
        }
    }
    // TheSongList.curSong->getTiming(midiFile, 0, midiFile[0]);
    // TheSongList.curSong->parseBeatLines(midiFile, TheSongList.curSong->BeatTrackID);
    for (int playerNum = 0; playerNum < MAX_PLAYERS; playerNum++) {
        if (ThePlayerManager.ActivePlayers[playerNum] == -1) continue;
        ZoneScopedN("RhythmEngine ctors")
        Player &player = ThePlayerManager.GetActivePlayer(playerNum);
        int diff = player.Difficulty;
        int inst = player.Instrument;

        int track = curSong->parts[inst].TrackInt;
        std::string trackName;

        if (curSong->parts[inst].Valid) {
            trackName = songPartsList[inst];
            Encore::EncoreLog(
                LOG_DEBUG,
                TextFormat("Loading part %s, diff %01i", trackName.c_str(), diff)
            );
            // LoadingState = NOTE_PARSING;
            // if plastic
            if (inst < PitchedVocals && inst != PlasticDrums && inst > PartVocals) {
                midiFile[track].linkNotePairs();
                if (curSong->hopoThreshold == -1) {
                    curSong->hopoThreshold = (midiFile.getTicksPerQuarterNote() / 3) + 1;
                }
                Encore::EncoreLog(
                    LOG_DEBUG,
                    TextFormat("Hopo threshold: %01i", curSong->hopoThreshold)
                );
                Encore::RhythmEngine::GuitarLoader chartLoader(
                    diff, curSong->hopoThreshold, &midiFile
                );
                chartLoader.LoadChart(midiFile[track]);

                player.engine =
                    std::make_shared<Encore::RhythmEngine::GuitarEngine>(
                    std::make_shared<Encore::RhythmEngine::BaseChart>(chartLoader.chart),
                        std::make_shared<Encore::RhythmEngine::GuitarStats>(0), &player
                    );
                player.engine->stats->Type = Encore::RhythmEngine::Guitar;

            } else if (inst == PlasticDrums) {
                midiFile[track].linkNotePairs();
                Encore::RhythmEngine::DrumsLoader chartLoader(diff, &midiFile);
                chartLoader.LoadChart(midiFile[track]);

                ThePlayerManager.GetActivePlayer(playerNum)
                    .engine = std::make_shared<Encore::RhythmEngine::DrumsEngine>(
                    std::make_shared<Encore::RhythmEngine::BaseChart>(chartLoader.chart),
                    std::make_shared<Encore::RhythmEngine::DrumsStats>(0),
                    &player
                );
                ThePlayerManager.GetActivePlayer(playerNum).engine->stats->Type =
                    Encore::RhythmEngine::Drums;
            } else if (inst < PlasticDrums) {
                midiFile[track].linkNotePairs();
                Encore::RhythmEngine::BaseChart chart;
                if (!curSong->parts[inst].AutoToPad) {
                    Encore::RhythmEngine::PadLoader chartLoader(diff, 170, &midiFile);
                    chartLoader.LoadChart(midiFile[track]);
                    chart = chartLoader.chart;
                } else {
                    if (curSong->hopoThreshold == -1) {
                        curSong->hopoThreshold = (midiFile.getTicksPerQuarterNote() / 3) + 1;
                    }
                    Encore::RhythmEngine::GuitarLoader chartLoader(
                        diff, curSong->hopoThreshold, &midiFile
                    );
                    chartLoader.LoadChart(midiFile[track]);
                    chart = Encore::RhythmEngine::PadConverters::ConvertGuitarToPad(chartLoader.chart);
                }


                ThePlayerManager.GetActivePlayer(playerNum).engine =
                    std::make_shared<Encore::RhythmEngine::PadEngine>(
                        std::make_shared<Encore::RhythmEngine::BaseChart>(chart),
                    std::make_shared<Encore::RhythmEngine::PadStats>(0),
                    &player
                );
                ThePlayerManager.GetActivePlayer(playerNum).engine->stats->Type =
                    Encore::RhythmEngine::Pad;
                if (diff < 3)
                    ThePlayerManager.GetActivePlayer(playerNum).engine->chart->Lanes.resize(4);
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
            ThePlayerManager.GetActivePlayer(playerNum).engine->stats->overdrive.ticks.GenerateOverdriveTicks(midiFile, curSong->BeatTrackID);
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

    curSong->getCodas(midiFile);

    // LoadingState = READY;

    // std::this_thread::sleep_for(std::chrono::seconds(1));
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
    curSong->LoadAlbumArt();
    TheAudioManager.unloadStreams();

    TheAudioManager.loadStreams(
    curSong->LoadAudioINI()
    );
    LoadCharts();
    // std::thread ChartLoader(LoadCharts);
    // ChartLoader.detach();
    gameplaySet.StartLoad();
}

void ChartLoadingMenu::Draw() {
    Units u = Units::getInstance();
    Assets &assets = Assets::getInstance();

    ClearBackground(BLACK);
    GameMenu::DrawAlbumArtBackground();
    encOS::DrawTopOvershell(0.15f);
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
    DrawOvershell();

    if (FinishedLoading) {
        // TheGameRenderer.LoadGameplayAssets();
        FinishedLoading = false;
        StartLoading = true;
        TheMenuManager.CreateAndSwitchMenu<GameplayMenu>(curSong);
    }
}

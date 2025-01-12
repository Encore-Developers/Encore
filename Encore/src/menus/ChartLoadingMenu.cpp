//
// Created by marie on 17/11/2024.
//

#include "ChartLoadingMenu.h"

#include "MenuManager.h"
#include "gameMenu.h"
#include "uiUnits.h"
#include "gameplay/gameplayRenderer.h"
#include "users/playerManager.h"

#include <thread>

bool StartLoading = true;
bool FinishedLoading = false;

void LoadCharts() {
    smf::MidiFile midiFile;
    midiFile.read(TheSongList.curSong->midiPath.string());
    TheSongList.curSong->getTiming(midiFile, 0, midiFile[0]);
    TheSongList.curSong->parseBeatLines(midiFile, TheSongList.curSong->BeatTrackID);
    for (int playerNum = 0; playerNum < ThePlayerManager.PlayersActive; playerNum++) {
        Player &player = ThePlayerManager.GetActivePlayer(playerNum);
        int diff = player.Difficulty;
        int inst = player.Instrument;
        int track = TheSongList.curSong->parts[inst]->charts[diff].track;
        std::string trackName;

        Chart &chart = TheSongList.curSong->parts[inst]->charts[diff];
        if (chart.valid) {
            Encore::EncoreLog(
                LOG_DEBUG,
                TextFormat("Loading part %s, diff %01i", trackName.c_str(), diff)
            );
            LoadingState = NOTE_PARSING;
            if (inst < PitchedVocals && inst != PlasticDrums && inst > PartVocals) {
                chart.plastic = true;
                chart.parsePlasticNotes(
                    midiFile, track, diff, inst, TheSongList.curSong->hopoThreshold
                );
            } else if (inst == PlasticDrums) {
                chart.plastic = true;
                chart.parsePlasticDrums(
                    midiFile, track, midiFile[track], diff, inst, player.ProDrums, true
                );
            } else {
                chart.plastic = false;
                chart.parseNotes(midiFile, track, midiFile[track], diff, inst);
            }

            if (!chart.plastic) {
                LoadingState = EXTRA_PROCESSING;
                int noteIdx = 0;
                for (Note &note : chart.notes) {
                    chart.notes_perlane[note.lane].push_back(noteIdx);
                    noteIdx++;
                }
            }
        }

        //}
        //}
        //}
    }

    TheSongList.curSong->getCodas(midiFile);
    LoadingState = READY;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    FinishedLoading = true;
}
/**
 * @brief Load chart, create new player
 */
void ChartLoadingMenu::Load() {
    for (int playerNum = 0; playerNum < ThePlayerManager.PlayersActive; playerNum++) {
        ThePlayerManager.GetActivePlayer(playerNum).stats = new PlayerGameplayStats(
            ThePlayerManager.GetActivePlayer(playerNum).Difficulty,
            ThePlayerManager.GetActivePlayer(playerNum).Instrument
        );
    }
    ThePlayerManager.BandStats = new BandGameplayStats;
    TheSongList.curSong->LoadAlbumArt();
    std::thread ChartLoader(LoadCharts);
    ChartLoader.detach();
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

    std::string LoadingPhrase;

    switch (LoadingState) {
    case BEATLINES: {
        LoadingPhrase = "Setting metronome";
        break;
    }
    case NOTE_PARSING: {
        LoadingPhrase = "Loading notes";
        break;
    }
    case NOTE_SORTING: {
        LoadingPhrase = "Cleaning up notes";
        break;
    }
    case PLASTIC_CALC: {
        LoadingPhrase = "Fixing up Classic";
        break;
    }
    case NOTE_MODIFIERS: {
        LoadingPhrase = "HOPO on, HOPO off";
        break;
    }
    case OVERDRIVE: {
        LoadingPhrase = "Gettin' your double points on";
        break;
    }
    case SOLOS: {
        LoadingPhrase = "Shuffling through solos";
        break;
    }
    case BASE_SCORE: {
        LoadingPhrase = "Calculating stars";
        break;
    }
    case EXTRA_PROCESSING: {
        LoadingPhrase = "Finishing touch-ups";
        break;
    }
    case READY: {
        LoadingPhrase = "Ready!";
        break;
    }
    default: {
        LoadingPhrase = "";
        break;
    }
    }

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
        TheGameRenderer.LoadGameplayAssets();
        FinishedLoading = false;
        StartLoading = true;
        TheMenuManager.SwitchScreen(GAMEPLAY);
    }
}
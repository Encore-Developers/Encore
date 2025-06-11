//
// Created by maria on 21/05/2025.
//

#include "RhythmEngine.h"

#include "GuitarEngine.h"
#include "GuitarStats.h"
#include "RhythmEngine/NoteVector.h"
#include "RhythmEngine/ChartLoaders/GuitarLoader.h"
#include "users/playerManager.h"

/*
void Encore::RhythmEngine::lol() {
    smf::MidiFile MidiFile;
    int track = 0; // this would be gathered from finding PART_GUITAR in .ini

    GuitarLoader loader(3, MidiFile[track]);
    loader.LoadChart(); // this would be done in a separate thread maybe?
    // maybe have the chart also be a shared ptr

    auto Chart = std::make_shared<GuitarChart>(loader.chart);
    auto Stats = std::make_shared<GuitarStats>(0);
    auto Engine = std::make_shared<GuitarEngine>();
    ThePlayerManager.GetActivePlayer(0).engine = Engine;
    ThePlayerManager.GetActivePlayer(0).engine->stats = Stats;
    ThePlayerManager.GetActivePlayer(0).engine->chart = Chart;
}
*/
//
// Created by maria on 15/06/2026.
//

#include "ChartLoader.h"

#include "DotChartLoader/DotChartHandler.h"
#include "MidiLoaders/MidiChartHandler.h"
#include "RhythmEngine/Engine/Drums/DrumsEngine.h"
#include "RhythmEngine/Engine/Guitar/GuitarEngine.h"
#include "RhythmEngine/Engine/Pad/PadEngine.h"

Encore::RhythmEngine::ChartLoader::ChartLoader(Song* song) {
    switch (song->chartFormat) {
    case ChartFormat::Midi:
        chartHandler = std::make_shared<MidiChartHandler>(song);
        break;
    case ChartFormat::DotChart:
        chartHandler = std::make_shared<DotChartHandler>(song);
        break;
    }
}

std::array<TrackInformation, PlasticVocals> & Encore::RhythmEngine::ChartLoader::
GetSongParts() const {
    return chartHandler->GetValidParts();
}

std::vector<Encore::RhythmEngine::LyricPhrase> Encore::RhythmEngine::ChartLoader::
GetLyricPhrases() const {
    return chartHandler->GetLyricPhrases();
}

Encore::RhythmEngine::OverdriveTicks Encore::RhythmEngine::ChartLoader::
GenerateOverdriveTicks() const {
    return chartHandler->GenerateOverdriveTicks();
}

std::pair<int, double> Encore::RhythmEngine::ChartLoader::GetEndEvent() const {
    return chartHandler->GetEndEvent();
}

std::vector<BPM> Encore::RhythmEngine::ChartLoader::GetBPMChanges() const {
    return chartHandler->GetBPMChanges();
}

std::vector<Section> Encore::RhythmEngine::ChartLoader::GetSections() const {
    return chartHandler->GetSections();
}

int Encore::RhythmEngine::ChartLoader::GetResolution() {
    return chartHandler->GetResolution();
}

std::vector<TimeSig> Encore::RhythmEngine::ChartLoader::GetTimeSigChanges() const {
    return chartHandler->GetTimeSigChanges();
}
void Encore::RhythmEngine::ChartLoader::PopulateSubsongs() {
    chartHandler->PopulateSubsongs();
}

Encore::RhythmEngine::BaseChart Encore::RhythmEngine::ChartLoader::GetChart(const SongPart part,
                                                                            int difficulty) const {
    return chartHandler->GetChart(part, difficulty);
}
std::shared_ptr<Encore::RhythmEngine::BaseEngine>
Encore::RhythmEngine::ChartLoader::CreateEngine(Player *player) const {
    int diff = player->Difficulty;
    int inst = player->Instrument;
    std::string trackName;
    std::shared_ptr<BaseEngine> engine;
    if (inst < PitchedVocals && inst != PlasticDrums && inst > PartVocals) {
        engine =
            std::make_shared<GuitarEngine>(
                std::make_shared<BaseChart>(
                    GetChart(SongPart(inst), diff)),
                player
            );
        engine->stats->Type = Guitar;
    } else if (inst == PlasticDrums) {
        engine = std::make_shared<DrumsEngine>(
            std::make_shared<BaseChart>(
                GetChart(SongPart(inst), diff)),
            player
        );
        engine->stats->Type = Drums;
    } else if (inst < PlasticDrums) {
        engine = std::make_shared<PadEngine>(
                std::make_shared<BaseChart>(
                    GetChart(SongPart(inst), diff)),
                player
            );
        engine->stats->Type = Pad;
        if (engine->chart->Lanes[4].empty())
            engine->chart->Lanes.
                             resize(4);
    }
    if (inst == PartVocals || inst == PartBass || inst == PlasticVocals) {
        engine->stats->ep.mult.max = 6;
    }
    for (int i = 0; i < engine->chart->Lanes.size(); i++) {

        engine->chart->at(i).shrink_to_fit();
        engine->chart->CurrentNoteIterators.at(i) =
            engine->chart->at(i).begin();
    }
    engine->stats->overdrive.ticks = GenerateOverdriveTicks();
    engine->chart->sections = GetSections();

    return engine;
}

void Encore::RhythmEngine::ChartLoader::LoadCharts() const {
    chartHandler->LoadCharts();
}
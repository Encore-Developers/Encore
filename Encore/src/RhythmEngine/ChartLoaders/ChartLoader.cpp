//
// Created by maria on 15/06/2026.
//

#include "ChartLoader.h"

#include "DotChartLoader/DotChartHandler.h"
#include "MidiLoaders/MidiChartHandler.h"
#include "RhythmEngine/Engine/Drums/DrumsEngine.h"
#include "RhythmEngine/Engine/Guitar/GuitarEngine.h"
#include "RhythmEngine/Engine/Pad/PadEngine.h"

using namespace Encore::RhythmEngine;

std::shared_ptr<BaseChart> ChartHandler::GetChart(SongPart *part, int diff) {
    for (auto& info : Parts) {
        if (info.Part == part) {
            if (info.ValidDiffs.at(diff)) {
                return info.Charts.at(diff);
            }
        }
    }
    return nullptr;
}
ChartLoader::ChartLoader(Song* song) {
    switch (song->chartFormat) {
    case ChartFormat::Midi:
        chartHandler = std::make_shared<MidiChartHandler>(song);
        break;
    case ChartFormat::DotChart:
        chartHandler = std::make_shared<DotChartHandler>(song);
        break;
    }
}

std::vector<TrackInformation>& ChartLoader::
GetSongParts() const {
    return chartHandler->GetValidParts();
}

std::vector<LyricPhrase> ChartLoader::
GetLyricPhrases() const {
    return chartHandler->GetLyricPhrases();
}

std::vector<TimePoint> ChartLoader::
GenerateOverdriveTicks() const {
    return chartHandler->GenerateOverdriveTicks();
}

std::pair<int, double> ChartLoader::GetEndEvent() const {
    return chartHandler->GetEndEvent();
}

std::vector<BPM> ChartLoader::GetBPMChanges() const {
    return chartHandler->GetBPMChanges();
}

std::vector<Section> ChartLoader::GetSections() const {
    return chartHandler->GetSections();
}
std::shared_ptr<TempoMap>
ChartLoader::GetTempoMap() {
    if (auto cached = tempoMap.lock()) {
        return cached;
    } else {
        auto newMap = std::make_shared<TempoMap>();
        newMap->BPMChanges = GetBPMChanges();
        newMap->TimeSigChanges = GetTimeSigChanges();
        newMap->ppqn = GetResolution();
        newMap->OverdriveTicks = GenerateOverdriveTicks();
        newMap->GenerateBeatmap(GetEndEvent().first);
        tempoMap = newMap;
        return newMap;
    }
}

int ChartLoader::GetResolution() const {
    return chartHandler->GetResolution();
}

std::vector<TimeSig> ChartLoader::GetTimeSigChanges() const {
    return chartHandler->GetTimeSigChanges();
}
void ChartLoader::PopulateSubsongs() {
    chartHandler->PopulateSubsongs();
}

std::shared_ptr<BaseChart> ChartLoader::GetChart(SongPart* part, int difficulty) const {
    return chartHandler->GetChart(part, difficulty);
}
std::shared_ptr<BaseEngine>
ChartLoader::CreateEngine(Player *player) {
    int diff = player->Difficulty;
    auto part = player->Part;
    std::string trackName;
    std::shared_ptr<BaseEngine> engine;
    auto chart = std::make_shared<BaseChart>(*GetChart(part, diff));
    assert(chart);
    switch (part->archetype) {
    case ArchetypePad: {
        engine = std::make_shared<PadEngine>(chart, player);
        if (engine->chart->Lanes[4].empty())
            engine->chart->Lanes.resize(4);
        break;
    }
    case ArchetypeDrums: {
        engine = std::make_shared<DrumsEngine>(chart, player);
        break;
    }
    case ArchetypeGuitar: {
        engine = std::make_shared<GuitarEngine>(chart, player);
        break;
    }
    }
    engine->stats->ep.mult.max = part->maxMultiplier;
    for (int i = 0; i < engine->chart->Lanes.size(); i++) {

        engine->chart->at(i).shrink_to_fit();
        engine->chart->CurrentNoteIterators.at(i) =
            engine->chart->at(i).begin();
    }
    engine->tempoMap = GetTempoMap();
    engine->chart->sections = GetSections();
    engine->CalculateWeightedScore();
    engine->part = part;
    return engine;
}

void ChartLoader::LoadCharts() const {
    chartHandler->LoadCharts();
}
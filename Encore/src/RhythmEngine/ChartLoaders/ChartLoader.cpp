//
// Created by maria on 15/06/2026.
//

#include "ChartLoader.h"

#include "DotChartLoader/DotChartHandler.h"
#include "MidiLoaders/MidiChartHandler.h"

Encore::RhythmEngine::ChartLoader::ChartLoader(const std::filesystem::path &chartPath) {
    if (chartPath.filename() == "notes.mid") {
        chartHandler = new MidiChartHandler(chartPath);
    } else if (chartPath.filename() == "notes.chart") {
        chartHandler = new DotChartHandler(chartPath);
    }
}

std::array<TrackInformation, PlasticVocals> & Encore::RhythmEngine::ChartLoader::
GetSongParts() const {
    return chartHandler->GetValidParts();
}

std::vector<Encore::RhythmEngine::EncLyricPhrase> Encore::RhythmEngine::ChartLoader::
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

Encore::RhythmEngine::BaseChart Encore::RhythmEngine::ChartLoader::GetChart(const SongPart part,
                                                                            int difficulty) const {
    return chartHandler->GetChart(part, difficulty);
}

void Encore::RhythmEngine::ChartLoader::LoadCharts() const {
    chartHandler->LoadCharts();
}
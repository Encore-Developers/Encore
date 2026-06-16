//
// Created by maria on 15/06/2026.
//

#include "ChartLoader.h"
#include "MidiLoaders/MidiChartHandler.h"

Encore::RhythmEngine::ChartLoader::ChartLoader(const std::filesystem::path &chartPath) {
    if (chartPath.filename() == "notes.mid") {
        chartHandler = new MidiChartHandler(chartPath);
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

Encore::RhythmEngine::BaseChart Encore::RhythmEngine::ChartLoader::GetChart(const SongPart part,
                                                                            int difficulty) const {
    return chartHandler->GetChart(part, difficulty);
}

void Encore::RhythmEngine::ChartLoader::LoadCharts() const {
    chartHandler->LoadCharts();
}
//
// Created by maria on 15/06/2026.
//

#include "ChartLoader.h"

#include "DotChartLoader/DotChartHandler.h"
#include "MidiLoaders/MidiChartHandler.h"

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

void Encore::RhythmEngine::ChartLoader::LoadCharts() const {
    chartHandler->LoadCharts();
}
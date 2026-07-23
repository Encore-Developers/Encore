#pragma once
#include "RhythmEngine/ChartLoaders/ChartLoader.h"
//
// Created by maria on 18/06/2026.
//


namespace Encore::RhythmEngine {
    class DotChartHandler final : public ChartHandler
    {
        std::ifstream chart;
        int EventsTrack = -1;
        int tpq = 192;
        bool processed = false;
        bool FinishedLoading = false;
        std::vector<TimeSig> TS;
        std::vector<BPM> bpms;
        std::pair<int, double> EndEvent;
    public:
        explicit DotChartHandler(std::filesystem::path fileName);;
        ~DotChartHandler() override;
        int GetResolution() override { return tpq; };
        std::vector<BPM> GetBPMChanges() override;
        std::vector<TimeSig> GetTimeSigChanges() override;
        std::array<TrackInformation, PlasticVocals>& GetValidParts() override;
        OverdriveTicks GenerateOverdriveTicks() override;
        std::vector<LyricPhrase> GetLyricPhrases() override;
        void LoadCharts() override;
        std::pair<int, double> GetEndEvent() override;
        std::vector<Section> GetSections() override;
        BaseChart GetChart(int part, int diff) override;
        bool IsLoaded() override;
    };

}
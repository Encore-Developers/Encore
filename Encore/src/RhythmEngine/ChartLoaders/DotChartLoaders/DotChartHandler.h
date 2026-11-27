#pragma once
#include "RhythmEngine/ChartLoaders/ChartLoader.h"
//
// Created by maria on 18/06/2026.
//


namespace Encore::RhythmEngine {
    class DotChartHandler final : public ChartHandler
    {
        int EventsTrack = -1;
        int tpq = 192;
        bool processed = false;
        bool FinishedLoading = false;
        std::vector<TimeSig> TS;
        std::vector<BPM> bpms;

        std::unordered_map<std::string, std::vector<std::string>> sections;

        struct DotChartEvent {
            std::string key;
            std::vector<std::string> values;
        };

        DotChartEvent ParseEvent(const std::string& line);

        /// Make sure GetBPMChanges() has been run before calling
        TimeSeconds TickToSeconds(TimeTicks tick);

    public:
        explicit DotChartHandler(Song* song);
        int GetResolution() override { return tpq; };
        std::vector<BPM> GetBPMChanges() override;
        std::vector<TimeSig> GetTimeSigChanges() override;
        std::vector<TrackInformation>& GetValidParts() override;
        std::vector<TimePoint> GenerateOverdriveTicks() override;
        std::vector<LyricPhrase> GetLyricPhrases() override;
        void PreloadCharts() override;
        std::pair<TimeTicks, TimeSeconds> GetEndEvent() override;
        std::vector<Section> GetSections() override;
        std::shared_ptr<BaseChart> GetChart(ChartLoadContext ctx) override;
        bool IsLoaded() override;
        void PopulateSubsongs() override;
    };

}
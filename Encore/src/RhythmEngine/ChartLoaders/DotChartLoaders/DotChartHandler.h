#pragma once
#include "RhythmEngine/ChartLoaders/ChartLoader.h"
//
// Created by maria on 18/06/2026.
//


namespace Encore::RhythmEngine {
    struct DotChartEvent {
        std::string key;
        std::vector<std::string> values;
    };

    class DotChartHandler final : public ChartHandler
    {
        bool processed = false;
        bool FinishedLoading = false;
        std::vector<TimeSig> TS;
        std::vector<BPM> bpms;

        std::unordered_map<std::string, std::vector<std::string>> sections;



        DotChartEvent ParseEvent(const std::string& line);

        /// Make sure GetBPMChanges() has been run before calling
        TimeSeconds TickToSeconds(TimeTicks tick);

        friend class DotChartBaseLoader;

    public:

        int tpq = 192;
        static std::array<std::pair<std::string, int>, 4> diffs;
        static const std::string& GetDiffName(int diff);

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
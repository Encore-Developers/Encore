#pragma once

#include "RhythmEngine/ChartLoaders/ChartLoader.h"
#include "RhythmEngine/Overdrive/OverdriveTicks.h"
//
// Created by maria on 15/06/2026.
//

namespace Encore::RhythmEngine {
    class MidiChartHandler final : public ChartHandler
    {
        smf::MidiFile midifile;
        static SongPart GetSongPart(smf::MidiEventList& track);
        int EventsTrack = -1;
        int tpq = 480;
        bool processed = false;
        bool FinishedLoading = false;
    public:
        explicit MidiChartHandler(std::filesystem::path fileName);
        ~MidiChartHandler() = default;
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
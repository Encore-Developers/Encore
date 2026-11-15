#pragma once

#include "RhythmEngine/ChartLoaders/ChartLoader.h"
//
// Created by maria on 15/06/2026.
//

namespace Encore::RhythmEngine {
    class MidiChartHandler final : public ChartHandler
    {
        smf::MidiFile midifile;
        static std::string GetMidiTrackName(smf::MidiEventList& track);
        int EventsTrack = -1;
        int VoxTrack = -1;
        int tpq = 480;
        bool processed = false;
        bool FinishedLoading = false;
    public:
        explicit MidiChartHandler(Song* song);
        ~MidiChartHandler() = default;
        int GetResolution() override { return tpq; };
        std::vector<BPM> GetBPMChanges() override;
        std::vector<TimeSig> GetTimeSigChanges() override;
        std::vector<TrackInformation>& GetValidParts() override;
        std::vector<TimePoint> GenerateOverdriveTicks() override;
        std::vector<LyricPhrase> GetLyricPhrases() override;
        void LoadCharts() override;
        std::pair<int, double> GetEndEvent() override;
        std::vector<Section> GetSections() override;
        bool IsLoaded() override;
        void PopulateSubsongs() override;
    };

}
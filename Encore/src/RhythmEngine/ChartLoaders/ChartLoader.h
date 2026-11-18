#pragma once
#include "RhythmEngine/SongPart.h"
#include "RhythmEngine/Chart/NoteVector.h"
#include "RhythmEngine/Engine/BaseEngine.h"
#include "gameplay/enctime.h"

struct TrackInformation {
    int TrackIndex = -1;
    Encore::RhythmEngine::SongPart* Part;
    bool Converted = false;
    std::array<bool, 4> ValidDiffs{ false, false, false, false };
};

namespace Encore::RhythmEngine {
    struct ChartLoadContext {
        SongPart* part;
        int diff;
        bool leftyFlip;
        bool cymbals[3];

        // chart-altering modifiers would also go here

        ChartLoadContext() = default;
        ChartLoadContext(Player* player);
    };


    class ChartHandler {
    protected:
        Song* song;
        std::vector<TrackInformation> Parts;
        int threshold = -1;
        std::vector<LyricPhrase> Lyrics;
        std::vector<std::vector<odPhrase>> overdrive;
    public:
        explicit ChartHandler(Song* song)
            : song(song), threshold(song->hopoThreshold) {
        };

        void SetThreshold(int thresh) {
            threshold = thresh;
        };
        virtual ~ChartHandler() = default;
        virtual std::vector<TrackInformation> &GetValidParts() = 0;
        virtual void PreloadCharts() = 0;
        virtual std::vector<LyricPhrase> GetLyricPhrases() = 0;
        virtual std::pair<int, double> GetEndEvent() = 0;
        virtual std::vector<Section> GetSections() = 0;
        virtual std::vector<TimePoint> GenerateOverdriveTicks() = 0;
        virtual int GetResolution() = 0;
        virtual std::vector<BPM> GetBPMChanges() = 0;
        virtual std::vector<TimeSig> GetTimeSigChanges() = 0;
        virtual std::shared_ptr<BaseChart> GetChart(ChartLoadContext ctx) = 0;
        virtual bool IsLoaded() = 0;
        virtual void PopulateSubsongs() = 0;
        std::vector<std::vector<odPhrase>>& GetOverdrive();
    };

    class ChartLoader {
        std::shared_ptr<ChartHandler> chartHandler;
        std::weak_ptr<TempoMap> tempoMap;

    public:
        ChartLoader(Song* song);

        std::vector<TrackInformation> &GetSongParts() const;
        std::vector<LyricPhrase> GetLyricPhrases() const;
        std::vector<TimePoint> GenerateOverdriveTicks() const;
        std::pair<int, double> GetEndEvent() const;
        std::vector<BPM> GetBPMChanges() const;
        std::vector<Section> GetSections() const;
        std::shared_ptr<TempoMap> GetTempoMap();
        int GetResolution() const;
        std::vector<TimeSig> GetTimeSigChanges() const;
        void PopulateSubsongs();

        std::shared_ptr<BaseChart> GetChart(ChartLoadContext ctx) const;
        std::shared_ptr<BaseEngine> CreateEngine(Player* player);
        bool IsLoaded() {return chartHandler->IsLoaded();};
        void PreloadCharts() const;
    };
}

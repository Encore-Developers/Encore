#pragma once
#include "RhythmEngine/Chart/NoteVector.h"
#include "RhythmEngine/Overdrive/OverdriveTicks.h"

struct TrackInformation {
    int TrackInt = -1;
    bool Valid = false;
    std::array<bool, 4> ValidDiffs{ false, false, false, false };
    bool AutoToPad = false;
};

namespace Encore::RhythmEngine {
    class ChartHandler {
    protected:
        std::filesystem::path file;
        std::array<std::pair<std::array<BaseChart, 4>, bool>, PitchedVocals> Charts;
        std::array<TrackInformation, PlasticVocals> Parts;
        int threshold = -1;
        std::vector<EncLyricPhrase> Lyrics;
    public:
        explicit ChartHandler(std::filesystem::path fileName)
            : file(std::move(fileName)) {
        };

        void SetThreshold(int thresh) {
            threshold = thresh;
        };
        virtual ~ChartHandler() = default;
        virtual std::array<TrackInformation, PlasticVocals> &GetValidParts() = 0;
        virtual void LoadCharts() = 0;
        virtual std::vector<EncLyricPhrase> GetLyricPhrases() = 0;
        virtual std::pair<int, double> GetEndEvent() = 0;
        virtual void GetSections() = 0;
        virtual OverdriveTicks GenerateOverdriveTicks() = 0;

        virtual BaseChart GetChart(int part, int diff) = 0;
        virtual bool IsLoaded() = 0;
    };

    class ChartLoader {
        ChartHandler *chartHandler;

    public:
        explicit ChartLoader(const std::filesystem::path &chartPath);;

        std::array<TrackInformation, PlasticVocals> &GetSongParts() const;
        std::vector<EncLyricPhrase> GetLyricPhrases() const;
        OverdriveTicks GenerateOverdriveTicks() const;
        std::pair<int, double> GetEndEvent() const;

        BaseChart GetChart(SongPart part, int difficulty) const;
        bool IsLoaded() {return chartHandler->IsLoaded();};
        void LoadCharts() const;
    };
}

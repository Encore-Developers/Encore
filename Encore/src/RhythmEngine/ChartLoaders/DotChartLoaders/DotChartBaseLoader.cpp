#include "DotChartBaseLoader.h"

using namespace Encore::RhythmEngine;

TimeSeconds DotChartBaseLoader::TickToSeconds(TimeTicks tick) {
    return parent->TickToSeconds(tick);
}
std::shared_ptr<BaseChart> DotChartBaseLoader::GetChart() {
    if (!loaded) {
        auto& lines = GetChartSection();
        for (auto& line : lines) {
            auto ev = parent->ParseEvent(line);
            ReceiveEvent(ev);
        }

        EventsDone();
        loaded = true;
    }
    return chart;
}
void DotChartBaseLoader::ReceiveEvent(const DotChartEvent &event) {}
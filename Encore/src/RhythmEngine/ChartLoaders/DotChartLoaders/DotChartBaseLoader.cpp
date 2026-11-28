#include "DotChartBaseLoader.h"

using namespace Encore::RhythmEngine;

TimeSeconds DotChartBaseLoader::TickToSeconds(TimeTicks tick) {
    return parent->TickToSeconds(tick);
}
std::shared_ptr<BaseChart> DotChartBaseLoader::GetChart() {
    if (!loaded) {
        auto& lines = GetChartSection();
        for (auto& ev : lines) {
            ReceiveEvent(ev);
        }

        EventsDone();
        chart->RecountPhrases();
        loaded = true;
    }
    return chart;
}
void DotChartBaseLoader::ReceiveEvent(const DotChartEvent &event) {
    if (event.values.at(0) == "S" && event.values.at(1) == "2") {
        auto tick = std::stoi(event.key);
        auto length = std::stoi(event.values.at(2));
        odPhrase newPhrase = {TickToSeconds(tick), tick, TickToSeconds(tick+length), tick+length};
        chart->overdrive.push_back(newPhrase);
    }
    if (event.values.at(0) == "E") {
        auto tick = std::stoi(event.key);
        if (event.values.at(1) == "solo") {
            curSolo = tick;
        }
        if (event.values.at(1) == "soloend") {
            chart->solos.emplace_back(TickToSeconds(curSolo), curSolo, TickToSeconds(tick), tick);
        }
    }
}
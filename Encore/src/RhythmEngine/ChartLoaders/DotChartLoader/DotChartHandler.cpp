//
// Created by maria on 18/06/2026.
//

#include "DotChartHandler.h"
using namespace Encore::RhythmEngine;

#include <regex>
// what is wrong with this language
std::array<std::pair<std::string, int>, 4> diffs
{
        {
            {"Expert", 3},
            {"Hard", 2},
            {"Medium", 1},
            {"Easy", 0}
        }
};

double GetTimeFromTick(double bpm, int resolution, int tickDelta, const double lastTime) {
    // using https://thenathannator.github.io/GuitarGame_ChartFormats/Implementation-Info/Time-Conversions/ as a reference
    double secondsPerBeat = 60.0 / bpm;
    double beatDelta = tickDelta / resolution;
    double secondDelta = secondsPerBeat * beatDelta;
    return secondDelta + lastTime;
};
DotChartHandler::DotChartHandler(Song* song)
    : ChartHandler(song) {
    static const std::regex event("  ([0-9]+) = ([A-Z]+) (.+) ?(\\d?)");
    auto chart = song->GetChartData();
    for (std::string line; std::getline(*chart, line);) {
        if (line.empty()) continue;
        if (line.starts_with('[') && line.ends_with(']')) {
            line.pop_back();
            line.erase(0, 1);
            if (line == "SyncTrack") {
                double LastTime = 0;
                int LastTick = 0;
                double CurBPM = 0;
                for (std::string sectLine; std::getline(*chart, sectLine);) {
                    if (sectLine.starts_with('{')) continue;
                    std::smatch syncMatch;
                    std::regex_match(sectLine, syncMatch, event);
                    if (syncMatch[2] == "B") {
                        CurBPM = double(std::stoi(syncMatch[3]))/1000.0;
                        double tick = std::stoi(syncMatch[1]);
                        double time = GetTimeFromTick(CurBPM, tpq, tick-LastTick, LastTime);
                        LastTime = time;
                        LastTick = tick;
                        bpms.emplace_back(time, CurBPM, tick);
                    }
                    if (syncMatch[2] == "TS") {
                        int numer = std::stoi(syncMatch[3]);
                        int tick = std::stoi(syncMatch[1]);
                        double time = 0;
                        if (CurBPM > 0)
                            time = GetTimeFromTick(CurBPM, tpq, tick-LastTick, LastTime);
                        int powr = 2;
                        if (syncMatch[4] != "")
                            powr = std::stoi(syncMatch[4]);
                        int denom = pow(2, powr);
                        TS.emplace_back(time, numer, denom, powr, tick);
                    }
                    if (sectLine.starts_with('}')) break;
                }
                if (bpms.empty()) {
                    bpms.emplace_back(0, 120, 0);
                }
                if (TS.empty()) {
                    TS.emplace_back(0, 4, 4, 2, 0);
                }
            }
            if (line == "Events") {
                for (std::string sectLine; std::getline(*chart, sectLine);) {
                    if (sectLine.starts_with('{')) continue;
                    std::smatch syncMatch;
                    std::regex_match(sectLine, syncMatch, event);
                    if (syncMatch[2] == "E" && syncMatch[3] == "end") {
                        int tick = std::stoi(syncMatch[1]);
                        double time = 0;
                        EndEvent = {tick, time};
                    }
                }
            }
        }
    }
    // todo: time analysis after parsing beatlines

}

std::vector<BPM> DotChartHandler::GetBPMChanges() {
    return bpms;
}

std::vector<TimeSig> DotChartHandler::GetTimeSigChanges() {
    // ts means this->🥀
    return TS;
}


std::vector<TrackInformation> & DotChartHandler::
GetValidParts() {
    if (processed) {
        return Parts;
    }
    // auto chart = song->GetChartData();
    // for (std::string line; std::getline(*chart, line);) {
    //     if (line.empty()) continue;
    //     if (line.starts_with('[') && line.ends_with(']')) {
    //         line.pop_back();
    //         line.erase(0, 1);
    //         for (auto &inst : insts) {
    //             if (line.ends_with(inst.first)) {
    //                 Parts[inst.second].Valid = true;
    //                 for (auto &diff : diffs) {
    //                     if (line.starts_with(diff.first)) {
    //                         Parts[inst.second].ValidDiffs[diff.second] = true;
    //                     }
    //                 }
    //             }
    //         }
    //     }
    // }
    processed = true;
    return Parts;
}

std::vector<TimePoint> DotChartHandler::
GenerateOverdriveTicks() {
    return {};
}

std::vector<LyricPhrase> DotChartHandler::
GetLyricPhrases() {
    return std::vector<LyricPhrase>{};
}

void DotChartHandler::PreloadCharts() {
}

std::pair<int, double> DotChartHandler::GetEndEvent() {
    if (EndEvent.first) return {10000000000000000, 100000000000000000};
    return EndEvent;
}

std::vector<Section> DotChartHandler::GetSections() {
    return {};
}
std::shared_ptr<BaseChart> DotChartHandler::GetChart(ChartLoadContext ctx) {
    return nullptr;
}

bool DotChartHandler::IsLoaded() {
    return true;
}
void DotChartHandler::PopulateSubsongs() {}
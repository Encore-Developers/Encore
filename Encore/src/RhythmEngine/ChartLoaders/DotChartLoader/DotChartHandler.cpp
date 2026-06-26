//
// Created by maria on 18/06/2026.
//

#include "DotChartHandler.h"

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

std::array<std::pair<std::string, int>, PitchedVocals> insts
{
        {
            {"Single", PlasticGuitar},
            {"DoubleBass", PlasticBass},
            {"Keyboard", PlasticKeys},
            {"Drums", PlasticDrums},
            {"PadLead", PartGuitar},
            {"PadBass", PartBass},
            {"PadDrums", PartDrums},
            {"PadVocals", PartVocals},
            {"PadKeys", PartKeys}
        }
};

double GetTimeFromTick(double bpm, int resolution, int tickDelta, const double lastTime) {
    // using https://thenathannator.github.io/GuitarGame_ChartFormats/Implementation-Info/Time-Conversions/ as a reference
    double secondsPerBeat = 60.0 / bpm;
    double beatDelta = tickDelta / resolution;
    double secondDelta = secondsPerBeat * beatDelta;
    return secondDelta + lastTime;
};
Encore::RhythmEngine::DotChartHandler::DotChartHandler(std::filesystem::path fileName)
    : ChartHandler(std::move(fileName)) {
    static const std::regex event("  ([0-9]+) = ([A-Z]+) (.+) ?(\\d?)");
    chart.open(file);
    for (std::string line; std::getline(chart, line);) {
        if (line.empty()) continue;
        if (line.starts_with('[') && line.ends_with(']')) {
            line.pop_back();
            line.erase(0, 1);
            if (line == "SyncTrack") {
                double LastTime = 0;
                int LastTick = 0;
                double CurBPM = 0;
                for (std::string sectLine; std::getline(chart, sectLine);) {
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
                for (std::string sectLine; std::getline(chart, sectLine);) {
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
    chart.close();
}

Encore::RhythmEngine::DotChartHandler::~DotChartHandler() {
    chart.close();
}


std::vector<BPM> Encore::RhythmEngine::DotChartHandler::GetBPMChanges() {
    return bpms;

}

std::vector<TimeSig> Encore::RhythmEngine::DotChartHandler::GetTimeSigChanges() {
    // ts means this->🥀
    return TS;
}


std::array<TrackInformation, PlasticVocals> & Encore::RhythmEngine::DotChartHandler::
GetValidParts() {
    if (processed) {
        return Parts;
    }
    chart.open(file);
    for (std::string line; std::getline(chart, line);) {
        if (line.empty()) continue;
        if (line.starts_with('[') && line.ends_with(']')) {
            line.pop_back();
            line.erase(0, 1);
            for (auto &inst : insts) {
                if (line.ends_with(inst.first)) {
                    Parts[inst.second].Valid = true;
                    for (auto &diff : diffs) {
                        if (line.starts_with(diff.first)) {
                            Parts[inst.second].ValidDiffs[diff.second] = true;
                        }
                    }
                }
            }
        }
    }
    processed = true;
    chart.close();
    return Parts;
}

Encore::RhythmEngine::OverdriveTicks Encore::RhythmEngine::DotChartHandler::
GenerateOverdriveTicks() {
    return OverdriveTicks{};
}

std::vector<Encore::RhythmEngine::EncLyricPhrase> Encore::RhythmEngine::DotChartHandler::
GetLyricPhrases() {
    return std::vector<EncLyricPhrase>{};
}

void Encore::RhythmEngine::DotChartHandler::LoadCharts() {
}

std::pair<int, double> Encore::RhythmEngine::DotChartHandler::GetEndEvent() {
    if (EndEvent.first) return {10000000000000000, 100000000000000000};
    return EndEvent;
}

void Encore::RhythmEngine::DotChartHandler::GetSections() {
}

Encore::RhythmEngine::BaseChart Encore::RhythmEngine::DotChartHandler::GetChart(int part,
    int diff) {
    return BaseChart{};
}

bool Encore::RhythmEngine::DotChartHandler::IsLoaded() {
    return true;
}
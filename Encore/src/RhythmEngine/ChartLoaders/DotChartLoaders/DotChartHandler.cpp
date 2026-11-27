#include "Guitar/DotChartGuitarLoader.h"
#include "RhythmEngine/ChartLoaders/PadConverters/PadConverters.h"
int latestTick = 0;//
// Created by maria on 18/06/2026.
//

#include "DotChartHandler.h"
using namespace Encore::RhythmEngine;

#include <regex>
// what is wrong with this language
std::array<std::pair<std::string, int>, 4> DotChartHandler::diffs {
    { { "Expert", 3 }, { "Hard", 2 }, { "Medium", 1 }, { "Easy", 0 } }
};
const std::string &DotChartHandler::GetDiffName(int diff) {
    for (auto& diffPair : diffs) {
        if (diffPair.second == diff) {
            return diffPair.first;
        }
    }
    return "Expert";
}

double GetTimeFromTick(double bpm, int resolution, int tickDelta, const double lastTime) {
    // using
    // https://thenathannator.github.io/GuitarGame_ChartFormats/Implementation-Info/Time-Conversions/
    // as a reference
    double secondsPerBeat = 60.0 / bpm;
    double beatDelta = tickDelta / resolution;
    double secondDelta = secondsPerBeat * beatDelta;
    return secondDelta + lastTime;
};
DotChartEvent DotChartHandler::ParseEvent(const std::string &line) {
    static const std::regex matcher(R"((\w+)\s*=\s*(.+))");
    std::smatch match;
    std::regex_match(line, match, matcher);
    std::string key = match[1];
    std::string valuelist = match[2];
    std::string currentValue;
    std::vector<std::string> values;
    bool quote = false;
    for (size_t i = 0; i < valuelist.size(); i++) {
        if (valuelist[i] == '\\') {
            if (i < valuelist.size() - 1) {
                currentValue.push_back(valuelist[i + 1]);
                i++;
                continue;
            }
        }
        if (valuelist[i] == '"') {
            quote = !quote;
            continue;
        }
        if (!quote && valuelist[i] == ' ') {
            values.push_back(currentValue);
            currentValue.clear();
            continue;
        }
        currentValue.push_back(valuelist[i]);
    }
    if (!currentValue.empty()) {
        values.push_back(currentValue);
    }
    return { key, values };
}
TimeSeconds DotChartHandler::TickToSeconds(TimeTicks tick) {
    return TempoMap::TickToSeconds(bpms, tpq, tick);
}
DotChartHandler::DotChartHandler(Song *song) : ChartHandler(song) {
    PreloadCharts();
}

std::vector<BPM> DotChartHandler::GetBPMChanges() {
    if (bpms.empty()) {
        // 120bpm can be assumed if there isn't one at tick 0
        BPM curBPM = { 0, 120, 0 };
        for (auto &line : sections["SyncTrack"]) {
            auto ev = ParseEvent(line);
            if (ev.values.at(0) == "B") {
                double bpm = std::stoi(ev.values.at(1)) / 1000.0;
                int tick = std::stoi(ev.key);
                TimeSeconds time = curBPM.time
                    + (((tick - curBPM.tick) / (double)tpq) / (curBPM.bpm / 60.0));
                BPM newBPM = { time, bpm, tick };
                bpms.push_back(newBPM);
                curBPM = newBPM;
            }
        }
        if (bpms.empty()) {
            bpms.push_back(curBPM);
        }
    }
    return bpms;
}

std::vector<TimeSig> DotChartHandler::GetTimeSigChanges() {
    if (TS.empty()) {
        // Ensure the bpm changes are loaded
        GetBPMChanges();
        for (auto &line : sections["SyncTrack"]) {
            auto ev = ParseEvent(line);
            if (ev.values.at(0) == "TS") {
                int tick = std::stoi(ev.key);
                int denomExp = ev.values.size() > 2 ? std::stoi(ev.values.at(2)) : 2;
                int numer = std::stoi(ev.values.at(1));
                TimeSig newTimeSig = {
                    TickToSeconds(tick), numer, (int)std::pow(2, denomExp), denomExp, tick
                };
                TS.push_back(newTimeSig);
            }
        }
        if (TS.empty()) {
            // Can assume 4/4 if none found
            TS.emplace_back(0, 4, 4, 2, 0);
        }
    }
    return TS;
}

std::vector<TrackInformation> &DotChartHandler::GetValidParts() {
    if (!processed) {
        for (auto &checkPart : SongPart::LoadedParts | std::views::values) {
            TrackInformation info;
            info.Part = &checkPart;
            std::string selectedSectionName;
            for (auto &checkSectionName : checkPart.dotChartSectionNames) {
                for (auto &diff : diffs) {
                    if (sections.contains(diff.first + checkSectionName)) {
                        selectedSectionName = checkSectionName;
                        break;
                    }
                }
                if (!selectedSectionName.empty()) {
                    break;
                }
            }
            if (selectedSectionName.empty()) {
                continue;
            }
            info.SectionName = selectedSectionName;
            for (auto &diff : diffs) {
                if (sections.contains(diff.first + selectedSectionName)) {
                    info.ValidDiffs.at(diff.second) = true;
                }
            }
            Parts.push_back(info);
        }

        PopulateAutoConverts();
        processed = true;
    }
    return Parts;
}

std::vector<TimePoint> DotChartHandler::GenerateOverdriveTicks() {
    // no BEAT track in .chart, have to always do the hacky thing
    // see TODO in MidiChartHandler::GenerateOverdriveTicks
    std::vector<TimePoint> ticks;

    int odTickCount = GetEndEvent().first / tpq;
    ticks.reserve(odTickCount);
    for (int i = 0; i < odTickCount; i++) {
        ticks.emplace_back(TickToSeconds(tpq * i), tpq * i);
    }
    return ticks;
}

std::vector<LyricPhrase> DotChartHandler::GetLyricPhrases() {
    return std::vector<LyricPhrase> {};
}

void DotChartHandler::PreloadCharts() {
    if (!sections.empty()) {
        return;
    }
    auto chart = song->GetChartData();
    std::string sectionName;
    std::vector<std::string> sectionLines;
    for (std::string line; std::getline(*chart, line);) {
        if (line.empty())
            continue;
        if (line.starts_with('{') || line.starts_with('}')) {
            continue;
        }
        // Kill the BOM
        if (line.starts_with("\uFEFF")) {
            line = line.substr(3);
        }
        if (line.ends_with('\r')) {
            line.pop_back();
        }
        // Skip over whitespace
        for (size_t i = 0; i < line.size(); i++) {
            if (line[i] > 32 && line[i] < 127) {
                line = line.substr(i);
                break;
            }
        }
        if (line.starts_with('[')) {
            if (!sectionName.empty()) {
                sections.emplace(sectionName, sectionLines);
                sectionLines.clear();
            }
            sectionName = line.substr(1, line.find_last_of(']') - 1);
            continue;
        }
        sectionLines.emplace_back(line);
    }
    if (!sectionName.empty()) {
        sections.emplace(sectionName, sectionLines);
    }

    if (sections.contains("Song")) {
        for (auto &line : sections["Song"]) {
            auto ev = ParseEvent(line);
            if (ev.key == "Resolution") {
                tpq = std::stoi(ev.values.at(0));
            }
        }
    } else {
        throw std::logic_error("Chart does not have a [Song] section");
    }
}

std::pair<TimeTicks, TimeSeconds> DotChartHandler::GetEndEvent() {
    GetBPMChanges();
    // First search the events section for an end event
    for (auto &line : sections["Events"]) {
        auto ev = ParseEvent(line);
        if (ev.values.at(0) == "E" && ev.values.at(1) == "end") {
            TimeTicks tick = std::stoi(ev.key);
            return { tick, TickToSeconds(tick) };
        }
    }
    // well shit, no end event, time to check every section for the latest event :)
    int latestTick = 0;
    for (auto &section : sections) {
        if (section.first != "Song") {
            for (auto &line : section.second) {
                auto ev = ParseEvent(line);
                // In case any other sections have non-tick keys
                try {
                    TimeTicks tick = std::stoi(ev.key);
                    if (tick > latestTick) {
                        latestTick = tick;
                    }
                } catch (std::exception e) {
                }
            }
        }
    }

    return { latestTick, TickToSeconds(latestTick) };
}

std::vector<Section> DotChartHandler::GetSections() {
    return {};
}
std::shared_ptr<BaseChart> DotChartHandler::GetChart(ChartLoadContext ctx) {
    for (auto& trackInfo : GetValidParts()) {
        if (ctx.part != trackInfo.Part) continue;
        if (!trackInfo.ValidDiffs.at(ctx.diff)) continue;
        switch (ctx.part->archetype) {
        case ArchetypePad:
            if (!trackInfo.Converted) {
                break;
            } else {
                auto chart = DotChartGuitarLoader(this, ctx).GetChart();
                return std::make_shared<BaseChart>(PadConverters::ConvertGuitarToPad(*chart));
            }
        case ArchetypeDrums:
            break;
        case ArchetypeGuitar:
            return DotChartGuitarLoader(this, ctx);
        }
    }
    return std::make_shared<BaseChart>();

}

bool DotChartHandler::IsLoaded() {
    return true;
}
void DotChartHandler::PopulateSubsongs() {}
//
// Created by maria on 15/06/2026.
//

#include "MidiChartHandler.h"

#include <regex>

#include "assets.h"
#include "MidiLyricLoader.h"
#include "Drums/MidiDrumsLoader.h"
#include "Guitar/MidiGuitarLoader.h"
#include "Pad/MidiPadLoader.h"
#include "RhythmEngine/ChartLoaders/PadConverters/PadConverters.h"
#include "RhythmEngine/Overdrive/OverdriveTicks.h"
#include "util/threadpool.h"
std::vector<std::vector<int>> pDiffRangeNotes = {
    { 60, 64 }, { 72, 76 }, { 84, 88 }, { 96, 100 }
};

SongPart Encore::RhythmEngine::MidiChartHandler::GetSongPart(smf::MidiEventList &track) {
    for (int events = 0; events < track.getSize(); events++) {
        if (!track[events].isMeta())
            continue;
        if ((int)track[events][1] == 3) {
            std::string trackName;
            for (int k = 3; k < track[events].getSize(); k++) {
                trackName += track[events][k];
            }
            return partFromStringINI(trackName);
        }
    }
    return Invalid;
}

Encore::RhythmEngine::MidiChartHandler::MidiChartHandler(std::filesystem::path fileName)
    : ChartHandler(std::move(fileName)) {
    midifile.read(file.string());
    midifile.doTimeAnalysis();
    midifile.linkNotePairs();
    tpq = midifile.getTPQ();
    MidiChartHandler::GetValidParts();
}

std::vector<BPM> Encore::RhythmEngine::MidiChartHandler::GetBPMChanges() {
    std::vector<BPM> bpms;
    tpq = midifile.getTicksPerQuarterNote();
    smf::MidiEventList &track = midifile[0];
    for (int i = 0; i < track.getSize(); i++) {
        if (track[i].isTempo()) {
            bpms.emplace_back(
                track[i].seconds, track[i].getTempoBPM()*TheAudioManager.songSpeed, track[i].tick
            );
        }

    }

    return bpms;
}

std::vector<TimeSig> Encore::RhythmEngine::MidiChartHandler::GetTimeSigChanges() {
    std::vector<TimeSig> ts;
    tpq = midifile.getTicksPerQuarterNote();
    smf::MidiEventList &track = midifile[0];
    for (int i = 0; i < track.getSize(); i++) {
        if (track[i].isMeta() && track[i][1] == 0x58) {
            int numer = (int)track[i][3];
            int denom = pow(2, track[i][4]);
            ts.emplace_back(
                track[i].seconds, numer, denom, track[i][4], track[i].tick
            );
        }
    }
    if (ts.empty()) {
        ts.emplace_back(0, 4, 4, 2, 0); // midi always assumed to be 4/4 if
        // time sig
        // event isn't found
    }
    return ts;
}

std::array<TrackInformation, PlasticVocals> &Encore::RhythmEngine::MidiChartHandler::
GetValidParts() {
    if (processed) {
        return Parts;
    }

    for (int track = 0; track < midifile.getTrackCount(); track++) {
        SongPart songPart = GetSongPart(midifile[track]);
        if (songPart == Invalid)
            continue;
        for (int diff = 0; diff < 4; diff++) {
            for (int i = 0; i < midifile[track].getSize(); i++) {
                if (midifile[track][i].isNoteOn() && !midifile[track][i].isMeta()
                    && midifile[track][i][1] >= pDiffRangeNotes[diff][0]
                    && midifile[track][i][1] <= pDiffRangeNotes[diff][1]) {
                    Parts[songPart].ValidDiffs.at(diff) = true;
                    Parts[songPart].TrackInt = track;
                    Parts[songPart].Valid = true;
                    break;
                }
            }
        }
        if (songPart == Events) {
            EventsTrack = track;
        }
    }
    if (!Parts[PartGuitar].Valid && Parts[PlasticGuitar].Valid) {
        Parts[PartGuitar] = Parts[PlasticGuitar];
        Parts[PartGuitar].AutoToPad = true;
    }
    if (!Parts[PartBass].Valid && Parts[PlasticBass].Valid) {
        Parts[PartBass] = Parts[PlasticBass];
        Parts[PartBass].AutoToPad = true;
    }
    if (!Parts[PartKeys].Valid && Parts[PlasticKeys].Valid) {
        Parts[PartKeys] = Parts[PlasticKeys];
        Parts[PartKeys].AutoToPad = true;
    }
    if (Parts[PitchedVocals].Valid) {
        Parts[PitchedVocals].Valid = false;
    }
    processed = true;
    return Parts;
}

Encore::RhythmEngine::OverdriveTicks
Encore::RhythmEngine::MidiChartHandler::GenerateOverdriveTicks() {
    ZoneScoped;
    //midiFile.doTimeAnalysis();
    smf::MidiEventList &track = midifile[EventsTrack];
    OverdriveTicks overdriveTicks;
    track.linkEventPairs();
    // 12 and 13 are the beats
    for (int i = 0; i < track.getSize(); i++) {
        if (track[i].isNoteOn() && (track[i][1] == 12 || track[i][1] == 13)) {
            overdriveTicks.ticks.emplace_back(track[i].seconds, track[i].tick);
        }
    }

    // todo: make this actually measure based potentially. this fucks over gh songs with non x/4 time sigs.
    if (overdriveTicks.ticks.empty()) {
        int overdriveTickCount = midifile.getFileDurationInTicks() / TheSongTime.songPPQN;
        for (int i = 0; i < overdriveTickCount; i++) {
            overdriveTicks.ticks.emplace_back(
                midifile.getTimeInSeconds(TheSongTime.songPPQN * i),
                TheSongTime.songPPQN * i);
        }
    }

    return overdriveTicks;
}

std::vector<Encore::RhythmEngine::EncLyricPhrase>
Encore::RhythmEngine::MidiChartHandler::GetLyricPhrases() {
    return Lyrics;
}

void Encore::RhythmEngine::MidiChartHandler::LoadCharts() {
    ThreadPool threads = std::thread::hardware_concurrency() - 1;

    for (auto & Part : Parts) {
        if (Part.TrackInt == -1)
            continue;
        midifile[Part.TrackInt].linkEventPairs();
    }
    for (int part = 0; part < Parts.size(); part++) {
        if (Parts[part].TrackInt == -1)
            continue;
        if (threshold == -1) {
            threshold = (tpq / 3) + 1;
        }
        int maxMult = 4;
        if (part == PartBass || part == PartVocals) {
            maxMult = 6;
        }
        switch (part) {
        case PartDrums:
        case PartBass:
        case PartGuitar:
        case PartKeys:
        case PartVocals: {
            ZoneScopedN("Pad loader")
            for (int diff = 0; diff < 4; diff++) {
                if (!Parts[part].ValidDiffs[diff])
                    continue;
                if (!Parts[part].AutoToPad) {
                    MidiPadLoader loader(diff, tpq, &midifile);
                    loader.LoadChart(midifile[Parts[part].TrackInt]);
                    Charts.at(part).first.at(diff) = loader.chart;
                } else {
                    MidiGuitarLoader chartLoader(diff, threshold, &midifile, maxMult);
                    chartLoader.chart.sections = TheSongTime.Sections;
                    chartLoader.LoadChart(midifile[Parts[part].TrackInt]);
                    Charts.at(part).first.at(diff) =
                        PadConverters::ConvertGuitarToPad(chartLoader.chart);
                }
            }
            break;
        }
        case PlasticDrums: {
            ZoneScopedN("Drums loader")
            for (int diff = 0; diff < 4; diff++) {
                if (!Parts[part].ValidDiffs[diff])
                    continue;
                MidiDrumsLoader chartLoader(diff, &midifile);
                chartLoader.chart.sections = TheSongTime.Sections;
                chartLoader.LoadChart(midifile[Parts[part].TrackInt]);
                Charts.at(part).first.at(diff) = chartLoader.chart;
            }
            break;
        }
        case PlasticBass:
        case PlasticGuitar:
        case PlasticKeys: {
            ZoneScopedN("Classic loader")
            for (int diff = 0; diff < 4; diff++) {
                if (!Parts[part].ValidDiffs[diff])
                    continue;
                MidiGuitarLoader chartLoader(diff, threshold, &midifile, maxMult);
                chartLoader.chart.sections = TheSongTime.Sections;
                chartLoader.LoadChart(midifile[Parts[part].TrackInt]);
                Charts.at(part).first.at(diff) = chartLoader.chart;
            }
            break;
        }
        case PitchedVocals: {
            ZoneScopedN("Lyric loader")
            MidiLyricLoader lyricLoader(&midifile, Parts[part].TrackInt);
            lyricLoader.LoadLyrics();
            Lyrics = lyricLoader.lyrics;
            break;
        }
        default:
            break;
        }
    }
}

std::pair<int, double> Encore::RhythmEngine::MidiChartHandler::GetEndEvent() {
    auto &events = midifile[EventsTrack];
    for (int i = 0; i < events.getSize(); i++) {
        if (events[i].isMeta() && (int)events[i][1] == 1) {
            std::string evt_string = "";
            for (int k = 3; k < events[i].getSize(); k++) {
                evt_string += events[i][k];
            }
            if (evt_string == "[end]") {
                Log::Trace("SONG: Song end: {:5.4f}", events[i].seconds);
                return { events[i].tick, events[i].seconds };
            }
        }
    }
    return { midifile.getFileDurationInTicks(), midifile.getFileDurationInSeconds() };
}

std::vector<Section> Encore::RhythmEngine::MidiChartHandler::GetSections() {
    std::vector<Section> sections;
    auto &events = midifile[EventsTrack];
    for (int i = 0; i < events.getSize(); i++) {
        auto &event = events[i];
        std::string evt_string;
        evt_string.reserve(event.getSize());
        for (int k = 3; k < event.getSize(); k++) {
            evt_string += event[k];
        }
        {
            ZoneScopedN("Regex");
            // I'm sorry.
            static const std::regex practiceRegex("\\[((prc_)|(section ))(.+?)\\]");
            std::smatch match;
            std::regex_match(evt_string, match, practiceRegex);
            if (match[4].matched) {
                sections.push_back({ match[4], event.seconds, event.tick });
            }
        }
    }
    return sections;
}

Encore::RhythmEngine::BaseChart Encore::RhythmEngine::MidiChartHandler::GetChart(int part,
    int diff) {
    return Charts.at(part).first.at(diff);
}

bool Encore::RhythmEngine::MidiChartHandler::IsLoaded() {
return true;
}
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
#include "util/threadpool.h"
using namespace Encore::RhythmEngine;

std::vector<std::vector<int> > pDiffRangeNotes = {
    { 60, 64 }, { 72, 76 }, { 84, 88 }, { 96, 100 }
};

std::string MidiChartHandler::GetMidiTrackName(smf::MidiEventList &track) {
    for (int events = 0; events < track.getSize(); events++) {
        if (!track[events].isMeta())
            continue;
        if ((int)track[events][1] == 3) {
            std::string trackName;
            for (int k = 3; k < track[events].getSize(); k++) {
                trackName += track[events][k];
            }
            return trackName;
        }
    }
    return "";
}

MidiChartHandler::MidiChartHandler(Song *song) : ChartHandler(song) {
    midifile.read(*song->GetChartData());
    midifile.doTimeAnalysis();
    midifile.linkNotePairs();
    tpq = midifile.getTPQ();
    MidiChartHandler::GetValidParts();
}

std::vector<BPM> MidiChartHandler::GetBPMChanges() {
    std::vector<BPM> bpms;
    tpq = midifile.getTicksPerQuarterNote();
    smf::MidiEventList &track = midifile[0];
    for (int i = 0; i < track.getSize(); i++) {
        if (track[i].isTempo()) {
            bpms.emplace_back(
                track[i].seconds,
                track[i].getTempoBPM() * TheAudioManager.songSpeed,
                track[i].tick
            );
        }
    }

    return bpms;
}

std::vector<TimeSig> MidiChartHandler::GetTimeSigChanges() {
    std::vector<TimeSig> ts;
    tpq = midifile.getTicksPerQuarterNote();
    smf::MidiEventList &track = midifile[0];
    for (int i = 0; i < track.getSize(); i++) {
        if (track[i].isMeta() && track[i][1] == 0x58) {
            int numer = (int)track[i][3];
            int denom = pow(2, track[i][4]);
            ts.emplace_back(track[i].seconds, numer, denom, track[i][4], track[i].tick);
        }
    }
    if (ts.empty()) {
        ts.emplace_back(0, 4, 4, 2, 0); // midi always assumed to be 4/4 if
        // time sig
        // event isn't found
    }
    return ts;
}

std::vector<TrackInformation> &MidiChartHandler::GetValidParts() {
    if (processed) {
        return Parts;
    }

    for (int track = 0; track < midifile.getTrackCount(); track++) {
        auto trackName = GetMidiTrackName(midifile[track]);
        if (trackName.empty())
            continue;
        if (trackName == "EVENTS") {
            EventsTrack = track;
            continue;
        }
        if (trackName == "PART VOCALS") {
            VoxTrack = track;
            continue;
        }
        auto songParts = SongPart::GetFromMidiTrackName(trackName);
        for (auto songPart : songParts) {
            if (!songPart->playable)
                continue;
            TrackInformation info;
            info.Part = songPart;
            info.TrackIndex = track;
            bool valid = false;
            for (int diff = 0; diff < 4; diff++) {
                for (int i = 0; i < midifile[track].getSize(); i++) {
                    if (midifile[track][i].isNoteOn() && !midifile[track][i].isMeta()
                        && midifile[track][i][1] >= pDiffRangeNotes[diff][0]
                        && midifile[track][i][1] <= pDiffRangeNotes[diff][1]) {
                        info.ValidDiffs.at(diff) = true;
                        valid = true;
                        break;
                    }
                }
            }
            if (valid) {
                Parts.push_back(info);
            }
        }
    }
    // Find auto-converts
    for (auto &convertToPart : SongPart::LoadedParts | std::views::values) {
        if (convertToPart.convertsFrom.empty())
            continue;
        // Check if we're missing this part
        bool missing = true;
        for (auto &checkPart : Parts) {
            if (checkPart.Part == &convertToPart) {
                missing = false;
                break;
            }
        }
        if (!missing)
            continue;

        // Find the part to convert from
        for (auto &checkPart : Parts) {
            if (checkPart.Part->id == convertToPart.convertsFrom) {
                TrackInformation newPart = checkPart;
                newPart.Part = &convertToPart;
                newPart.Converted = true;
                Parts.push_back(newPart);
                break;
            }
        }
    }
    std::ranges::sort(
        Parts.begin(), Parts.end(), [](TrackInformation &a, TrackInformation &b) {
            return a.Part->sortOrder > b.Part->sortOrder;
        }
    );
    processed = true;
    return Parts;
}

std::vector<TimePoint> MidiChartHandler::GenerateOverdriveTicks() {
    ZoneScoped;
    // midiFile.doTimeAnalysis();
    smf::MidiEventList &track = midifile[EventsTrack];
    std::vector<TimePoint> ticks;
    track.linkEventPairs();
    // 12 and 13 are the beats
    for (int i = 0; i < track.getSize(); i++) {
        if (track[i].isNoteOn() && (track[i][1] == 12 || track[i][1] == 13)) {
            ticks.emplace_back(track[i].seconds, track[i].tick);
        }
    }

    // todo: make this actually measure based potentially. this fucks over gh songs with
    // non x/4 time sigs.
    if (ticks.empty()) {
        int overdriveTickCount = midifile.getFileDurationInTicks() / tpq;
        for (int i = 0; i < overdriveTickCount; i++) {
            ticks.emplace_back(midifile.getTimeInSeconds(tpq * i), tpq * i);
        }
    }

    return ticks;
}

std::vector<LyricPhrase> MidiChartHandler::GetLyricPhrases() {
    return Lyrics;
}

void MidiChartHandler::PreloadCharts() {
    if (threshold == -1) {
        threshold = (tpq / 3) + 1;
    }
    if (VoxTrack != -1) {
        MidiLyricLoader lyricLoader(&midifile, VoxTrack);
        lyricLoader.LoadLyrics();
        Lyrics = lyricLoader.lyrics;
    }
    overdrive.resize(midifile.getTrackCount());
    for (auto &trackInfo : GetValidParts()) {
        auto &track = midifile[trackInfo.TrackIndex];
        for (int i = 0; i < track.size(); i++) {
            auto &event = track[i];
            if (event[1] == 116 && event.isNoteOn()) {
                overdrive.at(trackInfo.TrackIndex)
                    .emplace_back(
                        event.seconds,
                        event.tick,
                        event.getLinkedEvent()->seconds,
                        event.getLinkedEvent()->tick
                    );
                continue;
            }
        }
    }
}

std::pair<int, double> MidiChartHandler::GetEndEvent() {
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

std::vector<Section> MidiChartHandler::GetSections() {
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
std::shared_ptr<BaseChart> MidiChartHandler::GetChart(ChartLoadContext ctx) {
    for (auto &trackInfo : GetValidParts()) {
        if (trackInfo.Part != ctx.part)
            continue;
        if (!trackInfo.ValidDiffs.at(ctx.diff))
            continue;
        switch (trackInfo.Part->archetype) {
        case ArchetypePad: {
            if (!trackInfo.Converted) {
                MidiPadLoader loader(ctx, tpq, &midifile);
                loader.LoadChart(midifile[trackInfo.TrackIndex]);
                return std::make_shared<BaseChart>(loader.chart);
            } else {
                if (SongPart::LoadedParts[trackInfo.Part->convertsFrom].archetype
                    == ArchetypeGuitar) {
                    MidiGuitarLoader chartLoader(ctx, threshold, &midifile);
                    chartLoader.chart.sections = TheSongTime.Sections;
                    chartLoader.LoadChart(midifile[trackInfo.TrackIndex]);
                    return std::make_shared<BaseChart>(
                        PadConverters::ConvertGuitarToPad(chartLoader.chart)
                    );
                }
            }
            break;
        }
        case ArchetypeDrums: {
            MidiDrumsLoader loader(ctx, &midifile);
            loader.LoadChart(midifile[trackInfo.TrackIndex]);
            return std::make_shared<BaseChart>(loader.chart);
            break;
        }
        case ArchetypeGuitar:
            MidiGuitarLoader loader(ctx, threshold, &midifile);
            loader.LoadChart(midifile[trackInfo.TrackIndex]);
            return std::make_shared<BaseChart>(loader.chart);
            break;
        }
    }
    return nullptr;
}

bool MidiChartHandler::IsLoaded() {
    return true;
}
void MidiChartHandler::PopulateSubsongs() {
    if (song->subSongs.empty()) {
        return;
    }
    auto &events = midifile[EventsTrack];
    int currentSubsong = -1;
    for (int i = 0; i < events.getSize(); i++) {
        auto &event = events[i];
        std::string evt_string;
        evt_string.reserve(event.getSize());
        for (int k = 3; k < event.getSize(); k++) {
            evt_string += event[k];
        }

        if (evt_string == "[subsong]") {
            currentSubsong++;
            if (currentSubsong >= song->subSongs.size()) {
                Log::Error(
                    "Song has more [subsong] events than subsongs defined in metadata. Something is wrong!"
                );
                return;
            }
            auto &subsong = song->subSongs[currentSubsong];
            subsong.startTime = event.seconds;
            subsong.enterTime = event.seconds;
        }

        if (evt_string == "[subsong_enter]") {
            if (currentSubsong < 0) {
                // "supported yet" I'll probably allow these to be placed before a subsong
                // in order to control the base song's MTV overlay
                Log::Error(
                    "Song has a [subsong_enter] event before any [subsong] events. This isn't supported yet."
                );
                return;
            }
            auto &subsong = song->subSongs[currentSubsong];
            subsong.enterTime = event.seconds;
        }

        if (evt_string == "[subsong_exit]") {
            if (currentSubsong < 0) {
                Log::Error(
                    "Song has a [subsong_exit] event before any [subsong] events. This isn't supported yet."
                );
                return;
            }
            auto &subsong = song->subSongs[currentSubsong];
            subsong.exitTime = event.seconds;
        }
    }
}

#include "enctime.h"

#include "settings/settings.h"
#include "song/songlist.h"

#include <cmath>
#include "song/audio.h"

#include <regex>

SongTime TheSongTime;

void SongTime::BeatmapFromMidiTrack(smf::MidiFile &midiFile, int songEndTick) {
    ZoneScoped;
    //midiFile.doTimeAnalysis();
    smf::MidiEventList &track = midiFile[0];
    track.linkEventPairs();
    for (int i = 0; i < track.getSize(); i++) {
        if (track[i].isTempo()) {
            BPMChanges.emplace_back(
                track[i].seconds, track[i].getTempoBPM()*TheAudioManager.songSpeed, track[i].tick
            );
            // std::cout << "BPM @" << midiFile.getTimeInSeconds(trkidx, i) << ": "
            //           << events[i].getTempoBPM() << std::endl;
        } else if (track[i].isMeta() && track[i][1] == 0x58) {
            int numer = (int)track[i][3];
            int denom = pow(2, track[i][4]);
            TimeSigChanges.emplace_back(
                track[i].seconds, numer, denom, track[i][4], track[i].tick
            );
            // std::cout << "TIMESIG @" << midiFile.getTimeInSeconds(trkidx, i) << ":
            // "
            //           << numer << "/" << denom << std::endl;
        }
    }
    if (TimeSigChanges.empty()) {
        TimeSigChanges.emplace_back(0, 4, 4, 2, 0); // midi always assumed to be 4/4 if
                                                    // time sig
        // event isn't found
    }
    GenerateBeatmap(songEndTick);
}

/*
 *
 *
 * do calculations using curtick/lastick
 *
 *
 *
 */

bool compareTicks(const OverdriveTick &a, const OverdriveTick &b) {
    return a.tick < b.tick;
}
bool equalTicks(const OverdriveTick &a, const OverdriveTick &b) {
    return a.tick == b.tick;
}

void SongTime::GenerateOverdriveTicks(smf::MidiFile &midiFile, int TrackID) {
    ZoneScoped;
    //midiFile.doTimeAnalysis();
    smf::MidiEventList &track = midiFile[TrackID];
    track.linkEventPairs();
    // 12 and 13 are the beats
    for (int i = 0; i < track.getSize(); i++) {
        if (track[i].isNoteOn() && (track[i][1] == 12 || track[i][1] == 13)) {
            OverdriveTicks.emplace_back(track[i].seconds, track[i].tick);
        }
    }

    // todo: make this actually measure based potentially. this fucks over gh songs with non x/4 time sigs.
    if (OverdriveTicks.size() == 0) {
        int overdriveTickCount = midiFile.getFileDurationInTicks() / 480;
        for (int i = 0; i < overdriveTickCount; i++) {
            OverdriveTicks.emplace_back(midiFile.getTimeInSeconds(480 * i), 480 * i);
        }
    }
};

void SongTime::UpdateOverdriveTick() {
    // this is the overdrive code right here
    // std::cout << std::endl;
    double CurrentTime = GetElapsedTime();
    if (OverdriveTicks.size() == 0)
        return;
    while (CurrentODTickItr < OverdriveTicks.size() - 1) {
        // if the next tick's time is greater than the current time, stop
        // otherwise, just increase lmfao
        // std::cout << std::endl;


        const auto &NextTick = OverdriveTicks.at(CurrentODTickItr + 1);
        // Encore::EncoreLog(LOG_DEBUG, TextFormat("Next tick time: %4.4f",
        // NextTick.time)); Encore::EncoreLog(LOG_DEBUG, TextFormat("Current time: %4.4f",
        // CurrentTime));
        if (NextTick.time > CurrentTime) {
            // Encore::EncoreLog(LOG_DEBUG, TextFormat("Current Overdrive Tick: %01i",
            // (CurrentODTickItr)));

            break;
        };

        ++CurrentODTickItr;
        // Encore::EncoreLog(LOG_DEBUG, TextFormat("Skipping to next OD tick: %01i",
        // (CurrentODTickItr)));
    }
    LastODTick = CurrentODTick;
    // get the time since the last beat
    double timeDelta = CurrentTime - OverdriveTicks.at(CurrentODTickItr).time;
    // double tickDelta = CurrentODTick - OverdriveTicks.at(CurrentODTickItr).tick;
    // what if i legit delt with ticks. it would be funny and i could totally do it
    // get the total time of this tick
    // double tickBetweenTicks = OverdriveTicks.at(CurrentODTickItr + 1).tick
    //     - OverdriveTicks.at(CurrentODTickItr).tick;

    // double tickDeltaToPct = tickDelta / tickBetweenTicks;
    if (CurrentODTickItr == OverdriveTicks.size() - 1) {
        double timeBetweenTicks =
            (GetElapsedTime() + 10) - OverdriveTicks.at(CurrentODTickItr).time;

        double deltaMappedToPercentage = timeDelta / timeBetweenTicks;
        CurrentODTick = CurrentODTickItr + deltaMappedToPercentage;
    } else {
        double timeBetweenTicks = OverdriveTicks.at(CurrentODTickItr + 1).time
            - OverdriveTicks.at(CurrentODTickItr).time;

        double deltaMappedToPercentage = timeDelta / timeBetweenTicks;
        CurrentODTick = CurrentODTickItr + deltaMappedToPercentage;
    }
}
void SongTime::ParseSections(smf::MidiFile midiFile) {
    ZoneScoped;
    Sections.clear();
    for (int track = 0; track < midiFile.getTrackCount(); track++) {
        SongParts songPart = TheSongList.curSong->GetSongPart(midiFile[track]);
        TheSongList.curSong->IsPartValid(midiFile[track], songPart, track);
        if (songPart == Events) {
            auto &trackObj = midiFile[track];
            for (int i = 0; i < trackObj.getSize(); i++) {
                auto &event = trackObj[i];
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
                        Sections.push_back({match[4], event.seconds});
                    }
                }
            }
        }
    }
}

void SongTime::UpdateTick() {
    while (CurrentBPM < BPMChanges.size() - 1) {
        const auto &NextBPM = BPMChanges.at(CurrentBPM + 1);
        if (NextBPM.tick > CurrentTick) {
            break;
        };
        CurrentBPM++;
    }
    LastTick = CurrentTick; // updates last tick to current tick before tick calculations

    // time between the bpm's time and the current time
    double timeDelta = GetElapsedTime() - BPMChanges.at(CurrentBPM).time;

    // get beats since the beginning of the BPM
    double beatDelta = timeDelta * BPMChanges.at(CurrentBPM).bpm / 60.0;

    // add the last bpm's tick to the (beat delta * resolution)
    CurrentTick = BPMChanges.at(CurrentBPM).tick + (beatDelta * 480.0);
}
double SongTime::GetCurrentTick() const {
    return CurrentTick;
}
double SongTime::GetLastTick() const {
    return LastTick;
}
/*
 * expects that you have already run BeatmapFromMidiTrack.
 * maybe put this in there to generate said Beatmap
 */

double SongTime::TimeRangeToTickDelta(double timeStart, double timeEnd, const BPM &bpm) {
    double timeDelta = timeEnd - timeStart;
    double beatDelta = timeDelta * bpm.bpm / 60.0;
    return beatDelta * 480.0;
}

// major is like. the subdivision
// minor is the sub subdivision...?

int GetBeatlineType(TimeSig curTimeSig, int beatlineCount) {
    int majorStep = 4;
    int numer = curTimeSig.denom == 4 ? curTimeSig.numer * 2 : curTimeSig.numer;
    int denom = curTimeSig.denom == 4 ? 8 : curTimeSig.denom;
    int measureBeatCount = beatlineCount % numer;
    // say, beat 2 of the measure. that would technically be beat 4.
    // measures need to be treated as DOUBLE.

    // 8 or 16 divided by 4 would equal 2 or 4
    // since denom would be doubled, it would be 16 or 32 equalling 4 or 8.
    // actually. i think major rate being 2 makes more sense in this case, since
    // major beatlines are supposed to be representative of the actual beat (esp in 4/4)
    int majorRate = denom <= 4 ? 2 : denom / majorStep;

    // 1/x time sigs
    if (numer == 2) {
        // if this is the first beat of the time sig region, just make it a measure
        if (beatlineCount < 1) {
            return Measure;
        }
        // basically acts as one long measure
        return (beatlineCount % majorRate) == 0 ? Major : Minor;
    }

    if (measureBeatCount == 0)
        return Measure;

    // well if it isnt a measure beat its something else
    // if (denomDoubled <= 8)
    //    return Major;

    if ((measureBeatCount % majorRate) == 0) {
        // if its the last beat of the measure, its a minor beatline
        if (measureBeatCount == numer - 1)
            return Minor;
        // if its a major subdiv, make it major lol
        return Major;
    }
    return Minor;
}

void SongTime::CreateBeatlines(
    TimeSig timeSig, int startTick, int endTick, int &curTempo
) {
    // so actually this works fine in 4/4 but i realize in /8 that it gets *weird*
    // maybe have it so that /8 is just It Always?
    const int ThingForBeatSubdiv = timeSig.denom == 4 ? 8 : timeSig.denom;
    const int BeatSubdiv = (480 * 4) / ThingForBeatSubdiv;

    int BeatlineCount = 0;
    int curTick = startTick;
    auto &CurBPM = BPMChanges.at(curTempo);
    while (curTick <= endTick) {
        while (curTempo < BPMChanges.size() - 1) {
            const auto &NextBPM = BPMChanges.at(curTempo + 1);
            if (NextBPM.tick > curTick) {
                break;
            };
            CurBPM = NextBPM;
            curTempo++;
        } // such a funky way to progress through this whilst checking behind
        // and forwards

        Beatlines.emplace_back(
            TheSongList.curSong->midiFile.getTimeInSeconds(curTick),
            // TimeSinceBPMStart(CurBPM, curTick),
            curTick,
            GetBeatlineType(timeSig, BeatlineCount)
        );
        curTick += BeatSubdiv;
        BeatlineCount++;
    }
}

void SongTime::GenerateBeatmap(int songEndTick) {
    int curTSidx = 0;
    int curTempo = 0;
    auto &CurTS = TimeSigChanges.at(curTSidx);
    for (; curTSidx < TimeSigChanges.size() - 1; curTSidx++) {
        const auto &NextTS = TimeSigChanges.at(curTSidx + 1);

        const int StartTick = CurTS.tick;
        const int EndTick = NextTS.tick - 1;

        CreateBeatlines(CurTS, StartTick, EndTick, curTempo);
        CurTS = NextTS;
    }

    CreateBeatlines(CurTS, CurTS.tick, songEndTick, curTempo);
}

void SongTime::SetOffset(double audioCalibration) {
    aCalib = audioCalibration;
};

void SongTime::Start(double end) {
    endTime = end;
    lastTimeSample = GetTime();
}

// Resets internal state, 
void SongTime::Reset() {
    pauseTime = 0.0;
    running = false;
    paused = false;
}

void SongTime::Pause() {
    pauseTime = GetElapsedTime();
    TheAudioManager.pauseStreams();
    paused = true;
};

void SongTime::Resume() {
    TheAudioManager.seekStreams(pauseTime - 5 + aCalib);
    TheAudioManager.unpauseStreams();
    paused = false;
};

void SongTime::Stop() {
    running = false;
    paused = false;
    TheAudioManager.unloadStreams();
}

bool SongTime::Running() {
    return running;
}

double SongTime::GetElapsedTime() {
    double audioTime = TheAudioManager.GetMusicTimePlayed();
    if (audioTime >= TheAudioManager.GetMusicTimeLength()) {
        return TheAudioManager.GetMusicTimeLength() + (GetTime() - lastTimeSample);
    }
    lastTimeSample = GetTime();
    return audioTime - aCalib + TheGameSettings.VideoOffset/1000.0;
};
double SongTime::GetStartTime() {
    return 0;
}

// this is actually a lie. it returns "the system time when it ends" i think i forgort
// use GetSongLength if you need song duration
double SongTime::GetEndTime() {
    return endTime;
}

double SongTime::GetSongLength() {
    return endTime;
}

bool SongTime::SongComplete() {
    if (running) {
        return GetElapsedTime() > endTime;
    }
    return false;
}
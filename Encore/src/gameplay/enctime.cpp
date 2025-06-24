
#include "enctime.h"

#include "util/enclog.h"

#include <algorithm>
#include <iostream>
#include <cmath>

SongTime TheSongTime;

void SongTime::BeatmapFromMidiTrack(smf::MidiFile &midiFile, int songEndTick) {
    midiFile.doTimeAnalysis();
    smf::MidiEventList &track = midiFile[0];
    for (int i = 0; i < track.getSize(); i++) {
        if (track[i].isTempo()) {
            BPMChanges.emplace_back(
                track[i].seconds, track[i].getTempoBPM(), track[i].tick
            );
            // std::cout << "BPM @" << midiFile.getTimeInSeconds(trkidx, i) << ": "
            //           << events[i].getTempoBPM() << std::endl;
        } else if (track[i].isMeta() && track[i][1] == 0x58) {
            int numer = (int)track[i][3];
            int denom = pow(2, track[i][4]);
            TimeSigChanges.emplace_back(track[i].seconds, numer, denom, track[i].tick);
            // std::cout << "TIMESIG @" << midiFile.getTimeInSeconds(trkidx, i) << ":
            // "
            //           << numer << "/" << denom << std::endl;
        }
    }
    if (TimeSigChanges.empty()) {
        TimeSigChanges.emplace_back(0, 4, 4, 0); // midi always assumed to be 4/4 if time
                                                 // sig
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
    midiFile.doTimeAnalysis();
    smf::MidiEventList &track = midiFile[TrackID];
    track.linkEventPairs();
    // 12 and 13 are the beats
    for (int i = 0; i < track.getSize(); i++) {
        if (track[i].isNoteOn() && (track[i][1] == 12 || track[i][1] == 13)) {
            OverdriveTicks.emplace_back(track[i].seconds, track[i].tick);
        }
    }
};

void SongTime::UpdateOverdriveTick() {
    // this is the overdrive code right here
    std::cout << std::endl;
    double CurrentTime = GetElapsedTime();
    while (CurrentODTickItr < OverdriveTicks.size() - 1) {
        // if the next tick's time is greater than the current time, stop
        // otherwise, just increase lmfao
        std::cout << std::endl;
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
    double timeBetweenTicks = OverdriveTicks.at(CurrentODTickItr + 1).time
        - OverdriveTicks.at(CurrentODTickItr).time;

    double deltaMappedToPercentage = timeDelta / timeBetweenTicks;
    CurrentODTick = CurrentODTickItr + deltaMappedToPercentage;
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

double TimeRangeToTickDelta(double timeStart, double timeEnd, BPM bpm) {
    double timeDelta = timeEnd - timeStart;
    double beatDelta = timeDelta * bpm.bpm / 60.0;
    return beatDelta * 480.0;
}
double TickRangeToTimeDelta(int tickStart, int tickEnd, BPM currentBPM) {
    if (tickStart < currentBPM.tick)
        return 0;
    if (tickEnd < tickStart)
        return 0;

    int tickDelta = tickEnd - tickStart;
    double beatDelta = tickDelta / (double)480;
    double timeDelta = beatDelta * 60.0 / currentBPM.bpm;

    return timeDelta;
}
double TickToTime(BPM bpm, int endTick) {
    return bpm.time + TickRangeToTimeDelta(bpm.tick, endTick, bpm);
}

// major is like. the subdivision
// minor is the sub subdivision...?

int GetBeatlineType(TimeSig curTimeSig, int beatlineCount) {
    int majorStep = 4;
    int measureBeatCount = beatlineCount % curTimeSig.numer;
    // 8 or 16 divided by 4 would equal 2 or 4
    int majorRate = curTimeSig.denom <= 4 ? 1 : curTimeSig.denom / majorStep;

    // 1/x time sigs
    if (curTimeSig.numer == 1) {
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
    if (curTimeSig.denom <= 4)
        return Major;

    if ((measureBeatCount % majorRate) == 0) {
        // if its the last beat of the measure, its a minor beatline
        if (measureBeatCount == curTimeSig.numer - 1)
            return Minor;
        // if its a major subdiv, make it major lol
        return Major;
    }
    return Minor;
}

void SongTime::CreateBeatlines(TimeSig timeSig, int startTick, int endTick) {
    int CurrentTick = startTick;
    const int BeatSubdiv = (480 * 4) / timeSig.denom;
    auto &CurBPM = BPMChanges.at(CurrentBPM);
    int BeatlineCount = 0;
    for (; CurrentTick <= endTick; CurrentTick += BeatSubdiv) {
        while (CurrentBPM < BPMChanges.size() - 1) {
            const auto &NextBPM = BPMChanges.at(CurrentBPM + 1);
            if (NextBPM.tick > CurrentTick) {
                break;
            };
            CurBPM = NextBPM;
            CurrentBPM++;
        } // such a funky way to progress through this whilst checking behind
        // and forwards
        Beatlines.emplace_back(
            TickToTime(CurBPM, CurrentTick),
            CurrentTick,
            GetBeatlineType(timeSig, BeatlineCount)
        );
        BeatlineCount++;
    };
}

void SongTime::GenerateBeatmap(int songEndTick) {
    CurrentBPM = 0;
    int curTSidx = 0;
    auto &CurTS = TimeSigChanges.at(curTSidx);
    for (; curTSidx < TimeSigChanges.size() - 1; curTSidx++) {
        const auto &NextTS = TimeSigChanges.at(curTSidx + 1);

        const int StartTick = CurTS.tick;
        const int EndTick = NextTS.tick - 1;

        CreateBeatlines(CurTS, StartTick, EndTick);
        CurTS = NextTS;
    }

    CreateBeatlines(CurTS, CurTS.tick, songEndTick);
}

void SongTime::SetOffset(double audioCalibration) {
    aCalib = audioCalibration;
};

void SongTime::Reset() {
    pauseTime = 0.0;
    running = false;
    paused = false;
}
// start at audio beginning
void SongTime::Start(double end) {
    if (!running) {
        startTime = GetTime() + aCalib;
        endTime = end + aCalib;
        running = true;
        paused = false;
        std::cout << "Started gameplay";
    }
};
double SongTime::GetFakeStartTime() {
    return fakeStartTime;
}

// start at a specific time
void SongTime::Start(double start, double end) {
    if (!running) {
        startTime = GetTime() - start + aCalib;
        fakeStartTime = GetTime() + aCalib;
        endTime = end + aCalib;
        running = true;
        paused = false;
    }
};
void SongTime::Pause() {
    if (running && !paused) {
        pauseTime = GetTime();
        running = false;
        paused = true;
    }
};

void SongTime::Resume() {
    if (!running && paused) {
        double startOffset = GetTime() - pauseTime;
        startTime += startOffset + 3.0;
        pauseTime = 0.0;
        running = true;
        paused = false;
    }
};

void SongTime::Stop() {
    running = false;
    paused = false;
}
bool SongTime::Running() {
    return running;
}

double SongTime::GetElapsedTime() {
    if (!paused && running) {
        return GetTime() - startTime;
    } else if (paused) {
        return pauseTime - startTime;
    }
    return 0.0;
};
double SongTime::GetStartTime() {
    return startTime;
}

// this is actually a lie. it returns "the system time when it ends" i think i forgort
// use GetSongLength if you need song duration
double SongTime::GetEndTime() {
    return endTime + startTime;
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
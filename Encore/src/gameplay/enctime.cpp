
#include "enctime.h"

#include <iostream>
#include <cmath>

SongTime TheSongTime;

void SongTime::BeatmapFromMidiTrack(
    smf::MidiFile &midiFile, int TrackID, int songEndTick
) {
    smf::MidiEventList &track = midiFile[0];
    for (int i = 0; i < track.getSize(); i++) {
        if (track[i].isTempo()) {
            BPMChanges.emplace_back(
                midiFile.getTimeInSeconds(0, i), track[i].getTempoBPM(), track[i].tick
            );
            // std::cout << "BPM @" << midiFile.getTimeInSeconds(trkidx, i) << ": "
            //           << events[i].getTempoBPM() << std::endl;
        } else if (track[i].isMeta() && track[i][1] == 0x58) {
            int numer = (int)track[i][3];
            int denom = pow(2, (int)track[i][4]);
            TimeSigChanges.emplace_back(
                midiFile.getTimeInSeconds(0, i), numer, denom, track[i].tick
            );
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
void SongTime::UpdateTick() {
    while (CurrentBPM < BPMChanges.size() - 1) {
        const auto &NextBPM = BPMChanges.at(CurrentBPM + 1);
        if (NextBPM.tick > CurrentTick) {
            break;
        };
        CurrentBPM++;
    }
    LastTick = CurrentTick; // updates last tick to current tick before tick calculations
    double timeDelta = GetElapsedTime() - BPMChanges.at(CurrentBPM).time;
    double beatDelta = timeDelta * BPMChanges.at(CurrentBPM).bpm / 60.0;
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
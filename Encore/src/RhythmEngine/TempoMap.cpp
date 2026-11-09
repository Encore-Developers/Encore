#include "TempoMap.h"
#include "raymath.h"
using namespace Encore::RhythmEngine;

void BeatlineTracker::Advance(TimeSeconds time) {
    if (tempoMap->Beatlines.empty()) {
        return;
    }
    TimeSeconds nextBeatlineTime;
    do {
        if (currentBeatline == tempoMap->Beatlines.size()-1) {
            break;
        }
        nextBeatlineTime = tempoMap->Beatlines[currentBeatline+1].time;
        if (time >= nextBeatlineTime) {
            currentBeatline++;
        }
    } while (time >= nextBeatlineTime);
}
Beatline BeatlineTracker::GetBeatline(size_t index) {
    return tempoMap->Beatlines[index];
}
size_t BeatlineTracker::GetBeatlineCount() {
    return tempoMap->Beatlines.size();
}

TimeBeats TempoMap::SecondsToBeat(TimeSeconds seconds) {
    auto currentBpm = BPMChanges.begin();
    for (auto findBpm = BPMChanges.begin(); findBpm < BPMChanges.end(); ++findBpm) {
        if (findBpm->time <= seconds) {
            currentBpm = findBpm;
        } else {
            break;
        }
    }

    double timeDelta = seconds - currentBpm->time;
    double beatDelta = timeDelta * currentBpm->bpm / 60.0;

    return currentBpm->tick / (double)ppqn + beatDelta;
}
TimeSeconds TempoMap::BeatToSeconds(TimeBeats beat) {
    auto currentBpm = BPMChanges.begin();
    for (auto findBpm = BPMChanges.begin(); findBpm < BPMChanges.end(); ++findBpm) {
        if ((findBpm->tick/(double)ppqn) <= beat) {
            currentBpm = findBpm;
        } else {
            break;
        }
    }

    return currentBpm->time + (beat - (currentBpm->tick/(double)ppqn)) / (currentBpm->bpm / 60.0);
}
TimeODTicks TempoMap::SecondsToODTick(TimeSeconds seconds) const {
    size_t currentTick = 0;
    for (size_t i = 0; i < OverdriveTicks.size(); i++) {
        if (OverdriveTicks[i].sec <= seconds) {
            currentTick = i;
        } else {
            break;
        }
    }

    if (currentTick >= OverdriveTicks.size() - 1) {
        return OverdriveTicks.size() - 1;
    }

    return currentTick + ((seconds - OverdriveTicks[currentTick].sec) / (OverdriveTicks[currentTick+1].sec - OverdriveTicks[currentTick].sec));
}
TimeSeconds TempoMap::ODTickToSeconds(TimeODTicks odTick) const {
    if (odTick < 0) {
        return 0;
    }
    // Gracefully, these can corellate directly to indices
    if ((size_t)odTick >= OverdriveTicks.size() - 1) {
        return OverdriveTicks[OverdriveTicks.size() - 1].sec;
    }

    return Lerp(OverdriveTicks[(size_t)odTick].sec, OverdriveTicks[(size_t)odTick+1].sec, odTick-(size_t)odTick);
}
BeatlineTracker TempoMap::GetBeatlineTracker() {
    return {this, 0};
}

double TempoMap::GetBeatDelta(TimeSeconds time, int step) {
    TimeBeats beat = SecondsToBeat(time);
    return (beat-((size_t)(beat/step)))/step;
}

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

double GetTimeFromTick(const BPM& tempo, int resolution, int currentTick) {
    // using https://thenathannator.github.io/GuitarGame_ChartFormats/Implementation-Info/Time-Conversions/ as a reference
    double secondsPerBeat = 60.0 / tempo.bpm;
    double tickDelta = currentTick - tempo.tick;
    double beatDelta = tickDelta / resolution;
    double secondDelta = secondsPerBeat * beatDelta;
    return secondDelta + tempo.time;
};

void TempoMap::CreateBeatlines(TimeSig timeSig, int startTick, int endTick, int &curTempo) {
    // so actually this works fine in 4/4 but i realize in /8 that it gets *weird*
    // maybe have it so that /8 is just It Always?
    const int ThingForBeatSubdiv = timeSig.denom == 4 ? 8 : timeSig.denom;
    const int BeatSubdiv = (ppqn * 4) / ThingForBeatSubdiv;

    int BeatlineCount = 0;
    int curTick = startTick;
    const BPM *CurBPM = &BPMChanges.at(curTempo);
    while (curTick <= endTick) {
        while (curTempo < BPMChanges.size() - 1) {
            const auto &NextBPM = BPMChanges.at(curTempo + 1);
            if (NextBPM.tick > curTick) {
                break;
            };
            CurBPM = &NextBPM;
            curTempo++;
        } // such a funky way to progress through this whilst checking behind
        // and forwards

        Beatlines.emplace_back(
            GetTimeFromTick(*CurBPM, ppqn, curTick),
            // TimeSinceBPMStart(CurBPM, curTick),
            curTick,
            GetBeatlineType(timeSig, BeatlineCount)
        );
        curTick += BeatSubdiv;
        BeatlineCount++;
    }
}

void TempoMap::GenerateBeatmap(int songEndTick) {
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
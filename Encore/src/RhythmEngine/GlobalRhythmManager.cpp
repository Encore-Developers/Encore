//
// Created by maria on 06/05/2025.
//

#include "GlobalRhythmManager.h"

namespace Encore::RhythmEngine {
    double
    GlobalRhythmManager::TimeRangeToTickDelta(double timeStart, double timeEnd, BPM bpm) {
        double timeDelta = timeEnd - timeStart;
        double beatDelta = timeDelta * bpm.bpm / 60.0;
        return beatDelta * 480.0;
    }
    void GlobalRhythmManager::UpdateTime() {
        CurrentTime = TheSongTime.GetSongTime();
    }
    void GlobalRhythmManager::UpdateCurrentBeat() {
        if (TheSongTime.GetSongTime() > TheSongList.curSong->bpms[CurrentBPM].time
            && CurrentBPM < TheSongList.curSong->bpms.size() - 1)
            CurrentBPM++;
    }
    void GlobalRhythmManager::StartFrameTick() {
        UpdateCurrentBeat();
        CurrentTick = TheSongList.curSong->bpms[CurrentBPM].tick
            + TimeRangeToTickDelta(
                          TheSongList.curSong->bpms[CurrentBPM].time,
                          CurrentTime,
                          TheSongList.curSong->bpms[CurrentBPM]
            );
    }
    void GlobalRhythmManager::EndFrameTick() {
        LastTick = CurrentTick;
    }

}
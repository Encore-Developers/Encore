
#include "enctime.h"

#include <iostream>

SongTime TheSongTime;

void SongTime::SetOffset(double audioCalibration) {
    aCalib = audioCalibration;
};

void SongTime::Reset() {
    pauseTime = 0.0;
    running = false;
    paused = false;
}
void SongTime::Start(double end) {
    if (!running) {
        startTime = GetTime() + aCalib;
        endTime = end + aCalib;
        running = true;
        paused = false;
        std::cout << "Started gameplay";
    }
};
void SongTime::Start(double start, double end) {
    if (!running) {
        startTime = GetTime() - start + aCalib;
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

double SongTime::GetSongTime() {
    if (!paused && running) {
        return GetTime() - startTime;
    }
    else if (paused) {
        return pauseTime - startTime;
    }
    return 0.0;
};
double SongTime::GetStartTime() {
    return startTime;
}
double SongTime::GetEndTime() {
    return endTime + startTime;
}
double SongTime::GetSongLength() {
    return endTime;
}

bool SongTime::SongComplete() {
    if (running) {
        return GetSongTime() > endTime;
    }
    return false;
}
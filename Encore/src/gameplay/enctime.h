#pragma once
//
// Created by marie on 20/09/2024.
//

#include "raylib.h"

class SongTime {
private:
    double aCalib = 0.0;
    double startTime = 0.0;
    double endTime = 0.0;
    double pauseTime = 0.0;
    bool running = false;
    bool paused = false;

public:
    SongTime() {};

    // Start the timer
    void SetOffset(double audioCalibration);

    // TODO: implement pausing
    // TODO: reset after songs
    void Reset();
    void Start(double end);
    void Start(double start, double end);
    void Pause();
    void Resume();
    void Stop();
    double GetSongTime();
    double GetStartTime();
    double GetEndTime();
    double GetSongLength();
    bool SongComplete();
};

extern SongTime TheSongTime;

#pragma once
#include <map>
#include "easing/easing.h"

struct LerpState {
    float totalDuration;
    float elapsed;
    double value; // its a double because the easing library returns that
    easing_functions func;
    bool started;
};

class Lerp {
public:
    LerpState createLerp(std::string key, easing_functions ease, float duration, bool startAutomatically = true);
    void removeLerp(std::string key);
    void startLerp(std::string key);
    void updateStates();
    LerpState getState(std::string key);

protected:
    std::map<std::string, LerpState> activeLerps;
};
#include <string>
#include <cmath>
#include "easing/easing.h"
#include "game/lerp.h"
#include "raylib.h"

double clamp(double x, double upper, double lower)
{
    return std::min(upper, std::max(x, lower));
}

void Lerp::createLerp(std::string key, easing_functions ease, float duration, bool startAutomatically)
{
    if (this->activeLerps.contains(key))
        return;

    this->activeLerps[key] = { duration, 0, 0, ease, startAutomatically };
}

void Lerp::removeLerp(std::string key) {
    if (this->activeLerps.contains(key))
        this->activeLerps.erase(key);
}

void Lerp::startLerp(std::string key) {
    if (!this->activeLerps.contains(key))
        return;

    this->activeLerps[key].started = true;
}

void Lerp::updateStates()
{
    float dt = GetFrameTime();

    for (auto& it : this->activeLerps)
    {
        // remove manually :+1:
        if (!it.second.started || it.second.elapsed >= it.second.totalDuration)
            continue;

        it.second.elapsed += dt;
        it.second.value = clamp(getEasingFunction(it.second.func)(it.second.elapsed / it.second.totalDuration), 1, 0);
        printf("State %s, elapsed %f/%f, actual eased value: %f\n", it.first.c_str(), it.second.elapsed, it.second.totalDuration, it.second.value);
    }
}

LerpState Lerp::getState(std::string key)
{
    if (!this->activeLerps.contains(key)) {
        // key not found - return 1 as the lerp has either finished or doesnt exist at all
        return { 1, 1, 1, static_cast<easing_functions>(Linear), true };
    }

    return this->activeLerps[key];
}
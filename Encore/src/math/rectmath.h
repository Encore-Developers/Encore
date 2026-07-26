#pragma once

#include "math/glm.h"



inline Vector2 AspectFitRect(Vector2 smallSize, Vector2 bigSize)
{
    auto smallRatio = bigSize.x / bigSize.y;
    auto containerRatio = smallSize.x / smallSize.y;

    if (smallRatio < containerRatio)
    {
        auto ratio = bigSize.x / smallSize.x;
        auto height = smallSize.y * ratio;
        return {bigSize.x, height};
    } else
    {
        auto ratio = bigSize.y / smallSize.y;
        auto width = smallSize.x * ratio;
        return {width, bigSize.y};
    }
}

inline float AspectFitRectGetSize(Vector2 smallSize, Vector2 bigSize)
{
    auto smallRatio = bigSize.x / bigSize.y;
    auto containerRatio = smallSize.x / smallSize.y;

    if (smallRatio < containerRatio)
    {
        auto ratio = bigSize.x / smallSize.x;
        return ratio;
    } else
    {
        auto ratio = bigSize.y / smallSize.y;
        return ratio;
    }
}
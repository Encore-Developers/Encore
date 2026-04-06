#pragma once

#include "math/glm.h"

class Camera {
public:
    Vector3 position;
    Vector3 target;
    Vector3 up;
    float fovy;

    Matrix getMatrix();
};
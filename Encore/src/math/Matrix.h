#pragma once

#include "Vector.h"
class Matrix {
public:
    Vector4 x;
    Vector4 y;
    Vector4 z;
    Vector4 origin;

    Matrix() : x({1, 0, 0, 0}), y({0, 1, 0, 0}), z({0, 0, 1, 0}), origin( {0, 0, 0, 1}) {}
    Matrix(Vector4 x, Vector4 y, Vector4 z, Vector4 origin) : x(x), y(y), z(z), origin(origin) {}

    // Column-major
    // x.x y.x z.x o.x
    // x.y y.y z.y o.y
    // x.z y.z z.z o.z
    //  0   0   0   1

    static const Matrix IDENTITY;

    Vector4& operator[](int index) {
        [[unlikely]] if (index < 0 || index > 4) {
            // TODO: this should log
            return x;
        }
        return ((Vector4*)this)[index];
    }
};
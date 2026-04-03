#include "Vector.h"
#include "Matrix.h"

const Vector3 Vector3::UP = {0, 1, 0};
const Vector3 Vector3::RIGHT = {1, 0, 0};
const Vector3 Vector3::FORWARD = {0, 0, 1};
const Vector3 Vector3::ZERO = {0, 0, 0};

const Matrix Matrix::IDENTITY =
    {
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1}
};

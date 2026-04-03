#pragma once

// Vector/matrix math based on raylib

#include <cmath>
#include "imgui.h"

inline float Lerp(float a, float b, float x) {
    return a + (b-a)*x;
}

inline float Clamp(float x, float min, float max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

class Vector3 {
public:
    float x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vector3(float x) : x(x), y(x), z(x) {}

    static const Vector3 UP;
    static const Vector3 RIGHT;
    static const Vector3 FORWARD;
    static const Vector3 ZERO;

    [[nodiscard]] float LengthSquared() const {
        return x*x + y*y + z*z;
    }
    [[nodiscard]] float Length() const {
        return std::sqrt(LengthSquared());
    }
    [[nodiscard]] Vector3 Normalized() const {
        return *this / Length();
    }
    [[nodiscard]] Vector3 Lerp(Vector3 b, float t) const {
        return *this + (b-*this) * x;
    }

    // from raylib
    Vector3 Rotate(Vector3 axis, float angle)
    {
        // Using Euler-Rodrigues Formula
        // Ref.: https://en.wikipedia.org/w/index.php?title=Euler%E2%80%93Rodrigues_formula

        Vector3 result = *this;

        // Vector3Normalize(axis);
        float length = sqrtf(axis.x*axis.x + axis.y*axis.y + axis.z*axis.z);
        if (length == 0.0f) length = 1.0f;
        float ilength = 1.0f/length;
        axis.x *= ilength;
        axis.y *= ilength;
        axis.z *= ilength;

        angle /= 2.0f;
        float a = sinf(angle);
        float b = axis.x*a;
        float c = axis.y*a;
        float d = axis.z*a;
        a = cosf(angle);
        Vector3 w = { b, c, d };

        // Vector3CrossProduct(w, v)
        Vector3 wv = { w.y*z - w.z*y, w.z*x - w.x*z, w.x*y - w.y*x };

        // Vector3CrossProduct(w, wv)
        Vector3 wwv = { w.y*wv.z - w.z*wv.y, w.z*wv.x - w.x*wv.z, w.x*wv.y - w.y*wv.x };

        // Vector3Scale(wv, 2*a)
        a *= 2;
        wv.x *= a;
        wv.y *= a;
        wv.z *= a;

        // Vector3Scale(wwv, 2)
        wwv.x *= 2;
        wwv.y *= 2;
        wwv.z *= 2;

        result.x += wv.x;
        result.y += wv.y;
        result.z += wv.z;

        result.x += wwv.x;
        result.y += wwv.y;
        result.z += wwv.z;

        return result;
    }

    Vector3 operator +(const Vector3& rhs) const {
        return {x+rhs.x, y+rhs.y, z+rhs.z};
    }
    Vector3 operator -(const Vector3& rhs) const {
        return {x-rhs.x, y-rhs.y, z-rhs.z};
    }
    Vector3 operator -() const {
        return {-x, -y, -z};
    }
    Vector3 operator *(float scalar) const {
        return {x*scalar, y*scalar, z*scalar};
    }
    Vector3 operator /(float scalar) const {
        return {x/scalar, y/scalar, z/scalar};
    }
    void operator +=(const Vector3& rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
    }
    void operator *=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
    }
    void operator *=(const Vector3& rhs) {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
    }
    void operator /=(float scalar) {
        x /= scalar;
        y /= scalar;
        z /= scalar;
    }
    void operator /=(const Vector3& rhs) {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
    }
    Vector3 operator *(const Vector3& rhs) const {
        return {x*rhs.x, y*rhs.y, z*rhs.z};
    }
    Vector3 operator /(const Vector3& rhs) const {
        return {x/rhs.x, y/rhs.y, z/rhs.z};
    }
};

class Vector4 {
public:
    float x, y, z, w;

    Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    Vector4(Vector3 vec) : x(vec.x), y(vec.y), z(vec.z), w(0.0f) {}

    Vector4& operator=(const Vector3& vec) {
        x = vec.x;
        y = vec.y;
        z = vec.z;
        return *this;
    }

    operator Vector3() const {
        return {x, y, z};
    }
};

class Vector2 {
public:
    float x, y;

    Vector2() : x(0), y(0) {}
    Vector2(float x, float y) : x(x), y(y) {}
    Vector2(float x) : x(x), y(x) {}
    constexpr Vector2(ImVec2 vec) : x(vec.x), y(vec.y) {}

    float LengthSquared() const {
        return x*x + y*y;
    }
    float Length() const {
        return std::sqrt(LengthSquared());
    }
    Vector2 Normalized() const {
        return *this / Length();
    }

    Vector2 operator +(const Vector2& rhs) const {
        return {x+rhs.x, y+rhs.y};
    }
    Vector2 operator -(const Vector2& rhs) const {
        return {x-rhs.x, y-rhs.y};
    }
    Vector2 operator -() const {
        return {-x, -y};
    }
    Vector2 operator *(float scalar) const {
        return {x*scalar, y*scalar};
    }
    Vector2 operator /(float scalar) const {
        return {x/scalar, y/scalar};
    }
    void operator +=(const Vector2& rhs) {
        x += rhs.x;
        y += rhs.y;
    }
    void operator *=(float scalar) {
        x *= scalar;
        y *= scalar;
    }
    void operator *=(const Vector2& rhs) {
        x *= rhs.x;
        y *= rhs.y;
    }
    void operator /=(float scalar) {
        x /= scalar;
        y /= scalar;
    }
    void operator /=(const Vector2& rhs) {
        x /= rhs.x;
        y /= rhs.y;
    }
    Vector2 operator *(const Vector2& rhs) const {
        return {x*rhs.x, y*rhs.y};
    }
    Vector2 operator /(const Vector2& rhs) const {
        return {x/rhs.x, y/rhs.y};
    }

    operator Vector3() const {
        return {x, y, 0};
    }
    constexpr operator ImVec2() const {
        return {x, y};
    }
};

class Color {
public:
    float r, g, b, a;

    Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
    Color(Vector3 col) : r(col.x), g(col.y), b(col.z), a(1.0f) {}
    Color(float r, float g, float b) : r(r), g(g), b(b), a(1.0f) {}
    Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
    Color(uint8_t r, uint8_t g, uint8_t b) : r(r/255.0f), g(g/255.0f), b(b/255.0f), a(1.0f) {}
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r/255.0f), g(g/255.0f), b(b/255.0f), a(a/255.0f) {}
    constexpr Color(ImVec4 vec) : r(vec.x), g(vec.y), b(vec.z), a(vec.w) {}

    constexpr operator ImVec4() const {
        return {r, g, b, a};
    }
};
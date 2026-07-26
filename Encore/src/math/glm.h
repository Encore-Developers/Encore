#pragma once
// Standard set of includes for using glm
// Prefer to include this file when working with glm types instead of the individual ones
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_XYZW_ONLY
#define GLM_FORCE_INTRINSICS

#include <format>
#include <iostream>
#include <sstream>

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/ext/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale
#include "glm/ext/matrix_clip_space.hpp" // glm::perspective
#include "SDL3/SDL_pixels.h"
#include "glm/gtx/hash.hpp"
#include "glm/gtx/rotate_vector.hpp"


typedef glm::vec2 Vector2;
typedef glm::dvec2 Vector2l;
typedef glm::i32vec2 Vector2i;
typedef glm::u32vec2 Vector2u;
typedef glm::vec3 Vector3;
typedef glm::i32vec3 Vector3i;
typedef glm::vec4 Vector4;
typedef glm::mat4 Matrix;
typedef Vector4 Color;

template<>
struct std::formatter<Vector3> : std::formatter<std::string> {
    auto format(Vector3 p, format_context& ctx) const
    {
        return formatter<std::string>::format(std::format("<{}, {}, {}>", p.x, p.y, p.z), ctx);
    }
};

template<>
struct std::formatter<Vector3i> : std::formatter<std::string> {
    auto format(Vector3i p, format_context& ctx) const
    {
        return formatter<std::string>::format(std::format("<{}, {}, {}>", p.x, p.y, p.z), ctx);
    }
};

inline SDL_FColor ColorToSDLColor(const Color& color) {
    return {color.x, color.y, color.z, color.w};
}
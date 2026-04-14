#pragma once
#ifdef GLM_OPS_MAIN
#undef GLM_OPS_MAIN

// Custom operators and typedefs for glm stuff

typedef glm::vec2 Vector2;
typedef glm::vec3 Vector3;
typedef glm::vec4 Vector4;
typedef glm::mat4 Matrix;
typedef Vector4 Color;

#include "imgui.h"
#else
#error "Do not include glm_ops.h, include math/glm.h instead"
#endif
// Standard set of includes for using glm
// Prefer to include this file when working with glm types instead of the individual ones
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_XYZW_ONLY

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/ext/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale
#include "glm/ext/matrix_clip_space.hpp" // glm::perspective

// Guard to ensure that glm_ops.h can only be included here
#define GLM_OPS_MAIN
#include "math/glm_ops.h"


#include "camera.h"

#include "glm/gtc/matrix_inverse.hpp"
#include "math/glm.h"

Matrix Camera::getMatrix() {
    auto proj = glm::perspective(glm::radians(fovy), 16.0f/9.0f, 0.1f, 200.0f);

    return proj * glm::lookAt(position, target, {0, 1, 0});
}
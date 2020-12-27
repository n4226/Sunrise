#include "srpch.h"
#include "Camera.h"

namespace sunrise {

    glm::mat4 Camera::projection(float width, float height)
    {
        auto proj = glm::perspectiveFovLH(glm::radians(fov), width, height, zNear, zFar);
        proj[1][1] *= -1;
        return proj;
    }

    glm::mat4 Camera::view()
    {
        return glm::inverse(transform.matrix());
    }

    glm::mat4 Camera::viewProjection(float width, float height)
    {
        return projection(width, height) * view();
    }

}
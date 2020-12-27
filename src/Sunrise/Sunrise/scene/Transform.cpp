#include "Transform.h"
#include "pch.h"

glm::mat4 Transform::matrix()
{

    auto mat = glm::translate(glm::mat4(1.f), position);

    mat *= glm::toMat4(rotation);

    mat = glm::scale(mat, scale);

    return mat;
}


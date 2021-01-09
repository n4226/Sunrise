#include "srpch.h"
#include "Transform.h"


namespace sunrise {

    glm::mat4 Transform::matrix()
    {

        auto mat = glm::translate(glm::mat4(1.f), position);

        mat *= glm::toMat4(rotation);

        mat = glm::scale(mat, scale);

        return mat;
    }

}
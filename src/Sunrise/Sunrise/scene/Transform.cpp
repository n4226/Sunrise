#include "srpch.h"
#include "Transform.h"


namespace sunrise {

    glm::mat4 Transform::matrix() const
    {
        auto mat = glm::translate(glm::mat4(1.f), position);

        mat *= glm::toMat4(rotation);

        mat = glm::scale(mat, scale);

        return mat;
    }

	void Transform::reset()
	{
        position = { 0,0,0 };
        scale = { 1,1,1 };
        rotation = glm::qua<glm::float32>({ 0,0,0 });
	}

}
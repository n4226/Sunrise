#pragma once

#include "srpch.h"

namespace sunrise {


	struct Transform
	{
	public:

		glm::vec3 position = { 0,0,0 };
		glm::vec3 scale = { 1,1,1 };
		glm::qua<glm::float32> rotation = glm::qua<glm::float32>({ 0,0,0 });

		glm::mat4 matrix();

		//void rotate(float angle, glm::uvec3 axis);

	};


}
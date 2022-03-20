#pragma once

#include "srpch.h"

namespace sunrise {

	//TODO: find bvetter place
	//TODO: allow differenct sub mesh materials
	struct MeshRenderer {
		uint32_t material;
	};

	struct Transform
	{
	public:

		glm::vec3 position = { 0,0,0 };
		glm::vec3 scale = { 1,1,1 };
		glm::qua<glm::float32> rotation = glm::qua<glm::float32>({ 0,0,0 });

		void reset();
		glm::mat4 matrix() const;

		//void rotate(float angle, glm::uvec3 axis);

	};


}
#pragma once


#include "srpch.h"
#include "Sunrise/core/Window.h"
#include "SceneRenderCoordinator.h"

namespace sunrise {

	class WorldUniformCreator
	{
	public:
		static void updateSceneUniformBuffer(Window& window, glm::dvec3 sunLLA, glm::dvec3 earthCenter, double earchRadius, std::vector<std::vector<gfx::Buffer*>>& uniformBuffers);

		static void createUniforms(gfx::Renderer* renderer, std::vector<std::vector<gfx::Buffer*>>& uniformBuffers);
	protected:
	private:
	};

}
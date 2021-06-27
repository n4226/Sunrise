#pragma once

#include "srpch.h"
#include "Sunrise/Sunrise/graphics/vulkan/GPU Stages/GPURenderStage.h"
#include "Sunrise/Sunrise/math/mesh/Mesh.h"
#include "Sunrise/Sunrise/graphics/vulkan/resources/MeshBuffers.h"

namespace sunrise {

	class DeferredStage : public gfx::GPURenderStage
	{
	public:
		DeferredStage(Application& app);


		void setup() override;
		void cleanup() override;

		vk::CommandBuffer* encode(RunOptions options) override;

	protected:

		Basic2DMesh* square{};
		gfx::Basic2DMeshBuffer* meshBuff{};

	};

}
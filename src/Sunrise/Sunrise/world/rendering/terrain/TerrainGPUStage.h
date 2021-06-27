#pragma once

#include "srpch.h"
#include "Sunrise/Sunrise/graphics/vulkan/GPU Stages/GPURenderStage.h"

namespace sunrise {

	class TerrainGPUStage: public gfx::GPURenderStage
	{
	public:
		TerrainGPUStage(Application& app);
		

		void setup() override;
		void cleanup() override;

		vk::CommandBuffer* encode(RunOptions options) override;

	protected:



	};

}

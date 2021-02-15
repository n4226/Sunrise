#pragma once

#include "srpch.h"

#include "Sunrise/Sunrise/graphics/vulkan/generalAbstractions/Buffer.h"
#include "Sunrise/Sunrise/graphics/vulkan/resources/MeshBuffers.h"
#include "Sunrise/Sunrise/graphics/vulkan/renderPipelines/GraphicsPipeline.h"

namespace sunrise {

	class Application;

	namespace gfx {

		class GPUGenCommandsPipeline {
			public:
				GPUGenCommandsPipeline(Application& app, vk::Device device, GraphicsPipeline& terrainPipeline);


				VkIndirectCommandsLayoutNV commandsLayout;

				Buffer* commandsBuffer;

				void exicuteIndirectCommands(vk::CommandBuffer cmdBuff,uint32_t drawCount, BindlessMeshBuffer* meshBuff);
		private: 
			vk::DispatchLoaderDynamic dldid;
			vk::Device device;
			Application& app;
			GraphicsPipeline& terrainPipeline;
		};

	}

}
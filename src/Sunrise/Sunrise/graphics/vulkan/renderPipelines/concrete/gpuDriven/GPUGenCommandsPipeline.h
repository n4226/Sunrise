#pragma once

#include "srpch.h"

#include "Sunrise/Sunrise/graphics/vulkan/generalAbstractions/Buffer.h"
#include "Sunrise/Sunrise/graphics/vulkan/resources/MeshBuffers.h"
#include "Sunrise/Sunrise/graphics/vulkan/renderPipelines/GraphicsPipeline.h"
#include "Sunrise/Sunrise/graphics/vulkan/renderPipelines/ComputePipeline.h"

namespace sunrise {

	class Application;

	namespace gfx {

		class SUNRISE_API GPUGenCommandsPipeline: ComputePipeline {
			public:
				GPUGenCommandsPipeline(Application& app, vk::Device device, GraphicsPipeline& terrainPipeline);


				VkIndirectCommandsLayoutNV commandsLayout;

				Buffer* commandsBuffer;

				void exicuteIndirectCommands(vk::CommandBuffer cmdBuff,uint32_t drawCount, BindlessMeshBuffer* meshBuff);
		private: 

			// setup functions
			void setup();

			void createCommandsLayout();
			void getMemRequirements();

			vk::DispatchLoaderDynamic dldid;
			vk::Device device;
			Application& app;
			GraphicsPipeline& terrainPipeline;

			// function pointers
			PFN_vkCreateIndirectCommandsLayoutNV pfn_vkCreateIndirectCommandsLayoutNV;
			PFN_vkGetGeneratedCommandsMemoryRequirementsNV pfn_vkGetGeneratedCommandsMemoryRequirementsNV;
		};

	}

}
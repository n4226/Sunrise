#pragma once

#include "srpch.h"

#include "Sunrise/Sunrise/graphics/vulkan/generalAbstractions/Buffer.h"
#include "Sunrise/Sunrise/graphics/vulkan/renderPipelines/GraphicsPipeline.h"

namespace sunrise {

	class Application;

	namespace gfx {

		class GPUGenCommandsPipeline {
			public:
				GPUGenCommandsPipeline(Application& app, vk::Device device, GraphicsPipeline& terrainPipeline);


				VkIndirectCommandsLayoutNV commandsLayout;

				Buffer* commandsBuffer;

				void exicuteIndirectCommands();

		};

	}

}
#pragma once

#include "srpch.h"
#include "../RenderPassManager.h"
//#include <GLFW/glfw3.h>
//#include "../../dataObjects/Mesh.h"

namespace sunrise::gfx {

	class ComputePipeline;

	class SUNRISE_API GraphicsPipeline
	{
	public:

		GraphicsPipeline(vk::Device device, vk::Extent2D swapChainExtent, RenderPassManager& renderPassManager);
		~GraphicsPipeline();

		// pipeline properties

		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
		VkPipelineLayout pipelineLayout;

		vk::Pipeline vkItem;

		RenderPassManager& renderPassManager;

		virtual void createPipeline() = 0;

		static vk::PipelineShaderStageCreateInfo createShaderStageInfo(vk::Device device, const std::vector<char>& code, vk::ShaderStageFlagBits stage);

		static vk::ShaderModule createShaderModule(vk::Device device, const std::vector<char>& code);

		static std::vector<char> readFile(const std::string& filename);

	protected:


		vk::Device device;

		vk::Extent2D swapChainExtent;

		friend ComputePipeline;
	};

}
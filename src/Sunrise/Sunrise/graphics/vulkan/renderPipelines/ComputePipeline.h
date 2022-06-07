#pragma once

#include "srpch.h"
#include "../generalAbstractions/VkAbstractions.h"

namespace sunrise::gfx {

	
	struct ComputePipelineOptions {
		
		std::vector<DescriptorSetLayout::CreateOptions> descriptorSetLayouts;

		std::string shaderPath;


		//TODO add suportfor multiple push ranges
		bool enablePushConstants = false;
		VkDeviceSize pushConstantOffset = 0;
		VkDeviceSize pushConstantSize = 0;
		vk::ShaderStageFlags pushConstantStages;

	};

	class ComputePipeline
	{

	public:
		ComputePipeline(vk::Device device, std::vector<vk::DescriptorSetLayout>&& descriptorSetLayouts, const std::string& shaderFilePath);


		//std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
		vk::PipelineLayout pipelineLayout;


		vk::Pipeline pipeline;
	};

}
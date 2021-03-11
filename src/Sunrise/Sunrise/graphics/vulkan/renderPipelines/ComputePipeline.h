#pragma once

#include "srpch.h"
#include "../generalAbstractions/VkAbstractions.h"

namespace sunrise::gfx {


	class ComputePipeline
	{

	public:
		ComputePipeline(vk::Device device, std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts, const std::string& shaderFilePath);


		//std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
		vk::PipelineLayout pipelineLayout;


		vk::Pipeline pipeline;
	};

}
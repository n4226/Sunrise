#include "srpch.h"
#include "DeferredPipeline.h"

#include "../../../../../math/mesh/Mesh.h"

namespace sunrise {

	DeferredPipeline* deferredPipeline = new DeferredPipeline();

	gfx::GraphicsPipelineOptions DeferredPipeline::makeDeff() {
		PROFILE_FUNCTION;

		auto pipeOptions = sunrise::gfx::GraphicsPipelineOptions();

		auto layout = gfx::DescriptorSetLayoutBinding::createWholeSet(
			{
				{ vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment },
				{ vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment },
				{ vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment },
			}
		);

		pipeOptions.descriptorSetLayouts = { { layout } }; //DescriptorSetLayout::Create();

		pipeOptions.shaderStages = {
			{ "shaders/GBuffer.vert.spv",vk::ShaderStageFlagBits::eVertex },
			{ "shaders/GBuffer.frag.spv",vk::ShaderStageFlagBits::eFragment }
		};

		//pipeOptions.bindingDescriptions = { makeVertBinding(0, sizeof(glm::vec2)) };
		//pipeOptions.attributeDescriptions = { makeVertAttribute(0, 0, VertexAttributeFormat::vec2, 0) };

		auto meshBindingDes = Basic2DMesh::getBindingDescription();
		pipeOptions.bindingDescriptions = std::vector(meshBindingDes.begin(), meshBindingDes.end());
		auto meshAttributeDes = Basic2DMesh::getAttributeDescriptions();
		pipeOptions.attributeDescriptions = std::vector(meshAttributeDes.begin(), meshAttributeDes.end());

		return pipeOptions;
	}

}
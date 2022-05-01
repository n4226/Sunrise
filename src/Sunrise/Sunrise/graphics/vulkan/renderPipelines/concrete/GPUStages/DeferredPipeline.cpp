#include "srpch.h"
#include "DeferredPipeline.h"

#include "../../../../../math/mesh/Mesh.h"
#include "../../../renderer/DebugDrawer.h"

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
				{ vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment },
				
				// post preprocessing uniforms -- TEMP TODO: fix need - need to declare eVertex too because of debug draw lines pipeline in order to use same descritpors
				{ vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex},
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

		pipeOptions.enablePushConstants = true;
		pipeOptions.pushConstantOffset = 0;
		pipeOptions.pushConstantSize = sizeof(uint32_t);
		pipeOptions.pushConstantStages = vk::ShaderStageFlagBits::eVertex;


		return pipeOptions;
	}


	DebugLineDrawPipeline* debugLineDrawPipeline = new DebugLineDrawPipeline();


	sunrise::gfx::GraphicsPipelineOptions DebugLineDrawPipeline::makeDeff()
	{

		auto pipeOptions = sunrise::gfx::GraphicsPipelineOptions();

	/*	auto layout = gfx::DescriptorSetLayoutBinding::createWholeSet(
			{
				{ vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex },
			}
		);*/

		auto layout = gfx::DescriptorSetLayoutBinding::createWholeSet(
			{
				{ vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment },
				{ vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment },
				{ vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment },
				{ vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment },

				// post preprocessing uniforms -- has to have extra shaderflag bits
				{ vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment},
			}
		);

		pipeOptions.descriptorSetLayouts = { { layout } }; //DescriptorSetLayout::Create();

		pipeOptions.shaderStages = {
			{ "shaders/debugLine.vert.spv",vk::ShaderStageFlagBits::eVertex },
			{ "shaders/debugLine.frag.spv",vk::ShaderStageFlagBits::eFragment }
		};

		//pipeOptions.bindingDescriptions = { makeVertBinding(0, sizeof(glm::vec2)) };
		//pipeOptions.attributeDescriptions = { makeVertAttribute(0, 0, VertexAttributeFormat::vec2, 0) };

		auto meshBindingDes = gfx::DebugDrawer::LineData::getBindingDescription();
		pipeOptions.bindingDescriptions = std::vector(meshBindingDes.begin(), meshBindingDes.end());
		auto meshAttributeDes = gfx::DebugDrawer::LineData::getAttributeDescriptions();
		pipeOptions.attributeDescriptions = std::vector(meshAttributeDes.begin(), meshAttributeDes.end());

		//used for line color
		pipeOptions.enablePushConstants = true;
		pipeOptions.pushConstantOffset = 0;
		pipeOptions.pushConstantSize = sizeof(glm::vec4);
		pipeOptions.pushConstantStages = vk::ShaderStageFlagBits::eFragment;

		pipeOptions.primitiveType = vk::PrimitiveTopology::eLineStrip;
		pipeOptions.lineWidth = 10;

		//lines do not have a model matrix
		return pipeOptions;
	}

}
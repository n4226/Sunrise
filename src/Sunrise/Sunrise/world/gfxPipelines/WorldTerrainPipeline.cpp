#include "srpch.h"
#include "WorldTerrainPipeline.h"

#include "../../math/mesh/Mesh.h"
#include "../../graphics/vulkan/resources/uniforms.h"

namespace sunrise {

	WorldTerrainPipeline* worldTerrainPipeline = new WorldTerrainPipeline();

	gfx::GraphicsPipelineOptions sunrise::WorldTerrainPipeline::makeDeff()
	{
		PROFILE_FUNCTION;
        
		using namespace gfx;
		auto pipeOptions = sunrise::gfx::GraphicsPipelineOptions();

		// bindfings should be ordered from least update frequency (binding 0) to most updated
		  // global uniforms - 
		// set = 0, binding = 0 -> viewProj
		// set = 0, binding = 1 -> modelMat - per opbject/instance
		// set = 0, binding = 2 -> materialConstants - each draw call gets an index into this which has constants and texture indicies for that mat 
		// set = 0, binding = 3 -> materialImages - mat constant has a base index into this descriptor arrey which for now will have 5 contiguous indices for each texture 
		std::vector<DescriptorSetLayoutBinding> set = {
			{
				DescriptorSetLayoutBinding(0,vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex),
				DescriptorSetLayoutBinding(1,vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment),
				DescriptorSetLayoutBinding(2,vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eFragment),
				DescriptorSetLayoutBinding(
					3,vk::DescriptorType::eCombinedImageSampler, maxMaterialTextureDescriptorArrayCount,
					vk::ShaderStageFlagBits::eFragment,nullptr,
					vk::DescriptorBindingFlagBits::eVariableDescriptorCount | vk::DescriptorBindingFlagBits::ePartiallyBound
					| vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending
					),
			}
		};

		pipeOptions.descriptorSetLayouts = { {set} };

		pipeOptions.shaderStages = {
			{ "shaders/terrain.vert.spv",vk::ShaderStageFlagBits::eVertex },
			{ "shaders/terrain.frag.spv",vk::ShaderStageFlagBits::eFragment }
		};

		auto meshBindingDes = Mesh::getBindingDescription();
		pipeOptions.bindingDescriptions = std::vector(meshBindingDes.begin(), meshBindingDes.end());
		auto meshAttributeDes = Mesh::getAttributeDescriptions();
		pipeOptions.attributeDescriptions = std::vector(meshAttributeDes.begin(), meshAttributeDes.end());

		// push constants
		pipeOptions.enablePushConstants = true;
		pipeOptions.pushConstantOffset = 0;
		pipeOptions.pushConstantSize = sizeof(DrawPushData);
		pipeOptions.pushConstantStages = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;

		//TODO: set up depth

		return pipeOptions;
	}

}

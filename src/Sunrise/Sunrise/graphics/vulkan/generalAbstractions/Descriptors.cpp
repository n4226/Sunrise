#include "srpch.h"
#include "Descriptors.h"



sunrise::gfx::DescriptorSetLayoutBinding::DescriptorSetLayoutBinding(uint32_t bindingIndex, vk::DescriptorType descriptorType, vk::ShaderStageFlags stagesUsedIn)
	: DescriptorSetLayoutBinding(bindingIndex, descriptorType, 1, stagesUsedIn, nullptr, {})
{
}

sunrise::gfx::DescriptorSetLayoutBinding::DescriptorSetLayoutBinding(
	uint32_t bindingIndex, vk::DescriptorType descriptorType,
	uint32_t numberOfDescriptorsInArray, vk::ShaderStageFlags stagesUsedIn,
	const vk::Sampler* samplers, vk::DescriptorBindingFlags flags)
{
	vkItem.binding = bindingIndex;
	vkItem.descriptorType = VkDescriptorType(descriptorType);
	vkItem.descriptorCount = numberOfDescriptorsInArray;
	vkItem.stageFlags = VkShaderStageFlags(stagesUsedIn);

	// samplers not implimented yet
	assert(samplers == nullptr);
	//vkItem.pImmutableSamplers = samplers;

	auto c_flags = VkDescriptorBindingFlags(flags);

	if (c_flags != 0) {
		this->flags = new VkDescriptorBindingFlags(c_flags);
	}
}

sunrise::gfx::DescriptorSetLayoutBinding::~DescriptorSetLayoutBinding()
{
	delete flags;
}


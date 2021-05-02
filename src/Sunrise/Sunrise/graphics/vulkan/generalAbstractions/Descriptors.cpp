#include "srpch.h"
#include "Descriptors.h"

namespace sunrise::gfx {

	DescriptorSetLayoutBinding::DescriptorSetLayoutBinding(uint32_t bindingIndex, vk::DescriptorType descriptorType, vk::ShaderStageFlags stagesUsedIn)
		: DescriptorSetLayoutBinding(bindingIndex, descriptorType, 1, stagesUsedIn, nullptr, {})
	{
	}

	DescriptorSetLayoutBinding::DescriptorSetLayoutBinding(
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


	/// <summary>
	/// a simple way to create all the set layout bindings at once
	/// </summary>
	/// <returns></returns>

	inline std::vector<DescriptorSetLayoutBinding> DescriptorSetLayoutBinding::createWholeSet(std::vector<shell>&& bindings) {

		std::vector<DescriptorSetLayoutBinding> result(bindings.size());

		for (size_t i = 0; i < result.size(); i++)
		{
			result[i] = DescriptorSetLayoutBinding(i, bindings[i].descriptorType, bindings[i].numberOfDescriptorsInArray, bindings[i].stagesUsedIn);
		}

		return result;
	}

	DescriptorSetLayoutBinding::~DescriptorSetLayoutBinding()
	{
		delete flags;
	}


}
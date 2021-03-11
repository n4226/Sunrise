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

//std::vector<DescriptorSetLayoutBinding> sunrise::gfx::DescriptorSetLayoutBinding::createWholeSet(std::vector<shell>&& bindings)
//{
//	//std::vector< DescriptorSetLayoutBinding> result = {bindings.size()};
//
//}

vk::DescriptorSetLayout sunrise::gfx::DescriptorSetLayout::Create(CreateOptions options, vk::Device device)
{
	auto bindings = std::vector<VkDescriptorSetLayoutBinding>(options.setLayoutBindings.size());
	auto bindingFlags = std::vector<VkDescriptorBindingFlags>(options.setLayoutBindings.size());
	VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo;

	bool needsFlags = false;

	for (size_t i = 0; i < options.setLayoutBindings.size(); i++)
	{
		auto& binding = options.setLayoutBindings[i];
		bindings[i] = binding.vkItem;
		if (binding.flags != nullptr) {
			needsFlags = true;
			bindingFlags[i] = *binding.flags;
		}
		else if (needsFlags)
			bindingFlags[i] = {};
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = bindings.size();
	layoutInfo.pBindings = bindings.data();

	if (needsFlags) {
		bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		bindingFlagsInfo.pNext = nullptr;
		bindingFlagsInfo.bindingCount = bindingFlags.size();
		bindingFlagsInfo.pBindingFlags = bindingFlags.data();

		layoutInfo.pNext = &bindingFlagsInfo;
	}
	else {
		layoutInfo.pNext = nullptr;
	}

	// create layout
	return device.createDescriptorSetLayout(layoutInfo);
}

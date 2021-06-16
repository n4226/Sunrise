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



	DescriptorPool::DescriptorPool(vk::Device device,CreateOptions&& options)
		: device(device)
	{
		std::vector<vk::DescriptorPoolSize> poolSizes;
		poolSizes.reserve(options.typeSizes.size());

		for (auto size : options.typeSizes) {
			vk::DescriptorPoolSize poolSize{};
			poolSize.type = size.type;
			// not sure if this means array count - it might be array count 
			poolSize.descriptorCount = size.maxNum;

			poolSizes.push_back(poolSize);
		}

		vk::DescriptorPoolCreateInfo poolInfo{};
		poolInfo.poolSizeCount = poolSizes.size();
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.flags = options.flags;

		// there should be a set per swap image per physical window
		poolInfo.maxSets = options.maxSets;

		vkItem = device.createDescriptorPool(poolInfo);
	}

	void DescriptorPool::reset()
	{
		device.resetDescriptorPool(vkItem);
	}

	void DescriptorPool::free(std::vector<DescriptorSet>&& sets)
	{
		device.freeDescriptorSets(vkItem, sets);
	}

	std::vector<DescriptorSet> DescriptorPool::allocate(std::vector<vk::DescriptorSetLayout>&& layouts)
	{
		//std::vector<DescriptorSet*> sets;

		vk::DescriptorSetAllocateInfo allocInfo;

		allocInfo.descriptorPool = vkItem;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
		allocInfo.pSetLayouts = layouts.data();

		return device.allocateDescriptorSets(allocInfo);
	}

	//NOTE: this should be very efficent as it is in main render path
	void DescriptorPool::update(std::vector<UpdateOperation>&& ops)
	{

		//for (auto op : ops) {

		//	if (op.type == UpdateOperation::Type::write) {



		//		VkWriteDescriptorSet writeUpdate{};
		//		writeUpdate.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//		writeUpdate.dstSet = op.dstBinding.set;
		//		writeUpdate.dstBinding = op.dstBinding.inex;
		//		writeUpdate.dstArrayElement = op.dstStartArrayElement;

		//		writeUpdate.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		//		writeUpdate.descriptorCount = 1;

		//		writeUpdate.pBufferInfo = &globalUniformBufferInfo;
		//		writeUpdate.pImageInfo = nullptr; // Optional
		//		writeUpdate.pTexelBufferView = nullptr; // Optional

		//	}
		//	else {

		//	}
		//}

	}



}
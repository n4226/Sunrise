#include "srpch.h"
#include "Descriptors.h"

namespace sunrise::gfx {

	//global storage
	std::unordered_map<VkDescriptorSetLayout, std::vector<DescriptorSetLayoutBinding>*> layoutData;

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
		this->flags = c_flags;
		
	}


	/// <summary>
	/// a simple way to create all the set layout bindings at once
	/// </summary>
	/// <returns></returns>

    std::vector<DescriptorSetLayoutBinding> DescriptorSetLayoutBinding::createWholeSet(std::vector<Shell>&& bindings) {

		std::vector<DescriptorSetLayoutBinding> result(bindings.size());

		for (size_t i = 0; i < result.size(); i++)
		{
			result[i] = DescriptorSetLayoutBinding(i, bindings[i].descriptorType, bindings[i].numberOfDescriptorsInArray, bindings[i].stagesUsedIn);
		}

		return result;
	}

	DescriptorSetLayoutBinding::~DescriptorSetLayoutBinding()
	{
	}


	//vk::DescriptorSetLayout DescriptorSetLayout::Create(CreateOptions options, vk::Device device) {
	//	return Create(std::move(options), device);
	//}

	vk::DescriptorSetLayout DescriptorSetLayout::Create(CreateOptions options, vk::Device device)
	{
		auto bindings = std::vector<VkDescriptorSetLayoutBinding>(options.setLayoutBindings.size());
		auto bindingFlags = std::vector<VkDescriptorBindingFlags>(options.setLayoutBindings.size());
		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo;

		bool needsFlags = false;

		for (size_t i = 0; i < options.setLayoutBindings.size(); i++)
		{
			auto& binding = options.setLayoutBindings[i];
			bindings[i] = binding.vkItem;
			if (binding.flags != VkDescriptorBindingFlags()) {
				needsFlags = true;
				bindingFlags[i] = binding.flags;
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
		auto layout = device.createDescriptorSetLayout(layoutInfo);
		auto c_layout = static_cast<VkDescriptorSetLayout>(layout);
		layoutData[c_layout] = new std::vector<DescriptorSetLayoutBinding>(options.setLayoutBindings);

		return layout;
	}

	void DescriptorSetLayout::Destroy(vk::DescriptorSetLayout layout, vk::Device device)
	{
		auto c_layout = static_cast<VkDescriptorSetLayout>(layout);
		delete layoutData[c_layout];
		layoutData.erase(c_layout);

		device.destroyDescriptorSetLayout(layout);
	}

	DescriptorPool::DescriptorPool(vk::Device device,CreateOptions&& options)
		: device(device)
	{
        std::vector<vk::DescriptorPoolSize> poolSizes{};
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
        poolInfo.pNext = nullptr;

        
        
		vkItem = device.createDescriptorPool(poolInfo);
	}

	void DescriptorPool::reset()
	{
		device.resetDescriptorPool(vkItem);
	}

	void DescriptorPool::free(std::vector<DescriptorSet*>&& sets)
	{
		std::vector<vk::DescriptorSet> rawSets{};
		rawSets.reserve(sets.size());

		for (auto set : sets) {
			rawSets.push_back(set->vkItem);
		}

		device.freeDescriptorSets(vkItem, rawSets);
	}

	std::vector<DescriptorSet*> DescriptorPool::allocate(const std::vector<vk::DescriptorSetLayout>& layouts, const std::vector<uint32_t>& varibleArrayLengths)
	{
		std::vector<DescriptorSet*> sets;

		vk::DescriptorSetAllocateInfo allocInfo;

		allocInfo.descriptorPool = vkItem;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
		allocInfo.pSetLayouts = layouts.data();

        
        //for descriptor instancing varible array
        vk::DescriptorSetVariableDescriptorCountAllocateInfo varibleInfo{};
        varibleInfo.pNext = nullptr;
        varibleInfo.descriptorSetCount = varibleArrayLengths.size();
        varibleInfo.pDescriptorCounts = varibleArrayLengths.data();
        
        allocInfo.pNext = &varibleInfo;
        
		//TODO: assuming one to one relationship betwseen layout array and returned sets in terms of index - docs are not clear about this
		auto rawSets = device.allocateDescriptorSets(allocInfo);

		sets.reserve(rawSets.size());
		for (size_t i = 0; i < rawSets.size(); i++) {
			sets.push_back(new DescriptorSet(rawSets[i],layoutData[layouts[i]]));
		}

        
        
        
		return sets;
	}


	//NOTE: this should be very efficent as it is in main render path
	void DescriptorPool::update(std::vector<UpdateOperation>&& ops)
	{
		std::vector<vk::WriteDescriptorSet> writes;
		std::vector<vk::CopyDescriptorSet> copies;

		writes.reserve(ops.size());
		copies.reserve(ops.size());

		for (auto& op : ops) {

			if (op.type == UpdateOperation::Type::write) {

				vk::WriteDescriptorSet writeUpdate{};


				writeUpdate.dstSet = op.dstBinding.set->vkItem;
				writeUpdate.dstBinding = op.dstBinding.index;
				writeUpdate.dstArrayElement = op.dstStartArrayElement;


				auto layoutbindingOptions = (*op.dstBinding.set->layout)[op.dstBinding.index];

				writeUpdate.descriptorType = vk::DescriptorType(layoutbindingOptions.vkItem.descriptorType);
				writeUpdate.descriptorCount = op.discriptorCountToUpdate;

				switch (op.reference.index())
				{
				case 1:

					writeUpdate.pBufferInfo = &std::get<UpdateOperation::DescriptorBufferRef>(op.reference);
					break;
				case 2:
					writeUpdate.pImageInfo = &std::get<UpdateOperation::DescriptorImageRef>(op.reference); 
					break;
				case 3:
					//TODO case 3 not implimented yet
					//writeUpdate.pTexelBufferView = nullptr; 
					break;

				default:
					// error
					SR_CORE_ERROR("attempting to update a descriptor but no ref provided");
					//todo throw error here
					SR_CORE_ERROR("add throw error here");

					break;
				}

				writes.push_back(writeUpdate);
			}
			else {

				vk::CopyDescriptorSet copyUpdate{};

				copyUpdate.dstSet = op.dstBinding.set->vkItem;
				copyUpdate.dstBinding = op.dstBinding.index;
				copyUpdate.dstArrayElement = op.dstStartArrayElement;

				copyUpdate.srcSet = op.srcBinding.set->vkItem;
				copyUpdate.srcBinding = op.srcBinding.index;
				copyUpdate.srcArrayElement = op.srcStartArrayElement;

				copyUpdate.descriptorCount = op.discriptorCountToUpdate;

				copies.push_back(copyUpdate);
			}


		}

		device.updateDescriptorSets(writes, copies);
	}

	DescriptorPool::~DescriptorPool()
	{
		device.destroy(vkItem);
	}


	DescriptorSet::DescriptorSet(vk::DescriptorSet vkItem, std::vector<DescriptorSetLayoutBinding>* layout)
		: vkItem(vkItem), layout(layout)
	{

	}

	DescriptorBinding DescriptorSet::makeBinding(size_t index)
	{
		return { this, index };
	}



}


#pragma once

#include "srpch.h"



namespace sunrise::gfx {

	/*
		Using Descritpors

		1. define a layout
			- define the types descriptor sets and bindings you will and configuration
			 

		2. create the pool which can instanciate layouts

		descriptor parts:

		DescriptorSetLayoutBinding - definition of the type of a binding in a set layout and which stages it will be used in
		DescriptorSetLayout - a collection of DescriptorSetLayoutBindings that define one type of descreiptor set that can be initilized by a descriptorSetPool
	
	*/

	/// <summary>
	/// a wrapper of VkDescriptorSetLayoutBinding
	/// </summary>
	struct SUNRISE_API DescriptorSetLayoutBinding {

		struct shell {
			vk::DescriptorType descriptorType;
			vk::ShaderStageFlags stagesUsedIn;
			uint32_t numberOfDescriptorsInArray = 1;
		};

		/// <summary>
		/// Create a simple non array descriptor
		/// </summary>
		DescriptorSetLayoutBinding
		(uint32_t bindingIndex, vk::DescriptorType descriptorType,vk::ShaderStageFlags stagesUsedIn);
		
		/// <summary>
		/// The full config options for creating a set layout binding
		/// performance: has to loop over samplers
		/// </summary>
		/// <param name="bindingIndex"></param>
		/// <param name="descriptorType"></param>
		/// <param name="numberOfDescriptorsInArray">if not using the descriptor as an array, set this to 1. If creating a varible descriptor count array this should be the maximum descriptors in the array</param>
		/// <param name="stagesUsedIn"></param>
		/// <param name="samplers">if type is a combined image and sampler specify the sampler(s) here. The number of samplers should be the same as the number of decriptors in array parameter.<\param>
		/// <param name="VkDescriptorBindingFlags">optional flags. This is where things like varible length arrays are specified <\param>
		DescriptorSetLayoutBinding
			(uint32_t bindingIndex, vk::DescriptorType descriptorType,
			uint32_t numberOfDescriptorsInArray,vk::ShaderStageFlags stagesUsedIn,
			const vk::Sampler* samplers = nullptr, vk::DescriptorBindingFlags flags = {}
		);



		/// <summary>
		/// a simple way to create all the set layout bindings for one set layout at once
		/// this array can ve fed into the constructor for DescriptorSetLayout to create a layout
		/// </summary>
		/// <returns></returns>
		static std::vector<DescriptorSetLayoutBinding> createWholeSet(
			std::vector<shell>&& bindings
		);


		~DescriptorSetLayoutBinding();

		VkDescriptorSetLayoutBinding vkItem{};

		VkDescriptorBindingFlags* flags = nullptr;
		
		/// <summary>
		/// do not use
		/// </summary>
		DescriptorSetLayoutBinding() = default;
	protected:
		//friend static std::vector<DescriptorSetLayoutBinding> createWholeSet();
	};

	


	/// <summary>
	/// make one for each type of descripor set your would like to create
	/// </summary>
	namespace DescriptorSetLayout {
		struct CreateOptions {
			std::vector<DescriptorSetLayoutBinding>&& setLayoutBindings;
		};

		static vk::DescriptorSetLayout Create(CreateOptions options, vk::Device device) {
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


		//vk::DescriptorSetLayout vkItem;
	};


	class SUNRISE_API DescriptorPool {

	};



	struct SUNRISE_API DescriptorSet {


	};


	struct SUNRISE_API DescriptorBinding {

	};


}





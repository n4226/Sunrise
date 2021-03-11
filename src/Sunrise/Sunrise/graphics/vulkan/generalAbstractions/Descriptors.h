#pragma once

#include "srpch.h"



namespace sunrise::gfx {

	/*
		Using Descritpors

		1. define a layout
			- define the types descriptor sets and bindings you will and configuration

		2. create the 
	
	*/


	struct SUNRISE_API DescriptorSet {
	

	};


	struct SUNRISE_API DescriptorBinding {

	};
	
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
		/// a simple way to create all the set layout bindings at once
		/// </summary>
		/// <returns></returns>
		static std::vector<DescriptorSetLayoutBinding> createWholeSet(
			std::vector<shell>&& bindings
		);

		~DescriptorSetLayoutBinding();

		VkDescriptorSetLayoutBinding vkItem{};

		VkDescriptorBindingFlags* flags = nullptr;
	};

	


	/// <summary>
	/// make one for each type of descripor set your would like to create
	/// </summary>
	namespace DescriptorSetLayout {
		struct CreateOptions {
			std::vector<DescriptorSetLayoutBinding>&& setLayoutBindings;
		};

		static vk::DescriptorSetLayout Create(CreateOptions options, vk::Device device);

		//vk::DescriptorSetLayout vkItem;
	};


	class SUNRISE_API DescriptorPool {

	};



}





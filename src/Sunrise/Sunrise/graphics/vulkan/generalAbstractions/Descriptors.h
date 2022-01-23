#pragma once

#include "srpch.h"



namespace sunrise::gfx {

	/*
		Using Descritpors

		1. define a layout
			- define the types of descriptor sets and bindings you will use and configuration
			 

		2. create the pool which can instanciate layouts

		3.

		descriptor parts:

		DescriptorSetLayoutBinding - definition of the type of a binding in a set layout and which stages it will be used in
		DescriptorSetLayout - a collection of DescriptorSetLayoutBindings that define one type of descreiptor set that can be initilized by a descriptorSetPool
	
		for more info see vulkan docs

		TODO: A note about performance: right now structs like descriptor set layout, descriptor set etc wrap the vk objects which is 
		not got for performance since it requires looping to create and remove them. in the future the pointer could be used as a key in a map
		this would require creatoin and deletion methods to pass through this system so not doing this now but should in the future.

		as for image:

		// be weary of combined image samplers
		//https://www.reddit.com/r/vulkan/comments/4gvmus/best_way_for_textures_in_shaders/ - helpful

	*/

	/// <summary>
	/// a wrapper of VkDescriptorSetLayoutBinding
	/// </summary>
	struct SUNRISE_API DescriptorSetLayoutBinding {

		struct Shell {
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
			std::vector<Shell>&& bindings
		);


		~DescriptorSetLayoutBinding();

		VkDescriptorSetLayoutBinding vkItem{};

		VkDescriptorBindingFlags flags{};

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

		/*class HiddenParams {
		private:
			friend DescriptorPool;
			friend vk::DescriptorSetLayout Create(CreateOptions&& options, vk::Device device);

			static std::unordered_map<vk::DescriptorSetLayout, std::vector<DescriptorSetLayoutBinding>*> layoutInfo;
		};*/

		struct CreateOptions {
			std::vector<DescriptorSetLayoutBinding> setLayoutBindings;
		};

		vk::DescriptorSetLayout Create(CreateOptions options, vk::Device device);
		
		/// <summary>
		/// must have been created with the associated create method
		/// </summary>
		/// <param name=""></param>
		void Destroy(vk::DescriptorSetLayout layout, vk::Device device);

		//vk::DescriptorSetLayout vkItem;
	};

	class DescriptorPool;
	class DescriptorBinding;

	/// <summary>
	/// created by Descriptor pool
	/// </summary>
	//typedef vk::DescriptorSet DescriptorSet;
	struct SUNRISE_API DescriptorSet {
	public:

		vk::DescriptorSet vkItem;
		DescriptorBinding makeBinding(size_t index);

	private:
		friend DescriptorPool;

		
		std::vector<DescriptorSetLayoutBinding>* layout;

		DescriptorSet(vk::DescriptorSet vkItem, std::vector<DescriptorSetLayoutBinding>* layout);


		//void bindInto();

	};


	struct DescriptorBinding {
		DescriptorSet* set;
		size_t index;
	};


	/// <summary>
	/// wrapper of vk::DescriptorPool
	/// </summary>
	class SUNRISE_API DescriptorPool {
	public:

		struct CreateOptions {

			struct DescriptorTypeAllocOptions {
				DescriptorTypeAllocOptions(vk::DescriptorType type, size_t maxNum) {
					this->type = type;
					this->maxNum = maxNum;
				}
				DescriptorTypeAllocOptions() = default;
				DescriptorTypeAllocOptions(vk::DescriptorPoolSize size) {
					type = size.type;
					maxNum = size.descriptorCount;
				}

				vk::DescriptorType type;
				// the total max number of this descriptor allocated - if the pool allocates 2 sets and each one has 2 of this descriptor than thes would have to be 4 in order to allocate both sets
				// not sure if this means array count - I don't think it is
				size_t maxNum;
			};

			/// <summary>
			/// the maximum number of total sets that can be allocated
			/// </summary>
			size_t maxSets;

			/// <summary>
			/// the max number of each type of descriptor binding this set can store.
			/// </summary>
			std::vector<DescriptorTypeAllocOptions> typeSizes;

			/// <summary>
			/// see: https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkDescriptorPoolCreateFlagBits.html
			/// </summary>
			vk::DescriptorPoolCreateFlags flags = {};
		};

		DescriptorPool(vk::Device device, CreateOptions&& options);

		vk::DescriptorPool vkItem;

		// api

		/// <summary>
		/// resets all descriptor sets allocated by this set. those said sets are implicitly freed
		/// </summary>
		void reset();

		/// <summary>
		/// frees all given sets
		/// Asertion: all sets must have been created form this pool
		/// </summary>
		/// <param name="sets"></param>
		// note about error for me tommorow - might have to do with vulkan.hpp vk::descriptor set - it might be templated for descriptor type of something. check if other uses of descroptors use c equivilent and if this theory is correct
		void free(std::vector<DescriptorSet*>&& sets);

		// see: https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkAllocateDescriptorSets.html
		// if allocation is not valid because pool is full or full of a type, this will fail. 
		// also can fail within limits due to fragmentaion of internal storage 

		/// <summary>
		/// allocates one descriptor set for each layout in the input array of layouts
		/// </summary>
		/// <param name="layouts"></param>
        std::vector<DescriptorSet*> allocate(const std::vector<vk::DescriptorSetLayout>& layouts, const std::vector<uint32_t>& varibleArrayLengths = {});

		// api for spacific sets
		
		struct UpdateOperation {
			
			enum class Type {
				write, copy
			};

			typedef vk::DescriptorBufferInfo DescriptorBufferRef;

			/*
				typedef struct VkDescriptorImageInfo {
					VkSampler        sampler;
					VkImageView      imageView;
					VkImageLayout    imageLayout;
				} VkDescriptorImageInfo;
			*/
			typedef vk::DescriptorImageInfo DescriptorImageRef;


			//TODO: not functiuonal yet
			struct DescriptorTexalRef {

			};


			Type type;


			// includes set ptr
			// for update ops this is the binding to update and srcBiding can be ignored
			DescriptorBinding dstBinding;
			size_t dstStartArrayElement;
			/// <summary>
			/// for array this is number of elements to update starting at the dstStartArrayElement. 
			/// this is also the number of image, buffer, or texel buffer views 
			/// </summary>
			size_t discriptorCountToUpdate;


			//write spacific
			// this can be found from the dst binding object
			////vk::DescriptorType type;

			typedef std::variant<std::monostate, DescriptorBufferRef, DescriptorImageRef, DescriptorTexalRef> ReferenceType;
			
			ReferenceType reference;

			//copy spacific
			DescriptorBinding srcBinding{};
			size_t srcStartArrayElement = 0;

		};


		void update(std::vector<UpdateOperation>&& ops);

	private:

		vk::Device device;


	};



}





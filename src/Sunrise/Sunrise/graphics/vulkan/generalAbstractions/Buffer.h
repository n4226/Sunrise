#pragma once

#include "srpch.h"
#include "ResourceHeap.h"

namespace sunrise::gfx {


	enum SUNRISE_API ResourceStorageType
	{
		gpu = VMA_MEMORY_USAGE_GPU_ONLY,
		cpu = VMA_MEMORY_USAGE_CPU_ONLY,
		cpuToGpu = VMA_MEMORY_USAGE_CPU_TO_GPU,
		gpuToCpu = VMA_MEMORY_USAGE_GPU_TO_CPU,
		cpuCopy = VMA_MEMORY_USAGE_CPU_COPY,
		/// <summary>
		/// Not available on non TBDR architecture GPUs
		/// </summary>
		gpuLazy = VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED

	};

	struct SUNRISE_API BufferCreationOptions {
		ResourceStorageType storage;
		vk::BufferUsageFlags usage;
		vk::SharingMode sharingMode;
		std::vector<uint32_t> queueFamilieIndicies;
		uint32_t memoryTypeBits = UINT32_MAX;
	};

	//TODO: impolement native buffer and mem managment support
	struct NativeBufferCreationOptions {
		/*ResourceStorageType storage;
		vk::BufferUsageFlags usage;
		vk::SharingMode sharingMode;
		std::vector<uint32_t> queueFamilieIndicies;*/
		vk::MemoryRequirements2 memRequirments;
	};

	class SUNRISE_API Buffer
	{
	public:
		Buffer(const Buffer& othter) = delete;
		Buffer(vk::Device device, VmaAllocator allocator, VkDeviceSize size, BufferCreationOptions options);
		//Buffer(vk::Device device, NativeBufferCreationOptions options);

		/// <summary>
		/// a staging buffer is created, the thread is blocked while the data is sent to the actual gpu private buffer, than the new [gpu private buffer is returned.
		/// </summary>
		/// <param name="device"></param>
		/// <param name="allocator"></param>
		/// <param name="size"></param>
		/// <param name="data"></param>
		/// <param name="options"></param>
		/// <returns></returns>
		static Buffer* StageAndCreatePrivate(vk::Device device, vk::Queue& queue, vk::CommandPool commandPool, VmaAllocator allocator, VkDeviceSize size, const void* data, BufferCreationOptions options);

		~Buffer();

		void mapMemory();
		void unmapMemory();

		void* mappedData = nullptr;

		/// <summary>
		/// 
		/// </summary>
		/// <param name="data"></param>
		/// <param name="internalOffset"></param>
		/// <param name="size">if left 0 the buffer size will be used</param>
		void tempMapAndWrite(const void* srcData, size_t internalOffset = 0, size_t size = 0, bool mapMemory = true);

		bool getMemoryMapped();
		bool canMapMemory();

		VkBuffer vkItem = nullptr;
		VmaAllocation allocation = nullptr;
		const VkDeviceSize size;

		void gpuCopyToOther(Buffer& destination, vk::Queue& queue, vk::CommandPool commandPool);

		void gpuCopyToOther(Buffer& destination, vk::CommandBuffer& buffer);

		void name(const char* name,const VkDebug& debugObject) const;

	private:
		bool memoryMapped = false;
		VmaAllocator allocator;
		vk::Device device;
	};

}
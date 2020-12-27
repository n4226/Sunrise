#include "srpch.h"
#include "VkHelpers.h"

namespace sunrise::gfx::vkHelpers {


	void allocateCommandBuffers(vk::Device device, vk::CommandPool pool, vk::CommandBuffer* buffers, uint32_t count, vk::CommandBufferLevel level)
	{


		vk::CommandBufferAllocateInfo allocInfo{};
		allocInfo.commandPool = pool;


		allocInfo.level = level;
		allocInfo.commandBufferCount = count;



		//auto result = vkAllocateCommandBuffers(device, &allocInfo, buffers);
		auto result = device.allocateCommandBuffers(&allocInfo, buffers);
	}

	void createPoolsAndCommandBufffers(vk::Device device, std::vector<vk::CommandPool>& pools, std::vector<vk::CommandBuffer>& buffers, uint32_t count, uint32_t queueFamilyIndex, vk::CommandBufferLevel level)
	{
		PROFILE_FUNCTION

			vk::CommandPoolCreateInfo poolInfo{};

		poolInfo.queueFamilyIndex = queueFamilyIndex;
		poolInfo.flags = vk::CommandPoolCreateFlags(); // Optional

		pools.reserve(count);
		buffers.resize(count);

		for (size_t i = 0; i < count; i++)
		{
			pools.push_back(device.createCommandPool(poolInfo));
			allocateCommandBuffers(device, pools[i], buffers.data() + i, 1, level);
		}

	}

	vk::ImageView createImageView(vk::Device device, VkImage image, ImageViewCreationOptions options)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = image;

		createInfo.viewType = VkImageViewType(options.type);
		createInfo.format = VkFormat(options.format);

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VkImageAspectFlags(options.aspectFlags);
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = options.mipLevels;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		return device.createImageView({ createInfo });
	}

}
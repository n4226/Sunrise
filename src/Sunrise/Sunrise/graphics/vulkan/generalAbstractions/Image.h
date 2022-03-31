#pragma once

#include "srpch.h"
#include "Buffer.h"

namespace sunrise::gfx {


	struct SUNRISE_API ImageCreationOptions {
		// resource spacific
		ResourceStorageType storage;
		vk::ImageUsageFlags usage;
		vk::SharingMode sharingMode;// = vk::SharingMode::eExclusive;
		std::vector<uint32_t> sharingQueueFamilieIndicies;

		// image spacific
		vk::ImageLayout layout;// = vk::ImageLayout::eUndefined;
		vk::ImageType type;
		vk::ImageTiling tilling;// = vk::ImageTiling::eOptimal;
		vk::Format format;

		bool mipmaps = false;

		uint32_t layers = 1;
	};

	struct SUNRISE_API ImageViewCreationOptions {
		vk::ImageViewType type;
		vk::Format format;
		vk::ImageAspectFlags aspectFlags;
		uint32_t mipLevels = 1;
		uint32_t layerCount = 1;
		uint32_t startLayer = 0;
	};

	class SUNRISE_API Image
	{
	public:

		Image(const Image& othter) = delete;
		Image(vk::Device device, VmaAllocator allocator, vk::Extent3D size, ImageCreationOptions options, vk::ImageAspectFlags aspectFlags);
		~Image();

		vk::ImageView createView(uint32_t layers, uint32_t startLayer);

		void name(const char* name, const VkDebug& debugObject) const;

		vk::Extent3D size;
		VmaAllocation allocation = nullptr;
		VkImage vkItem = nullptr;
		uint32_t mipLevels = 1;

		vk::ImageView view;

	private:
		VmaAllocator allocator;
		vk::Device device;

		std::vector<vk::ImageView> allocatedViews;

		ImageCreationOptions options;
		vk::ImageAspectFlags aspectFlags;
	};


}
#include "srpch.h"
#include "Image.h"
#include "VkHelpers.h"


namespace sunrise::gfx {


	Image::Image(vk::Device device, VmaAllocator allocator, vk::Extent3D size, ImageCreationOptions options, vk::ImageAspectFlags aspectFlags)
		: size(size), allocator(allocator), device(device), options(options), aspectFlags(aspectFlags)
	{

		vk::ImageCreateInfo imageInfo{};
		imageInfo.imageType = options.type;
		imageInfo.extent = size;
		if (options.mipmaps && options.type == vk::ImageType::e2D) {
			auto mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(imageInfo.extent.width, imageInfo.extent.height)))) + 1;

			imageInfo.mipLevels = mipLevels;
			this->mipLevels = mipLevels;
		}
		else imageInfo.mipLevels = 1;

		imageInfo.arrayLayers = options.layers;

		imageInfo.format = options.format;

		imageInfo.tiling = options.tilling;

		imageInfo.initialLayout = options.layout;

		imageInfo.usage = options.usage;
		imageInfo.sharingMode = options.sharingMode;

		// currently abstracted out of image creation

		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.flags = {};

		if (options.sharingMode == vk::SharingMode::eConcurrent)
			imageInfo.setQueueFamilyIndices(options.sharingQueueFamilieIndicies);

		VkImageCreateInfo c_imageInfo = static_cast<VkImageCreateInfo>(imageInfo);


		// allocate

		VmaAllocationCreateInfo allocInfo;

		allocInfo.flags = 0;//VMA_ALLOCATION_CREATE_MAPPED_BIT;
		allocInfo.preferredFlags = 0;
		allocInfo.requiredFlags = 0;
		allocInfo.memoryTypeBits = UINT32_MAX;
		allocInfo.pool = nullptr;
		allocInfo.pUserData = nullptr;
		allocInfo.usage = VmaMemoryUsage(options.storage);


		vmaCreateImage(allocator, &c_imageInfo, &allocInfo, &vkItem, &allocation, nullptr);

		ImageViewCreationOptions viewOptions = { vk::ImageViewType::e2D, options.format, aspectFlags, mipLevels, options.layers,0 };

		view = vkHelpers::createImageView(device, vkItem, viewOptions);
	}

	vk::ImageView Image::createView(uint32_t layers, uint32_t startLayer)
	{
		ImageViewCreationOptions viewOptions = { vk::ImageViewType::e2D, options.format, aspectFlags, mipLevels, layers,startLayer };
		auto view = vkHelpers::createImageView(device, vkItem, viewOptions);
		allocatedViews.push_back(view);
		return view;
	}

	Image::~Image()
	{
		for each (auto v in allocatedViews)
			device.destroyImageView(v);

		device.destroyImageView(view);
		vmaDestroyImage(allocator, vkItem, allocation);
	}

	void Image::name(const char* name, const VkDebug& debugObject) const
	{
#if SR_VK_OBJECT_NAMES
		debugObject.nameObject(reinterpret_cast<size_t>(vkItem), vk::DebugReportObjectTypeEXT::eImage, name);

		auto longName = std::string(name) + " View";
		debugObject.nameObject(reinterpret_cast<size_t>(VkImageView(view)), vk::DebugReportObjectTypeEXT::eImageView, longName.c_str());
#endif
	}

}
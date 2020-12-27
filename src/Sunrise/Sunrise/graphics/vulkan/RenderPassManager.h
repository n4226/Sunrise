#pragma once

#include <vulkan/vulkan.hpp>

class RenderPassManager
{
public:

	RenderPassManager(vk::Device device, VkFormat albedoFormat, VkFormat normalFormat, VkFormat aoFormat, VkFormat swapChainImageFormat, VkFormat depthBufferFormat);
	~RenderPassManager();


	VkRenderPass renderPass;

	static const size_t subPassCount = 2;
	static const size_t gbufferAttachmentCount = 3;

private:

	void createMainRenderPass();

	//GBufferFormats

	VkFormat albedoFormat;
	VkFormat normalFormat;
	VkFormat aoFormat;

	VkFormat swapChainImageFormat;
	VkFormat depthBufferFormat;

	vk::Device device;
};


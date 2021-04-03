#pragma once

#include "srpch.h"


namespace sunrise::gfx {

	class RenderPassManager
	{
	public:

		RenderPassManager(vk::Device device, VkFormat albedoFormat, VkFormat normalFormat, VkFormat aoFormat, VkFormat swapChainImageFormat, VkFormat depthBufferFormat);
		virtual ~RenderPassManager();


		VkRenderPass renderPass;

		virtual size_t subPassCount();
		static const size_t gbufferAttachmentCount = 3;

		virtual void createMainRenderPass();

	protected:

		//GBufferFormats

		VkFormat albedoFormat;
		VkFormat normalFormat;
		VkFormat aoFormat;

		VkFormat swapChainImageFormat;
		VkFormat depthBufferFormat;

		vk::Device device;
	};


}
#pragma once

#include "srpch.h"


namespace sunrise::gfx {

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


}
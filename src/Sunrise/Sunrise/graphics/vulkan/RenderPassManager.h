#pragma once

#include "srpch.h"


namespace sunrise::gfx {

	class RenderPassManager
	{
	public:

		RenderPassManager(vk::Device device, VkFormat albedoFormat, VkFormat normalFormat, VkFormat aoFormat, VkFormat swapChainImageFormat, VkFormat depthBufferFormat);
		virtual ~RenderPassManager();


		VkRenderPass renderPass;



		virtual size_t getSubPassCount();
		virtual size_t getTotalAttatchmentCount();
		static const size_t gbufferAttachmentCount = 3;

		virtual void createMainRenderPass();

		//TODO: make better api for multiviewport
		bool multiViewport = false;
		size_t multiViewCount = 1;

		static constexpr size_t subPassCount = 2;

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
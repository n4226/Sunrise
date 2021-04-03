#pragma once

#include "srpch.h"
#include "RenderPassManager.h"

namespace sunrise::gfx {

	class SingalPassRenderPassManager: public RenderPassManager
	{
	public:
		//using RenderPassManager::RenderPassManager;
		SingalPassRenderPassManager(vk::Device device, VkFormat albedoFormat, VkFormat normalFormat, VkFormat aoFormat, VkFormat swapChainImageFormat, VkFormat depthBufferFormat);
		~SingalPassRenderPassManager();


		//VkRenderPass renderPass;

		size_t subPassCount() override;
		static const size_t gbufferAttachmentCount = 3;

	private:

		void createMainRenderPass() override;

		//GBufferFormats

	/*	VkFormat albedoFormat;
		VkFormat normalFormat;
		VkFormat aoFormat;

		VkFormat swapChainImageFormat;
		VkFormat depthBufferFormat;

		vk::Device device;*/
	};


}
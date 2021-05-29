#pragma once

#include "srpch.h"
#include "RenderPassManager.h"

namespace sunrise::gfx {

	class GPUPassRenderPassManager: public RenderPassManager
	{
	public:
		//using RenderPassManager::RenderPassManager;
		GPUPassRenderPassManager(vk::Device device, VkFormat albedoFormat, VkFormat normalFormat, VkFormat aoFormat, VkFormat swapChainImageFormat, VkFormat depthBufferFormat);
		~GPUPassRenderPassManager();


		//VkRenderPass renderPass;

		size_t getSubPassCount() override;
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
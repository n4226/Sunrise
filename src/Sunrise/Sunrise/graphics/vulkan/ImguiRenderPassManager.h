#pragma once

#include "srpch.h"
#include "RenderPassManager.h"

namespace sunrise::gfx {

	class ImguiRenderPassManager : public RenderPassManager
	{
	public:
		//using RenderPassManager::RenderPassManager;
		ImguiRenderPassManager(vk::Device device, VkFormat albedoFormat, VkFormat normalFormat, VkFormat aoFormat, VkFormat swapChainImageFormat, VkFormat depthBufferFormat);
		~ImguiRenderPassManager();


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
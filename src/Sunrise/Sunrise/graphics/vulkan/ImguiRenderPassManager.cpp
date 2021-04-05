#include "srpch.h"
#include "ImguiRenderPassManager.h"

namespace sunrise::gfx {

	ImguiRenderPassManager::ImguiRenderPassManager(vk::Device device, VkFormat albedoFormat, VkFormat normalFormat, VkFormat aoFormat, VkFormat swapChainImageFormat, VkFormat depthBufferFormat)
		: RenderPassManager(device, albedoFormat, normalFormat, aoFormat, swapChainImageFormat, depthBufferFormat)
	{
	}

	ImguiRenderPassManager::~ImguiRenderPassManager()
	{
		device.destroyRenderPass(renderPass);
	}

	size_t ImguiRenderPassManager::getSubPassCount()
	{
		return 2;
	}
#if SR_RELEASE || SR_DIST
#pragma optimize( "", off )
#endif
	void ImguiRenderPassManager::createMainRenderPass()
	{
		/*VkAttachmentDescription attachment = {};
		attachment.format = swapChain.imagesFormat;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(1);
		renderPassInfo.pAttachments = attachment;
		renderPassInfo.subpassCount = subpasses.size();
		renderPassInfo.pSubpasses = subpasses.data();
		renderPassInfo.dependencyCount = dependencies.size();
		renderPassInfo.pDependencies = dependencies.data();

		renderPass = device.createRenderPass(renderPassInfo);*/

	}
#if SR_RELEASE || SR_DIST
#pragma optimize( "", on ) 
#endif

}
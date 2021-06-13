#include "srpch.h"
#include "GPURenderStage.h"

#include "Sunrise/Sunrise/core/Application.h"
#include "Sunrise/Sunrise/core/Window.h"
#include "Sunrise/Sunrise/graphics/vulkan/renderer/Renderer.h"

namespace sunrise::gfx {

	GPURenderStage::GPURenderStage(Application& app, std::string&& name)
		: GPUStage(app,std::move(name))
	{
		createRequiredRenderResources();
	}

	GPURenderStage::~GPURenderStage()
	{

	}

	void GPURenderStage::createRequiredRenderResources()
	{
		PROFILE_FUNCTION;

		SR_CORE_TRACE("Creating {} GPU Render Stage resources", name);


		cmdBufferPools.resize(app.renderers[0]->windows.size());
		commandBuffers.resize(app.renderers[0]->windows.size());

		for (size_t i = 0; i < app.renderers[0]->windows.size(); i++)
			vkHelpers::createPoolsAndCommandBufffers
			(app.renderers[0]->device, cmdBufferPools[i], commandBuffers[i], app.maxSwapChainImages, app.renderers[0]->queueFamilyIndices.graphicsFamily.value(), vk::CommandBufferLevel::eSecondary);

	}


	//TODO: right now this is assuming what framebuffer and render pass etc to use
	vk::CommandBuffer* GPURenderStage::selectAndSetupCommandBuff(uint32_t subpass, sunrise::Window& window)
	{
		uint32_t bufferIndex = window.currentSurfaceIndex;

		auto renderer = app.renderers[0];

		renderer->device.resetCommandPool(cmdBufferPools[window.indexInRenderer][bufferIndex], {});

		vk::CommandBuffer* buffer = &commandBuffers[window.indexInRenderer][bufferIndex];


		vk::CommandBufferInheritanceInfo inheritanceInfo{};
		inheritanceInfo.renderPass = window.renderPassManager->renderPass;
		inheritanceInfo.subpass = subpass;
		inheritanceInfo.framebuffer = window.swapChainFramebuffers[bufferIndex];

		vk::CommandBufferBeginInfo beginInfo{};
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eRenderPassContinue; // Optional
		beginInfo.pInheritanceInfo = &inheritanceInfo; // Optional

		buffer->begin(beginInfo);

		return buffer;
	}

	void GPURenderStage::setPipeline(sunrise::Window& window, vk::CommandBuffer buffer, VirtualGraphicsPipeline* pipeline)
	{
		auto rawPipe = window.loadedPipes[pipeline];

		buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, rawPipe->vkItem);
	}

	

}
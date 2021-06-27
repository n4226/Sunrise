#include "srpch.h"
#include "GPURenderStage.h"

#include "Sunrise/Sunrise/core/Application.h"
#include "Sunrise/Sunrise/core/Window.h"
#include "Sunrise/Sunrise/graphics/vulkan/renderer/Renderer.h"
#include "Sunrise/Sunrise/graphics/vulkan/renderer/SceneRenderCoordinator.h"

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


	vk::CommandBuffer* GPURenderStage::selectAndSetupCommandBuff(RunOptions options)
	{
		uint32_t bufferIndex = options.window.currentSurfaceIndex;

		auto renderer = options.window.renderer;

		//TODO write down why i am using a comand buffer and pool per serface if they are reset each frame - think it has to do with inflight frames
		renderer->device.resetCommandPool(cmdBufferPools[options.window.indexInRenderer][bufferIndex], {});

		vk::CommandBuffer* buffer = &commandBuffers[options.window.indexInRenderer][bufferIndex];

		auto [renderPass, subpass] = options.coordinator->sceneRenderpassHolders[0]->renderPass(options.pass);

		vk::CommandBufferInheritanceInfo inheritanceInfo{};
		inheritanceInfo.renderPass = renderPass->renderPass;
		inheritanceInfo.subpass = subpass;
		inheritanceInfo.framebuffer = options.coordinator->sceneRenderpassHolders[0]->getFrameBuffer(options.pass,&options.window,bufferIndex);

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

	GraphicsPipeline* GPURenderStage::getConcretePipeline(sunrise::Window& window, VirtualGraphicsPipeline* pipeline)
	{
		return window.loadedPipes[pipeline];
	}

	

}
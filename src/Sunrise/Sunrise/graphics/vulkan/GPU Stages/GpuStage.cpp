#include "srpch.h"
#include "GpuStage.h"

#include "Sunrise/Sunrise/graphics/vulkan/renderer/SceneRenderCoordinator.h"

namespace sunrise::gfx {

	GPUStage::GPUStage(SceneRenderCoordinator* coord, std::string&& name)
		: app(coord->app), coord(coord)
#if SR_LOGGING
		,name(name)
#endif
	{

	}

	GPUStage::~GPUStage()
	{

	}


	vk::CommandBuffer* GPUStage::selectAndSetupCommandBuff(RunOptions options)
	{
		uint32_t bufferIndex = options.window.currentSurfaceIndex;

		auto renderer = options.window.renderer;

		//TODO write down why i am using a command buffer and pool per surface if they are reset each frame - think it has to do with inflight frames
		renderer->device.resetCommandPool(cmdBufferPools[options.window.indexInRenderer][bufferIndex], {});

		vk::CommandBuffer* buffer = &commandBuffers[options.window.indexInRenderer][bufferIndex];

		setupCommandBuff(*buffer, options.coordinator, options.pass, options.window, bufferIndex);

		return buffer;
	}

	void GPUStage::setupCommandBuff(vk::CommandBuffer buff, SceneRenderCoordinator* coordinator, size_t pass, const Window& window, size_t surface, vk::CommandBufferUsageFlags flags)
	{
		auto [renderPass, subpass] = coordinator->sceneRenderpassHolders[0]->renderPass(pass);

		vk::CommandBufferInheritanceInfo inheritanceInfo{};
		inheritanceInfo.renderPass = renderPass->renderPass;
		inheritanceInfo.subpass = subpass;
		inheritanceInfo.framebuffer = coordinator->sceneRenderpassHolders[0]->getFrameBuffer(pass, &window, surface);

		vk::CommandBufferBeginInfo beginInfo{};
		beginInfo.flags = flags; // Optional
		beginInfo.pInheritanceInfo = &inheritanceInfo; // Optional

		buff.begin(beginInfo);
	}

}
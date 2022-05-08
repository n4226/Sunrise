#include "srpch.h"
#include "GPURenderStage.h"

#include "Sunrise/Sunrise/core/Application.h"
#include "Sunrise/Sunrise/core/Window.h"
#include "Sunrise/Sunrise/graphics/vulkan/renderer/Renderer.h"
#include "Sunrise/Sunrise/graphics/vulkan/renderer/SceneRenderCoordinator.h"

namespace sunrise::gfx {

	GPURenderStage::GPURenderStage(SceneRenderCoordinator* coord, std::string&& name, bool useInternalresources)
		: GPUStage(coord,std::move(name)), useInternalresources(useInternalresources)
	{
		if (useInternalresources)
			createRequiredRenderResources();
	}

	GPURenderStage::~GPURenderStage()
	{
		//todo destroy resources if useInternalResources
		if (useInternalresources) {

			for (auto pool : cmdBufferPools)
				for (auto spool : pool)
					coord->renderer->device.destroyCommandPool(spool);
		}
	}

	void GPURenderStage::createRequiredRenderResources()
	{
		PROFILE_FUNCTION;

		SR_CORE_TRACE("Creating {} GPU Stage resources", name);


		cmdBufferPools.resize(coord->renderer->windows.size());
		commandBuffers.resize(coord->renderer->windows.size());

		for (size_t i = 0; i < coord->renderer->windows.size(); i++)
			vkHelpers::createPoolsAndCommandBufffers
			(coord->renderer->device, cmdBufferPools[i], commandBuffers[i], app.maxSwapChainImages, coord->renderer->queueFamilyIndices.graphicsFamily.value(), vk::CommandBufferLevel::eSecondary);

	}


	void GPURenderStage::setPipeline(const sunrise::Window& window, vk::CommandBuffer buffer, VirtualGraphicsPipeline* pipeline)
	{
		auto rawPipe = getConcretePipeline(window, pipeline);

		buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, rawPipe->vkItem);
	}

	//TODO change this to a genaric one which finds the regestered pipeline of that type 
	GraphicsPipeline* GPURenderStage::getConcretePipeline(const sunrise::Window& window, VirtualGraphicsPipeline* pipeline)
	{
		return window.loadedPipes.find(pipeline)->second;
	}



	void GPURenderStage::registerPipeline(VirtualGraphicsPipeline* virtualPipe)
	{
		coord->registerPipeline(virtualPipe, this);
	}


}
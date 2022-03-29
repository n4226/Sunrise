#pragma once

#include "srpch.h"
#include "../../ComposableRenderPass.h"
#include "../../renderPipelines/GraphicsPipeline.h"


namespace sunrise::gfx {

	class RenderTerget
	{
	public:




		int getSurfaceCount();

		uint32_t currentSurfaceIndex();

	protected:

		VkFormat imageFormat;
		VkExtent2D imageExtent;
		size_t imageLayers;

		uint32_t _currentSurfaceIndex = 0;

		std::unordered_map<const gfx::VirtualGraphicsPipeline*, gfx::GraphicsPipeline*> loadedPipes = {};


		// Synchronization objects
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		/// <summary>
		/// indicies are frames i.e. app.currentFrame
		/// </summary>
		std::vector<VkFence> inFlightFences;
		/// <summary>
		/// indicies are surface indexes i.e. RenderTarget.currentSurfaceIndex
		/// </summary>
		std::vector<VkFence> imagesInFlight;


		void createSwapchainAndImages(const std::vector<ComposableRenderPass::CreateOptions::VAttatchment>& attachments);

		Image createImageForAttachment(const ComposableRenderPass::CreateOptions::VAttatchment& attachment);

	private:
	};

}
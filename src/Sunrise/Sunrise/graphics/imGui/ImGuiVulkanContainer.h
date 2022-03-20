#pragma once


#include "srpch.h"

#include "../vulkan/GPU Stages/GPURenderStage.h"
#include "../vulkan/ComposableRenderPass.h"

namespace sunrise::gfx {

	class ImGuiVulkanContainer
	{

	public:

		void generateIMGUIResources(Application& app);
		void destroyIMGUIResources(Application& app);

		void setupIMGUIForWindow(Window* window, Application& app);
		void cleanupIMGUIForWindow(Window* window, Application& app);

		void setupIMGUIForRenderer(Renderer* renderer, Application& app);

	protected:


		vk::RenderPass renderPass;

		gfx::DescriptorPool* descriptorPool;

	};

}
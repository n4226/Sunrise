#include "srpch.h"
#include "UISceneRenderCoordinator.h"

namespace sunrise {


	void UISceneRenderCoordinator::createPasses()
	{

		generateImguiStage = false;
		auto guiStage = new gfx::ImGuiStage(this);

		registerStage(guiStage, {}, {},{});

		setLastStage(guiStage);

	}

	gfx::ComposableRenderPass::CreateOptions UISceneRenderCoordinator::renderpassConfig(vk::Format swapChainFormat)
	{
		auto deferredA = gfx::ComposableRenderPass::CreateOptions::VAttatchment();

		deferredA.type = gfx::ComposableRenderPass::CreateOptions::AttatchmentType::Color;
		deferredA.format = swapChainFormat;
		deferredA.loadOp = vk::AttachmentLoadOp::eClear;
		deferredA.initialLayout = vk::ImageLayout::eUndefined;
		deferredA.transitionalToAtStartLayout = vk::ImageLayout::eColorAttachmentOptimal;
		//deferredA.transitionalToAtStartLayout = vk::ImageLayout::eColorAttachmentOptimal;//vk::ImageLayout::eColorAttachmentOptimal;
		deferredA.finalLayout = vk::ImageLayout::ePresentSrcKHR;
		//glm::vec3 selectedColor{ 13, 63, 143 };
		glm::vec3 selectedColor{ 11, 50, 114 };
		selectedColor /= 255;
		deferredA.clearColor = { selectedColor.r, selectedColor.g, selectedColor.b, 1.0f };
		deferredA.name = "FinalRenderTarget";

		gfx::ComposableRenderPass::CreateOptions options = { {deferredA}, 0 };

		return options;
	}

}
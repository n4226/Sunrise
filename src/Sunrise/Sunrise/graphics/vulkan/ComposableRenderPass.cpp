 #include "srpch.h"
#include "ComposableRenderPass.h"
#include "Sunrise/Sunrise/core/Application.h"
#include "Sunrise/Sunrise/graphics/vulkan/renderer/Renderer.h"

namespace sunrise::gfx {


	ComposableRenderPass::ComposableRenderPass(Application& app, CreateOptions&& options)
		: RenderPassManager(app.renderers[0]->device,
			VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED), app(app)
	{
		createMainRenderPass();
	}

	void ComposableRenderPass::createMainRenderPass()
	{

	}

	size_t ComposableRenderPass::getSubPassCount()
	{
		//TODO add suport for multi sub passes - for now render coordinator will manag dependancies and only use one subpass
		return 1;
	}

	ComposableRenderPass::~ComposableRenderPass()
	{
	}

}
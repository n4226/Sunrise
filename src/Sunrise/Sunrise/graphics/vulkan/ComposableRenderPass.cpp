 #include "srpch.h"
#include "ComposableRenderPass.h"
#include "Sunrise/Sunrise/core/Application.h"
#include "Sunrise/Sunrise/core/Window.h"
#include "Sunrise/Sunrise/graphics/vulkan/renderer/Renderer.h"

namespace sunrise::gfx {


	ComposableRenderPass::ComposableRenderPass(Renderer* renderer, CreateOptions&& options)
		: RenderPassManager(renderer->device,
			VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED), 
		renderer(renderer),
		options(std::move(options))
	{
		createMainRenderPass();
		createWindowSpacificResources();
	}

	ComposableRenderPass::~ComposableRenderPass()
	{
	}

	void ComposableRenderPass::createWindowSpacificResources()
	{
		//TODO only works for one renderer/gpu

		// for each unowned window create instances of all attatchments exept the one which will be presented as that is created by the swapchain
		for (auto win : renderer->allWindows) {
			if (!win->isOwned()) {
				//createWindowRenderPass(win);

				createWindowImagesAndFrameBuffer(win);

			}
		}
	}


	void ComposableRenderPass::createWindowImagesAndFrameBuffer(Window* window)
	{
		// create framebuffer images

		ImageCreationOptions createOptions{};

		createOptions.sharingMode = vk::SharingMode::eExclusive;
		createOptions.storage = ResourceStorageType::gpu;

		createOptions.type = vk::ImageType::e2D;
		createOptions.layout = vk::ImageLayout::eUndefined;
		createOptions.tilling = vk::ImageTiling::eOptimal;

		createOptions.layers = 1;
		if (multiViewport)
			createOptions.layers = multiViewCount;

		for (size_t i = 0; i < vattachments.size(); i++)
		{
			if (i == options.presentedAttachment) { continue; }
			auto vatt = vattachments[i];

			createOptions.usage = vatt->usage;
			createOptions.format = vatt->format;

			auto aspect = vk::ImageAspectFlagBits::eColor;

			if (vatt->type == CreateOptions::AttatchmentType::Depth)
				aspect = vk::ImageAspectFlagBits::eDepth;

			auto attatchImage = new Image(device, renderer->allocator, { window->swapchainExtent.width,window->swapchainExtent.height,1 }, createOptions, aspect);

			if (images.find(window) == images.end()) {
				images[window] = new std::vector<Image*>();
				images.reserve(vattachments.size());
			}
			images[window]->push_back(attatchImage);
		}

		if (vattachments.size() == 1) {
			images[window] = new std::vector<Image*>();
		}


		window->swapChainFramebuffers.resize(window->swapChainImageViews.size());
		for (size_t i = 0; i < window->swapChainImageViews.size(); i++) {
			// see renderpass.cpp for info on order of attachments
			std::vector<vk::ImageView> attachments = {};
			attachments.reserve(vattachments.size());

			auto winAttachments = images[window];
			
			// this is so that the indixies line up between local images and swap chain images stored in windows themselvs
			bool passedSwapImage = false;
			for (size_t a = 0; a < vattachments.size(); a++)
			{
				if (a == options.presentedAttachment) {
					attachments.push_back(window->swapChainImageViews[i]);
					passedSwapImage = true;
				}
				else {
					attachments.push_back((*winAttachments)[passedSwapImage ? i - 1 : i]->view);
				}
			}

			vk::FramebufferCreateInfo framebufferInfo{};
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = attachments.size();
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width =  window->swapchainExtent.width;
			framebufferInfo.height = window->swapchainExtent.height;
			framebufferInfo.layers = 1;

			window->swapChainFramebuffers[i] = device.createFramebuffer(framebufferInfo);
		}
	}

	//TODO look at othe render pass manager where optimizitions were turned off just for the creation functoin like this one. I beilive this was becasuze it stopped soroking on release or dist buids
	void ComposableRenderPass::createMainRenderPass()
	{
		// converts abstract definiton given in create options and parses it into a vk Render pass

		std::vector<vk::AttachmentDescription> attachments;
		std::vector<vk::AttachmentReference> colorReferences;
		std::vector<vk::AttachmentReference> depthReferences;
		attachments.reserve(options.attatchments.size());
		colorReferences.reserve(options.attatchments.size());
		depthReferences.reserve(1);

		for (auto vatt : options.attatchments) {
			auto heapVatt = new CreateOptions::VAttatchment(vatt);
			vattachments.push_back(heapVatt);

			auto des = descriptonFromVatt(heapVatt);
			auto ref = referenceFromVatt(heapVatt);

			attachments.push_back(des);
			if (vatt.type == CreateOptions::AttatchmentType::Color) {
				colorReferences.push_back(ref);
			}
			else if (vatt.type == CreateOptions::AttatchmentType::Depth) {
				depthReferences.push_back(ref);
			}
		}

		vk::SubpassDescription mainSubPass{};
		{
			// flags are whre can enable per view attributes for multi view one gpu rendereing in the future
			mainSubPass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;

			// the output color attatchments to this subpass
			mainSubPass.colorAttachmentCount = colorReferences.size();
			mainSubPass.pColorAttachments = colorReferences.data();

			// the input attatchmets to this subpass
			//TODO: right now there are never input attatchments because there is only one subpass - I hope this does not cause issues
			mainSubPass.inputAttachmentCount = 0;
			mainSubPass.pInputAttachments = nullptr;

			if (depthReferences.size() == 1) {
				mainSubPass.pDepthStencilAttachment = depthReferences.data();
			} else mainSubPass.pDepthStencilAttachment = nullptr;

			//todo add support for resolve attatchments for MSAA

		}

		// create render pass

		// this exdternal dependancy is so that writing to attatchments in the render pass is not done until the image is given from the presentation engine
		VkSubpassDependency externalGBufferDependency{};
		externalGBufferDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		externalGBufferDependency.dstSubpass = 0;
		// not sure what VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT is for anymore might have to look bake to vuilkan tutorial - probably just the same thing just for the depth attaqchment
		externalGBufferDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		externalGBufferDependency.srcAccessMask = 0;

		externalGBufferDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		externalGBufferDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<vk::SubpassDependency, 1> dependencies = {
			externalGBufferDependency
		};

		//TODO Not funcitonal yet just for show right now
		VkRenderPassMultiviewCreateInfo multiViewInfo{};
		if (multiViewport) { // multi view setup
			// see vulkan docs: https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkRenderPassMultiviewCreateInfo.html
			// Some implementations may not support multiview in conjunction with geometry shaders or tessellation shaders.

			multiViewInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO;
			multiViewInfo.pNext = nullptr;
			multiViewInfo.subpassCount = subPassCount;

			std::array<uint32_t, subPassCount> viewMasks;

			for (size_t i = 0; i < subPassCount; i++)
			{
				viewMasks[i] = 2 ^ (multiViewCount)-1;
			}

			multiViewInfo.pViewMasks = viewMasks.data();

			// " It  viewMasks; ount is zero, each dependency’s view offset is treated as zero. "
			// these are the offsets of the dpendencies for ech view but since each "layer" only depends on the same layer in the previus pass these should be left zero
			multiViewInfo.dependencyCount = 0;
			multiViewInfo.pViewOffsets = nullptr;

			/*TODO: allow for windows tro be rendnered:
				all individually without mvr
				multiple groups of mvr views (where each mvr group is a window class which is a fake window)
				one mvr view group
			*/



		}

		vk::RenderPassCreateInfo renderPassInfo{};
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &mainSubPass;
		renderPassInfo.dependencyCount = dependencies.size();
		renderPassInfo.pDependencies = dependencies.data();

		if (multiViewport)
			renderPassInfo.pNext = &multiViewInfo;
		
		renderPass = device.createRenderPass(renderPassInfo);
	}

	size_t ComposableRenderPass::getTotalAttatchmentCount()
	{
		return options.attatchments.size();
	}

	size_t ComposableRenderPass::getSubPassCount()
	{
		//TODO add suport for multi sub passes - for now render coordinator will manag dependancies and only use one subpass
		return 1;
	}


	vk::AttachmentDescription ComposableRenderPass::descriptonFromVatt(CreateOptions::VAttatchment* vatt)
	{
		vk::AttachmentDescription description{};
		description.format = vatt->format;
		description.samples = vatt->samples;

		description.loadOp = vatt->loadOp;
		description.storeOp = vatt->storeOp;

		description.stencilLoadOp = vatt->stencilLoadOp;
		description.stencilStoreOp = vatt->stencilStoreOp;

		description.initialLayout = vatt->initialLayout;
		description.finalLayout = vatt->finalLayout;

		return description;
	}

	vk::AttachmentReference ComposableRenderPass::referenceFromVatt(CreateOptions::VAttatchment * vatt)
	{
		auto index = std::find(vattachments.begin(), vattachments.end(), vatt);

		SR_ASSERT(index != vattachments.end());

		vk::AttachmentReference reff{};
		reff.attachment = index - vattachments.begin();
		// vulkan will transfer image to this layout at start of subpass
		reff.layout = vatt->transitionalToAtStartLayout;

		return reff;
	}

}
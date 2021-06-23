#include "srpch.h"
#include "CRPHolder.h"

#include "renderer/Renderer.h"
#include "Sunrise/Sunrise/core/Window.h"

namespace sunrise::gfx {
	inline CRPHolder::CRPHolder(const ComposableRenderPass::CreateOptions& wholeOptions, const HolderOptions& spacificOptions, Renderer* renderer)
		: renderer(renderer), frameOptions(wholeOptions)
	{

		/* tasks:

		for each pass create a new render pass passing in modified options

		*/
		auto options = wholeOptions;

		// copy shananagins is need for passes which dont need all attachments
		bool copyNeeded = false;
		// array of indexes needed to be left out of copy
		std::vector<size_t> copyWithoutIndexes;
		ComposableRenderPass::CreateOptions localCopy{};


		for (size_t i = 0; i < spacificOptions.passes; i++)
		{
			SR_ASSERT(spacificOptions.passStartLayout[i].size() == options.attatchments.size());

			// change options for spacific render pass
			for (size_t attach = 0; attach < options.attatchments.size(); attach++)
			{
				if (spacificOptions.passStartLayout[i][attach] == vk::ImageLayout::eUndefined) {
					// this attachment is not used in this pass
					copyNeeded = true;
					copyWithoutIndexes.push_back(attach);
					continue;
				}

				if (i > 0) {
					// the initial layout (the layout befoer the render pass begins) shoulod be the final layout of the last pass
					options.attatchments[attach].initialLayout = options.attatchments[attach].finalLayout;
				}
				// the start layout (the layout to transtion to at the beginnign of the renderpass) should be what the dependancy needs
				options.attatchments[attach].transitionalToAtStartLayout = spacificOptions.passStartLayout[i][attach];

				if (i == options.attatchments.size() - 1) { // if the last pass
															// the final layout (the layout to transtion to at the end of the renderpass) should be the original final user defined layout
					options.attatchments[attach].finalLayout = wholeOptions.attatchments[attach].finalLayout;
				}
				else {
					// the final layout (the layout to transtion to at the end of the renderpass) should be what it transition to at the beggingin ofthe frame
					//becuase the layout transition to the next pass will happen at the beggignin of the next pass
					options.attatchments[attach].finalLayout = options.attatchments[attach].transitionalToAtStartLayout;
				}
			}

			ComposableRenderPass* pass;

			if (copyNeeded) {
				localCopy.presentedAttachment = options.presentedAttachment;

				for (size_t newA = 0; newA < options.attatchments.size(); newA++)
				{
					bool copyAttach = true;
					for (auto index : copyWithoutIndexes) {
						if (newA == index)
							copyAttach = false;
					}
					if (copyAttach)
						localCopy.attatchments.push_back(options.attatchments[newA]);
				}
				pass = new ComposableRenderPass(renderer,localCopy);

				localCopy = {};
				copyWithoutIndexes = {};
				copyNeeded = false;
			}
			else
				pass = new ComposableRenderPass(renderer, options);

			renderpasses.push_back(pass);
		}

	}
	void CRPHolder::createWindowSpacificResources()
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


	void CRPHolder::createWindowImagesAndFrameBuffer(Window* window)
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

		// for each virtual attachment (in the first pass) create all images exept the one for the swapchain
		for (size_t i = 0; i < vattachments[0].size(); i++)
		{
			// skip the swap image
			if (i == options.presentedAttachment) { continue; }
			//for now can use only vatts from first pass as all fields used in this func are the same for all passes
			auto vatt = vattachments[0][i];
			createOptions.usage = vatt->usage;
			createOptions.format = vatt->format;


			auto aspect = vk::ImageAspectFlagBits::eColor;

			if (vatt->type == ComposableRenderPass::CreateOptions::AttatchmentType::Depth)
				aspect = vk::ImageAspectFlagBits::eDepth;

			auto attatchImage = new Image(renderer->device, renderer->allocator, { window->swapchainExtent.width,window->swapchainExtent.height,1 }, createOptions, aspect);

#if SR_VK_OBJECT_NAMES
			const char* name = vatt->name.append("_%d", window->globalIndex).c_str();

			VkDebug::nameObject(renderer->device, reinterpret_cast<size_t>(attatchImage->vkItem), vk::DebugReportObjectTypeEXT::eImage, name);
#endif
			//if no array object for this window than make an empty one
			if (images.find(window) == images.end()) {
				images[window] = std::vector<Image*>();
				images.reserve(vattachments.size());
			}
			images[window].push_back(attatchImage);
		}

		// if the only attachment is the swap image and hense no empty array was created in the loop create one here
		if (vattachments.size() == 1) {
			images[window] = std::vector<Image*>();
		}

		// make all frame buffers:
		{

			for (size_t pass = 0; pass < option; pass++)
			{

			}


			// the window->swapChainFramebuffers just has references to the same frame buffers in the last passes frame buffers of this class
			window->swapChainFramebuffers.resize(window->swapChainImageViews.size());
			for (size_t i = 0; i < window->swapChainImageViews.size(); i++) { // i is the swap chain image index
				// see renderpass.cpp for info on order of attachments
				std::vector<vk::ImageView> attachments = {};
				attachments.reserve(vattachments.size());

				auto& winAttachments = images[window];

				// this is so that the indixies line up between local images and swap chain images stored in windows themselvs
				bool passedSwapImage = false;
				for (size_t a = 0; a < vattachments.size(); a++)
				{
					if (a == options.presentedAttachment) {

						auto imageView = window->swapChainImageViews[i];

#if SR_VK_OBJECT_NAMES
						const char* name = vattachments[a]->name.append("_%d", window->globalIndex).append("_%d", i).c_str();

						VkDebug::nameObject(device, reinterpret_cast<size_t>(VkImageView(imageView)), imageView.debugReportObjectType, name);
#endif

						attachments.push_back(imageView);
						passedSwapImage = true;
					}
					else {
						attachments.push_back(winAttachments[passedSwapImage ? a - 1 : a]->view);
					}
				}

				// make frame buffer for this swap chain view

				vk::FramebufferCreateInfo framebufferInfo{};
				framebufferInfo.renderPass = renderPass;
				framebufferInfo.attachmentCount = attachments.size();
				framebufferInfo.pAttachments = attachments.data();
				framebufferInfo.width = window->swapchainExtent.width;
				framebufferInfo.height = window->swapchainExtent.height;
				framebufferInfo.layers = 1;

				window->swapChainFramebuffers[i] = device.createFramebuffer(framebufferInfo);
			}
		}
	}

}
#include "srpch.h"
#include "CRPHolder.h"

#include "renderer/Renderer.h"
#include "Sunrise/Sunrise/core/Window.h"

namespace sunrise::gfx {
	inline CRPHolder::CRPHolder(const ComposableRenderPass::CreateOptions& wholeOptions, const HolderOptions& spacificOptions, Renderer* renderer)
		: renderer(renderer), frameOptions(wholeOptions), holderOptions(spacificOptions)
	{
		createPasses(wholeOptions, spacificOptions);
		createWindowSpacificResources();
	}


	/// <summary>
	/// returns the CRP and the supbass within it to use for a "logical pass"
	/// </summary>
	/// <param name="pass"></param>
	/// <returns>the CRP object and the pass within it</returns>

	inline std::pair<ComposableRenderPass*, size_t> CRPHolder::renderPass(size_t pass) {
		SR_ASSERT(pass < renderpasses.size());
		return std::make_pair(renderpasses[pass], 0);
	}

	void CRPHolder::createPasses(const sunrise::gfx::ComposableRenderPass::CreateOptions& wholeOptions, const sunrise::gfx::CRPHolder::HolderOptions& spacificOptions)
	{
		SR_ASSERT(wholeOptions.presentedAttachment >= 0);

		/* tasks:

		for each pass create a new render pass passing in modified options

		*/

		//WARNING - this function is a mess
		for (size_t i = 0; i < spacificOptions.passes; i++)
		{
			ComposableRenderPass::CreateOptions thisPassOptions{};

			// in passes, none of the attachments are the swap chain image 
			// UNLESS THAT atachment is present in the pass
			thisPassOptions.presentedAttachment = -1;
			// add empty vector to be filled with the global indicies for the attachments of this pass
			passAttachGlobalIndicies.push_back({});

#if SR_ENABLE_PRECONDITION_CHECKS
			if (i > 0) 
			// checking that a layout for all atributes was specified in the dependancy
				SR_ASSERT(spacificOptions.passStartLayout[i - 1].size() == wholeOptions.attatchments.size());
#endif
			// change options for spacific render pass
			for (size_t attach = 0; attach < wholeOptions.attatchments.size(); attach++)
			{
				if ((i == 0 && wholeOptions.attatchments[attach].transitionalToAtStartLayout == vk::ImageLayout::eUndefined)
					|| (i > 0 && spacificOptions.passStartLayout[i - 1][attach] == vk::ImageLayout::eUndefined)) {
					// this attachment is not used in this pass
					continue;
				}

				auto attachOptions = wholeOptions.attatchments[attach];

				// if this attachment will be the swapImage mark it as so
				if (attach == wholeOptions.presentedAttachment) {
					thisPassOptions.presentedAttachment = thisPassOptions.attatchments.size();
				}

				if (i > 0) {
					// the initial layout (the layout befoer the render pass begins) shoulod be the final layout of the last pass
					attachOptions.initialLayout = passOptions[passOptions.size() - 1].attatchments[attach].finalLayout;
				}
				// the start layout (the layout to transtion to at the beginnign of the renderpass) should be what the dependancy needs
				// or the initial one specified if it is the first pass
				if (i > 0)
					attachOptions.transitionalToAtStartLayout = spacificOptions.passStartLayout[i - 1][attach];
				else
					attachOptions.transitionalToAtStartLayout = wholeOptions.attatchments[attach].transitionalToAtStartLayout;

				if (i == spacificOptions.passes - 1) { // if the last pass
																 // the final layout (the layout to transtion to at the end of the renderpass) should be the original final user defined layout
					attachOptions.finalLayout = wholeOptions.attatchments[attach].finalLayout;
				}
				else {
					// the NOT final layout 
					// (the layout to transtion to at the end of this renderpass but there is another render pass after) 
					// should be what it transition to at the beggingin of this pass
					//becuase the layout transition to the next pass will happen at the beggignin of the next pass
					attachOptions.finalLayout = attachOptions.transitionalToAtStartLayout;
				}
				thisPassOptions.attatchments.push_back(attachOptions);
				passAttachGlobalIndicies[passAttachGlobalIndicies.size() - 1].push_back(attach);
			}

			auto pass = new ComposableRenderPass(renderer, thisPassOptions);

			renderpasses.push_back(pass);
			//TODO: make sure move constructore actualy exists for the create options class
			passOptions.push_back(std::move(thisPassOptions));
		}
	}


	/// <summary>
	/// returns the concreete image for a window of a "virtual" attachment index
	/// index must not be the index of the swapchain drawable as the render pass does not own those images.
	/// </summary>
	/// <param name="index"></param>
	/// <param name="window"></param>
	/// <returns></returns>

	inline Image* CRPHolder::getImage(size_t globalIndex, Window* window)
	{
		SR_ASSERT(globalIndex != frameOptions.presentedAttachment);
		auto image = images[window][(globalIndex > frameOptions.presentedAttachment) ? globalIndex - 1 : globalIndex];

		return image;
	}

	inline vk::Framebuffer CRPHolder::getFrameBuffer(size_t pass, Window* window, size_t surfaceIndex) {
		return frameBuffers[pass][window][surfaceIndex];
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

		//TODO: ut oh ??? do you need a copy of each image per swapchain image ? - i think so
		// create framebuffer images
		{ 
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
			for (size_t i = 0; i < frameOptions.attatchments.size(); i++)
			{
				// skip the swap image
				if (i == frameOptions.presentedAttachment) { continue; }

				auto& vatt = frameOptions.attatchments[i];
				createOptions.usage = vatt.usage;
				createOptions.format = vatt.format;


				auto aspect = vk::ImageAspectFlagBits::eColor;

				if (vatt.type == ComposableRenderPass::CreateOptions::AttatchmentType::Depth)
					aspect = vk::ImageAspectFlagBits::eDepth;

				auto attatchImage = new Image(renderer->device, renderer->allocator, { window->swapchainExtent.width,window->swapchainExtent.height,1 }, createOptions, aspect);

#if SR_VK_OBJECT_NAMES
				const char* name = vatt.name.append("_%d", window->globalIndex).c_str();

				VkDebug::nameObject(renderer->device, reinterpret_cast<size_t>(attatchImage->vkItem), vk::DebugReportObjectTypeEXT::eImage, name);
#endif
				//if no array object for this window than make an empty one
				if (images.find(window) == images.end()) {
					images[window] = std::vector<Image*>();
					images.reserve(frameOptions.attatchments.size());
				}
				images[window].push_back(attatchImage);
			}

			// if the only attachment is the swap image and hense no empty array was created in the loop create one here
			if (frameOptions.attatchments.size() == 1) {
				images[window] = std::vector<Image*>();
			}
		}

		// make all frame buffers:
		{
			if (frameBuffers.size() == 0) {
				frameBuffers.resize(passOptions.size());
			}


			// make a frame buffer for each pass and if its the last pass add a ref to the window class
			for (size_t pass = 0; pass < passOptions.size(); pass++)
			{
				// the window->swapChainFramebuffers just has references to the same frame buffers in the last passes frame buffers of this class
				window->swapChainFramebuffers.resize(window->swapChainImageViews.size());


				// swapImage is the swap chain image index
				for (size_t swapImage = 0; swapImage < window->swapChainImageViews.size(); swapImage++) { 
					// see renderpass.cpp for info on order of attachments
					std::vector<vk::ImageView> attachments = {};
					attachments.reserve(passOptions[pass].attatchments.size());

					auto& winAttachments = images[window];

					// this is so that the indixies line up between local images and swap chain images stored in windows themselvs
					bool passedSwapImage = false;
					// a = pass local attach index
					for (size_t a = 0; a < passOptions[pass].attatchments.size(); a++) 
					{
						auto gloablAttachIndex = passAttachGlobalIndicies[pass][a];
						if (gloablAttachIndex == frameOptions.presentedAttachment) {

							auto imageView = window->swapChainImageViews[swapImage];

#if SR_VK_OBJECT_NAMES
							// if pass that will be presented
							if (pass == passOptions.size() - 1) {
								const char* name = frameOptions.attatchments[gloablAttachIndex].name.append("_%d", window->globalIndex).append("_%d", swapImage).c_str();
								VkDebug::nameObject(renderer->device, reinterpret_cast<size_t>(VkImageView(imageView)), imageView.debugReportObjectType, name);
							}
#endif

							attachments.push_back(imageView);
							passedSwapImage = true;
						}
						else {
							attachments.push_back(winAttachments[passedSwapImage ? gloablAttachIndex - 1 : gloablAttachIndex]->view);
						}
					}

					// make frame buffer for this swap chain view

					vk::FramebufferCreateInfo framebufferInfo{};
					framebufferInfo.renderPass = renderpasses[pass]->renderPass;
					framebufferInfo.attachmentCount = attachments.size();
					framebufferInfo.pAttachments = attachments.data();
					framebufferInfo.width = window->swapchainExtent.width;
					framebufferInfo.height = window->swapchainExtent.height;
					//todo: change for multi viewport rendering
					framebufferInfo.layers = 1;

					auto frameBuffer = renderer->device.createFramebuffer(framebufferInfo);

					if (pass == passOptions.size())
						window->swapChainFramebuffers[swapImage] = frameBuffer;

					//TODO: count is less eficient than could be for this
					if (frameBuffers[pass].count(window) == 0) {
						frameBuffers[pass][window] = {};
					}

					frameBuffers[pass][window].push_back(frameBuffer);
				}

			}
			
		}
	}

}
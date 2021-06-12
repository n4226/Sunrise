#pragma once

#include "RenderPassManager.h"


namespace sunrise {
	class Window;
	namespace gfx {


		/// <summary>
		/// Made to work in conjunction with GPU-Stages 
		/// Proper Use: create one and register it in one or many scene render coordinators
		/// Description: this class encapsolates the definition of an abstract render pass and its rener targets as well as the 
		/// functionality of creating frame buffers linked to the virtual attatchments for all unowned windows
		/// </summary>
		class SUNRISE_API ComposableRenderPass : public RenderPassManager
		{
		public:

			struct CreateOptions {

				struct VAttatchment {
					vk::AttachmentDescriptionFlags    flags = {};
					vk::Format                        format;
					vk::SampleCountFlagBits           samples = vk::SampleCountFlagBits::e1;
					vk::AttachmentLoadOp              loadOp;
					vk::AttachmentStoreOp             storeOp = vk::AttachmentStoreOp::eStore;
					vk::AttachmentLoadOp              stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
					vk::AttachmentStoreOp             stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
					// layout the image is in before render pass beggins
					vk::ImageLayout                   initialLayout;
					// layout to transition image to at start of renderPass (subpass but there is only one for now)
					vk::ImageLayout                   transitionalToStartLayout;
					vk::ImageLayout                   finalLayout;
				};


				std::vector<VAttatchment> attatchments;
			};

			ComposableRenderPass(Application& app, CreateOptions&& options);

			virtual void createMainRenderPass() override;
			virtual size_t getSubPassCount() override;

			//TODO: do proper delete of reasources
			~ComposableRenderPass();

		private:

			//remember many private properties of superclass are undefined as they are not used in this subclass

			std::vector<CreateOptions::VAttatchment*> vattatchments;

			std::unordered_map <Window*, std::vector<vk::Image>*> images;

			Application& app;

		};


	}
}
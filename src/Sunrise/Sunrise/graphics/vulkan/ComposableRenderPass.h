#pragma once

#include "RenderPassManager.h"


namespace sunrise {
	class Window;
	namespace gfx {
		class Renderer;
		class Image;

		/// <summary>
		/// Made to work in conjunction with GPU-Stages 
		/// Proper Use: create one and register it in one or many scene render coordinators
		/// Description: this class encapsolates the definition of an abstract render pass and its rener targets as well as the 
		/// functionality of creating frame buffers linked to the virtual attatchments for all unowned windows
		/// 
		/// TODO: add ability to specifuy and custimize image and image view creation with things like aliesing
		/// TODO: does not support multi viewport rendering yet
		/// </summary>
		class SUNRISE_API ComposableRenderPass : public RenderPassManager
		{
		public:

			struct CreateOptions {

				enum class AttatchmentType {
					Color, Depth
				};

				struct VAttatchment {

					AttatchmentType type;

					vk::AttachmentDescriptionFlags    flags = {};
					vk::Format                        format;
					vk::SampleCountFlagBits           samples = vk::SampleCountFlagBits::e1;
					vk::AttachmentLoadOp              loadOp;
					vk::AttachmentStoreOp             storeOp = vk::AttachmentStoreOp::eStore;
					vk::AttachmentLoadOp              stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
					vk::AttachmentStoreOp             stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
					/// layout the image is in before render pass beggins
					vk::ImageLayout                   initialLayout;
					/// layout to transition image to at start of renderPass (subpass but there is only one for now)
					vk::ImageLayout                   transitionalToAtStartLayout;
					vk::ImageLayout                   finalLayout;

					vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment;
				};


				std::vector<VAttatchment> attatchments;
				/// <summary>
				/// This is the attachment that will be linked to the swapchain and presented to screen
				/// default is -1 which is none
				/// </summary>
				signed int presentedAttachment = -1;
			};


			// one for every unowned window i.e one for every swapchain
			class SUNRISE_API DisplayImageHolder {

			};

			ComposableRenderPass(Renderer* renderer, CreateOptions&& options);

			virtual void createMainRenderPass() override;
			virtual size_t getSubPassCount() override;

			//TODO: do proper delete of reasources
			~ComposableRenderPass();

		private:

			void createWindowSpacificResources();
			void createWindowImagesAndFrameBuffer(Window* window);

			Renderer* renderer;
			CreateOptions options;

			//remember many private properties of superclass are undefined as they are not used in this subclass

			// index in this array is an attachment's vulkan attatchment index
			std::vector<CreateOptions::VAttatchment*> vattachments;

			// keys are unowned windows, every unowned window has its own swap chain and hense swap chain images and image views as well as concreate images for all other attatchemnts
			// the swap chain images and image views are still sotred in the window all other attatchments are stored here
			// in the future shared attatchment images bewtween multiple unowned winodows might be added - see todo at top of class about image creation
			std::unordered_map<Window*, std::vector<gfx::Image*>*> images;

			vk::AttachmentDescription descriptonFromVatt(CreateOptions::VAttatchment* vatt);

			/// <summary>
			/// the pointer givin myst be valid and registered in vattatchments vector
			/// </summary>
			/// <param name="vatt"></param>
			/// <returns></returns>
			vk::AttachmentReference referenceFromVatt(CreateOptions::VAttatchment* vatt);

		};


	}
}
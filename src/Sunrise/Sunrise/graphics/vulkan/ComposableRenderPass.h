#pragma once

#include "RenderPassManager.h"


namespace sunrise {
	class Window;
	namespace gfx {
		class Renderer;
		class Image;
		class Renderer;

		/// <summary>
		/// Made to work in conjunction with GPU-Stages 
		/// Proper Use: create one and register it in one or many scene render coordinators
		/// Description: this class encapsolates the definition of an abstract render pass and its rener targets as well as the 
		/// functionality of creating frame buffers linked to the virtual attatchments for all unowned windows
		/// 
		/// TODO: add ability to specifuy and custimize image and image view creation with things like aliesing
		/// TODO: does not support multi viewport rendering yet
		/// 
		/// helpful pages: https://www.saschawillems.de/blog/2018/07/19/vulkan-input-attachments-and-sub-passes/
		/// </summary>
		class SUNRISE_API ComposableRenderPass : public RenderPassManager
		{
		public:

			struct CreateOptions {

				enum class AttatchmentType {
					Color, Depth, Stencil
				};

				struct VAttatchment {

					AttatchmentType type = AttatchmentType::Color;

					vk::AttachmentDescriptionFlags    attachmentFlags = {};
					vk::Format                        format;
					vk::SampleCountFlagBits           samples = vk::SampleCountFlagBits::e1;
					vk::AttachmentLoadOp              loadOp;
					vk::AttachmentStoreOp             storeOp = vk::AttachmentStoreOp::eStore;
					vk::AttachmentLoadOp              stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
					vk::AttachmentStoreOp             stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
					/// layout the image is in before render pass beggins, layoiut undifined here means it can be anything
					vk::ImageLayout                   initialLayout;
					// for Render Coordinator ----
						//(layout to transition image to at start of the frame rendering 
						//-- if this is marked undefined than the image will not exist until a later render pass (if that is the implementation chosen)
					// --
					vk::ImageLayout                   transitionalToAtStartLayout;
					/// for Scene render coordinator - layout to transition image to at end of the frame rendering - else the layout at the end of the renderpass
					vk::ImageLayout                   finalLayout;

					vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment;

					/// <summary>
					/// only visable if load op is set to clear and type == color
					/// </summary>
					std::array<float, 4> clearColor = { 0.0f, 0.0f, 0.2f, 1.0f };
					/// <summary>
					/// only visivle if load op is set to clear and type == depth
					/// </summary>
					vk::ClearDepthStencilValue clearDepthStencil = { 1.f, 0};

					//debug options
					std::string name = "";
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

			ComposableRenderPass(Renderer* renderer, const CreateOptions& options);

			virtual void createMainRenderPass() override;
			virtual size_t getTotalAttatchmentCount() override;
			virtual size_t getColorAttatchmentCount() override;
			virtual size_t getSubPassCount() override;


			//TODO: do proper delete of reasources
			~ComposableRenderPass();


			const CreateOptions options;
		private:

			friend Renderer;

			Renderer* renderer;

			//remember many private properties of superclass are undefined as they are not used in this subclass

			// index in this array is an attachment's vulkan attatchment index
			std::vector<CreateOptions::VAttatchment*> vattachments;

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
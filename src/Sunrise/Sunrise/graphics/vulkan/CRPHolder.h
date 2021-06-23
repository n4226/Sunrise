#pragma once

#include "srpch.h"

#include "ComposableRenderPass.h"

namespace sunrise::gfx {

	/// <summary>
	/// 
	///  Made to work in conjunction with GPU-Stages 
	/// mangages one or more Composable Render passes for a scene render coordinator 
	/// weather the underling vulkan code is one rendxer pass with multiple subpasses or multiple renderPasses is an internal implimentation detail 
	/// which is subject to changse without notice and be different on different machenes but can be queried.
	/// 
	/// owns and create the images for the composable render pass(es)
	/// 
	/// //TODO might need to be rethought a little for compute
	/// 
	/// </summary>
	class CRPHolder
	{
	public:

		struct HolderOptions {
			size_t passes;
			/// <summary>
			/// outer array is passes inner is attachment;
			/// outer length must = passes - 1 (skipping first stage)
			/// 
			/// use vk::imageLayout::eUndefined to signify that that attachment is not used for a given pass
			/// </summary>
			std::vector< std::vector<vk::ImageLayout>> passStartLayout;
		};

		CRPHolder(const ComposableRenderPass::CreateOptions& wholeOptions,
			const HolderOptions& spacificOptions, Renderer* renderer);


		bool multipleSubPasses() const { return _multipleSubPasses; }

		/// <summary>
		/// returns the concreete image for a window of a "virtual" attachment index
		/// index must not be the index of the swapchain drawable as the render pass does not own those images.
		/// </summary>
		/// <param name="index"></param>
		/// <param name="window"></param>
		/// <returns></returns>
		Image* getImage(size_t index, Window* window);

	private:

		void createWindowSpacificResources();
		void createWindowImagesAndFrameBuffer(Window* window);

		// now only option is false
		bool _multipleSubPasses = false;

		//TODO: make this linked with render passes
		//TODO: make better api for multiviewport
		bool multiViewport = false;
		size_t multiViewCount = 1;

		Renderer* renderer;
		// total options
		ComposableRenderPass::CreateOptions frameOptions;
		std::vector<ComposableRenderPass::CreateOptions> passOptions;

		//remember many private properties of superclass are undefined as they are not used in this subclass

		// index in the outer array is the pass index
		// index in the inner array is an attachment's vulkan attatchment index
		std::vector<std::vector<ComposableRenderPass::CreateOptions::VAttatchment*>> vattachments{};

		// keys are unowned windows, every unowned window has its own swap chain and hense swap chain images and image views as well as concreate images for all other attatchemnts
		// the swap chain images and image views are still sotred in the window all other attatchments are stored here
		// in the future shared attatchment images bewtween multiple unowned winodows might be added - see todo at top of class about image creation
		std::unordered_map<Window*, std::vector<gfx::Image*>> images{};


		std::vector<ComposableRenderPass*> renderpasses{};

		/// <summary>
		/// one for each supbass and onowned window, the last one is also referenced by the respective window
		/// outer array is pass index, inner array is window index (filtered to onowned windows only)
		/// </summary>
		std::vector<std::vector<vk::Framebuffer*>> frameBuffers{};

	};

}
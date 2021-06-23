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
	/// you defin logical passes when creating the object which are then tunred into vk subpases or renderpasses
	/// 
	/// //TODO might need to be rethought a little for compute
	/// 
	/// Asumptions:
	/// 
	/// last pass renders to a swap image that will then be presented
	/// 
	/// </summary>
	class SUNRISE_API CRPHolder
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
			std::vector<std::vector<vk::ImageLayout>> passStartLayout;
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
		Image* getImage(size_t globalIndex, Window* window);

		vk::Framebuffer getFrameBuffer(size_t pass, Window* window, size_t surfaceIndex);

		/// <summary>
		/// returns the CRP and the supbass within it to use for a "logical pass" index (0..<passCount())
		/// </summary>
		/// <param name="pass"></param>
		/// <returns>the CRP object and the pass within it</returns>
		std::pair<ComposableRenderPass*, size_t> renderPass(size_t pass);

		bool arePassesDifferentRenderPasses(size_t pass1, size_t pass2) { return pass1 != pass2; }

		size_t passCount() { return holderOptions.passes; }

	private:

		void createPasses(const sunrise::gfx::ComposableRenderPass::CreateOptions& wholeOptions, const sunrise::gfx::CRPHolder::HolderOptions& spacificOptions);

		void createWindowSpacificResources();
		void createWindowImagesAndFrameBuffer(Window* window);

		// now only option is false
		bool _multipleSubPasses = false;

		//TODO: make this linked with render passes
		//TODO: make better api for multiviewport
		bool multiViewport = false;
		size_t multiViewCount = 1;

		Renderer* renderer;

		HolderOptions holderOptions;
		// total options
		ComposableRenderPass::CreateOptions frameOptions;
		std::vector<ComposableRenderPass::CreateOptions> passOptions{};
		/// <summary>
		/// outer = passes
		/// inner = attachments of the pass
		/// value = indicy into global frame options attachments array
		/// </summary>
		std::vector<std::vector<size_t>> passAttachGlobalIndicies{};


		// keys are unowned windows, every unowned window has its own swap chain and hense swap chain images and image views as well as concreate images for all other attatchemnts
		// the swap chain images and image views are still sotred in the window all other attatchments are stored here
		// in the future shared attatchment images bewtween multiple unowned winodows might be added - see todo at top of class about image creation
		std::unordered_map<Window*, std::vector<gfx::Image*>> images{};


		std::vector<ComposableRenderPass*> renderpasses{};

		/// <summary>
		/// one for each supbass and onowned window, the last one is also referenced by the respective window
		/// outer array is pass index, map is the window, inner array is window surface index
		/// </summary>
		std::vector<std::unordered_map<Window*, std::vector<vk::Framebuffer>>> frameBuffers{};

	};

}
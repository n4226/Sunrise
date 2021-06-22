#pragma once

#include "srpch.h"

namespace sunrise::gfx {

	/// <summary>
	/// mangages one or more Composable Render passes for a scene render coordinator 
	/// weather the underling vulkan code is one rendxer pass with multiple subpasses or multiple renderPasses is an internal implimentation detail 
	/// which is subject to change without notice and be different on different machenes but can be queried.
	/// </summary>
	class CRPHolder
	{
	public:
		CRPHolder();


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

		// now only option is false
		bool _multipleSubPasses = false;


	};

}
#pragma once

#include "srpch.h"
#include "RenderTarget.h"

namespace sunrise::gfx {

	/// <summary>
	/// ads swapchain items and other specifics for windows
	/// </summary>
	class WindowRenderTarget: public RenderTerget
	{
	public:



	protected:

		vk::SwapchainKHR swapChain = nullptr;


	private:
	};

}
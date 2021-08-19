#pragma once

#include "srpch.h"
#include "../../graphics/vulkan/renderer/RenderSystem.h"

namespace sunrise {

	class WindowCameraController: public System
	{
	public:

		void update() override;

	};

}

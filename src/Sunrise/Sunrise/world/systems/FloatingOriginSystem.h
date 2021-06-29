#pragma once

#include "srpch.h"
#include "../../graphics/vulkan/renderer/RenderSystem.h"

namespace sunrise {


	class FloatingOriginSystem : public System
	{
	public:
		void update() override;

	};

}
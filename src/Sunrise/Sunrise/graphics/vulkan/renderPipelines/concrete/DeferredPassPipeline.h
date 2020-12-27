#pragma once

#include "srpch.h"
#include "../GraphicsPipeline.h"

namespace sunrise::gfx {


	class DeferredPass : public GraphicsPipeline
	{
	public:
		//GBufferPass(vk::Device device);
		//~GBufferPass();

		void createPipeline() override;

		using GraphicsPipeline::GraphicsPipeline;

	};

}
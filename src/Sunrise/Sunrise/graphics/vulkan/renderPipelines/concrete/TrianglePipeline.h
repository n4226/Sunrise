#pragma once

#include "../GraphicsPipeline.h"

namespace sunrise::gfx {

	class TrianglePipeline : public GraphicsPipeline
	{
	public:
		using GraphicsPipeline::GraphicsPipeline;
		void createPipeline() override;
	protected:
	};

}
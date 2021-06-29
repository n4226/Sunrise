#pragma once

#include "srpch.h"
#include "../../../ComposableRenderPass.h"
#include "../../../renderPipelines/GraphicsPipeline.h"

namespace sunrise {

	/// <summary>
	/// this is an abstract comman "deffered" pieleine and gpu stage to be modularaly used
	/// </summary>
	class DeferredPipeline: public gfx::VirtualGraphicsPipeline
	{
	public:
		using gfx::VirtualGraphicsPipeline::VirtualGraphicsPipeline;

	protected:

		virtual gfx::GraphicsPipelineOptions makeDeff() override;


	};

	extern DeferredPipeline* deferredPipeline;

}
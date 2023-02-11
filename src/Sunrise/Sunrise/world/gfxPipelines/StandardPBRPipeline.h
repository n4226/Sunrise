#pragma once

#include "srpch.h"
#include "../../graphics/vulkan/ComposableRenderPass.h"
#include "../../graphics/vulkan/renderPipelines/GraphicsPipeline.h"

namespace sunrise {

	/// <summary>
	/// this is an abstract comman "deffered" pieleine and gpu stage to be modularaly used
	/// </summary>
	class StandardPBRPipeline : public gfx::VirtualGraphicsPipeline
	{
	public:
		using gfx::VirtualGraphicsPipeline::VirtualGraphicsPipeline;

	protected:

		gfx::GraphicsPipelineOptions makeDeff() override;


	};


	//TOOD: find beter way for this
	gfx::GraphicsPipelineOptions StandardPBRDef();

	extern StandardPBRPipeline* standardPBRPipeline;

}

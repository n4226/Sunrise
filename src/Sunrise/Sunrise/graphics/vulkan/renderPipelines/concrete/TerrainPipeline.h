#pragma once

#include "../PipelineCreator.h"

class TerrainPipeline: public GraphicsPipeline
{
public:
	using GraphicsPipeline::GraphicsPipeline;
	void createPipeline() override;
protected:
};


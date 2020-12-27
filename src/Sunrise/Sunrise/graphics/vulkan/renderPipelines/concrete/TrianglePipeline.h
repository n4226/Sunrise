#pragma once

#include "../PipelineCreator.h"

class TrianglePipeline : public GraphicsPipeline
{
public:
	using GraphicsPipeline::GraphicsPipeline;
	void createPipeline() override;
protected:
};


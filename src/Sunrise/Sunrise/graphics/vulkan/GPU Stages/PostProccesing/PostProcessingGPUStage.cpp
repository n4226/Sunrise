#include "srpch.h"
#include "PostProcessingGPUStage.h"

namespace sunrise::gfx {

	PostProcessingGPUStage::PostProcessingGPUStage(SceneRenderCoordinator* coord, const std::vector<PostProcessingEffect>& effects)
		: GPURenderStage(coord,"Post Processing Stage")
	{

	}

	void PostProcessingGPUStage::setup()
	{
	}

	void PostProcessingGPUStage::cleanup()
	{
	}

	vk::CommandBuffer* PostProcessingGPUStage::encode(RunOptions options)
	{
		return nullptr;
	}

}

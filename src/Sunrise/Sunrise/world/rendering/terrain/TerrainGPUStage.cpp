#include "srpch.h"
#include "TerrainGPUStage.h"

namespace sunrise {

	TerrainGPUStage::TerrainGPUStage(Application& app)
		: GPURenderStage(app,"World Terrain Render Stage")
	{

	}

	void TerrainGPUStage::setup()
	{
	}

	void TerrainGPUStage::cleanup()
	{
	}

	vk::CommandBuffer* TerrainGPUStage::encode(RunOptions options)
	{

		auto buff = selectAndSetupCommandBuff(options);


		buff->end();

		return buff;
	}

}
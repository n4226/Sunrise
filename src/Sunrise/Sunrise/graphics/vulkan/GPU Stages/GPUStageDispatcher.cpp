#include "srpch.h"
#include "GPUStageDispatcher.h"

namespace sunrise::gfx {


	GPUStageDispatcher::GPUStageDispatcher()
	{
	}

	GPUStageDispatcher::~GPUStageDispatcher()
	{
	}

	void GPUStageDispatcher::registerStage(GPUStage* stage, std::vector<GPUStage>&& runDependencies, std::vector<GPUStage>&& encodeDependencies)
	{
	}

	void GPUStageDispatcher::unregisterStage(GPUStage* stage)
	{
	}

	void GPUStageDispatcher::unregisterAllStages()
	{
	}

}
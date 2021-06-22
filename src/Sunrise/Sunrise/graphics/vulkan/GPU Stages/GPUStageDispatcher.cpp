#include "srpch.h"
#include "GPUStageDispatcher.h"
#include "GpuStage.h"

namespace sunrise::gfx {


	GPUStageDispatcher::GPUStageDispatcher()
	{
	}

	GPUStageDispatcher::~GPUStageDispatcher()
	{
	}

	void GPUStageDispatcher::registerStage(GPUStage* stage, std::vector<GPUStage*>&& runDependencies, std::vector<DependencyOptions>&& runDependencyOptions, std::vector<GPUStage*>&& encodeDependencies)
	{
#if SR_ENABLE_PRECONDITION_CHECKS
		// this is to make sure a stage is not registered more than once
		assert(individualRunDependencies.count(stage) == 0);
		assert(individualRunDependencyOptoins.count(stage) == 0);

		assert(!graphBuilt);
#endif

		if (!stage->_setup) {
			stage->setup();
			stage->_setup = true;
		}

		for (auto& dep : runDependencyOptions) {
			if (dep.newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
				SR_CORE_INFO("Render Coordinator creating multiple renderpasses");
				multipleRenderPasses = true;
			}
		}

		individualRunDependencies[stage] = std::move(runDependencies);
		individualRunDependencyOptoins[stage] = std::move(runDependencyOptions);

	}


	void GPUStageDispatcher::unregisterStage(GPUStage* stage)
	{
#if SR_ENABLE_PRECONDITION_CHECKS
		for (auto& s : individualRunDependencies) {
			assert(std::count(s.second.begin(),s.second.end(),stage) == 0);
		}
#endif

		individualRunDependencies.erase(stage);

	}

	void GPUStageDispatcher::unregisterAllStages()
	{
		individualRunDependencies = {};
	}

}
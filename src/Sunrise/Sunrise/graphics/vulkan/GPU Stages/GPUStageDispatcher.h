#pragma once

#include "srpch.h"


namespace sunrise::gfx {

	class GPUStage;

	class SUNRISE_API GPUStageDispatcher
	{
	public:

		GPUStageDispatcher();
		virtual ~GPUStageDispatcher();
		

		/// <summary>
		/// must be called in order to register the stage with the engine to start running as part of render loop.
		/// Imdeiatly after initilization is useually the best time to call this method.
		/// </summary>
		/// <param name="runDependencies">stages that must complete before this stage exicutes each </param>
		/// <param name="encodeDependencies"></param>
		void registerStage(GPUStage* stage, std::vector<GPUStage>&& runDependencies, std::vector<GPUStage>&& encodeDependencies);

		/// <summary>
		/// removes a stage from the graph
		/// MUST NOT BE THE DEPENDACY OF ANY REGISTERED STAGE or runtime error (soon)
		/// </summary>
		/// <param name="stage"></param>
		void unregisterStage(GPUStage* stage);
		void unregisterAllStages();


	};

}
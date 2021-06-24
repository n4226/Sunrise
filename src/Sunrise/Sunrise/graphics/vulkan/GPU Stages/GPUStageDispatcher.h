#pragma once

#include "srpch.h"


namespace sunrise::gfx {

	class GPUStage;

	class SUNRISE_API GPUStageDispatcher
	{
	public:


		struct SUNRISE_API DependencyOptions {
			size_t resourceIndex;
			vk::ImageLayout newLayout;
			vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eLoad;
			vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore;
			vk::AttachmentLoadOp stencilLoadOp   = vk::AttachmentLoadOp::eDontCare;
			vk::AttachmentStoreOp stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		};

		GPUStageDispatcher();
		virtual ~GPUStageDispatcher();
		

		/// <summary>
		/// must be called in order to register the stage with the engine to start running as part of render loop.
		/// Imdeiatly after initilization is useually the best time to call this method.
		/// 
		/// Precondition: stage must not already be registered
		/// 
		/// TODO: look into makeing these vector parameter types into temporaries again eg std::move or &&
		/// </summary>
		/// <param name="runDependencies">stages that must complete before this stage exicutes each </param>
		/// <param name="runDependencyOptions"> not one to one related to run dependancies vector, each option is about a resource</param>
		/// <param name="encodeDependencies"></param>
		void registerStage(GPUStage* stage, std::vector<GPUStage*>&& runDependencies, std::vector<DependencyOptions>&& runDependencyOptions, std::vector<GPUStage*>&& encodeDependencies);

		/// <summary>
		/// removes a stage from the graph
		/// MUST NOT BE THE DEPENDACY OF ANY REGISTERED STAGE or runtime error (soon)
		/// 
		/// currently this class does not manage memory of any stages this must be done externaly
		/// eventually this could be handled by calling this method off all dispatchers a stage is registered in its initilizer
		/// </summary>
		/// <param name="stage"></param>
		void unregisterStage(GPUStage* stage);
		void unregisterAllStages();

		// runing
		///////////////////////// this functionality was moved to the render coordinator class
		/*/// <summary>
		/// righ now this will only encode on one queue
		/// </summary>
		/// <param name="firstLevelCMDBuffer"></param>
		void encodeAll(vk::CommandBuffer firstLevelCMDBuffer);*/

	protected:

		bool multipleRenderPasses = false;
		bool graphBuilt = false;

		/// <summary>
		/// key = a stage
		/// value = all of its dependencies
		/// </summary>
		std::unordered_map<GPUStage*, std::vector<GPUStage*>> individualRunDependencies = {};

		/// <summary>
		/// key = a stage
		/// value = all of its dependency options
		/// </summary>
		std::unordered_map<GPUStage*, std::vector<DependencyOptions>> individualRunDependencyOptoins = {};



		//TODO: for now encoding dependencies are not suported all stages are assumed to not care what order they are encoded in

		/// <summary>
		/// key = a stage
		/// value = all of its dependencies
		/// </summary>
		//std::unordered_map<GPUStage*, std::vector<GPUStage*>> individualEncodeDependencies = {};



	};

}
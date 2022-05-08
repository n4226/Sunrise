#pragma once

#include "srpch.h"
#include "GpuStage.h"
#include "../renderPipelines/GraphicsPipeline.h"

namespace sunrise::gfx {


	class SUNRISE_API GPURenderStage : public GPUStage
	{
	public:
		GPURenderStage(SceneRenderCoordinator* coord,std::string&& name, bool useInternalresources = true);
		virtual ~GPURenderStage();


	protected:

		void createRequiredRenderResources();

		void setPipeline(const sunrise::Window& window, vk::CommandBuffer buffer, VirtualGraphicsPipeline* pipeline);
		GraphicsPipeline* getConcretePipeline(const sunrise::Window& window, VirtualGraphicsPipeline* pipeline);


		// Inherited via GPUStage
		// 
		//called once 
		virtual void setup() override = 0;
		virtual void cleanup() override = 0;
		// called every frame
		virtual vk::CommandBuffer* encode(RunOptions options) override = 0;

		void registerPipeline(VirtualGraphicsPipeline* virtualPipe);


		bool useInternalresources;
	};


}
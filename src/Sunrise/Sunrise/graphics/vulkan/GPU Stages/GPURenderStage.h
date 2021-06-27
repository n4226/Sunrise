#pragma once

#include "srpch.h"
#include "GpuStage.h"
#include "../renderPipelines/GraphicsPipeline.h"

namespace sunrise::gfx {


	class SUNRISE_API GPURenderStage : public GPUStage
	{
	public:
		GPURenderStage(Application& app,std::string&& name);
		~GPURenderStage();

		void createRequiredRenderResources();

	protected:


		// Render Resources

		/// <summary>
		/// one for each drawable
		/// </summary>
		std::vector<std::vector<vk::CommandPool  >> cmdBufferPools;
		std::vector<std::vector<vk::CommandBuffer>> commandBuffers;


		/// <summary>
		/// configrues and begins encoding in the buffer
		/// </summary>
		/// <param name="subpass"></param>
		/// <param name="window"></param>
		/// <returns></returns>
		vk::CommandBuffer* selectAndSetupCommandBuff(RunOptions options);

		void setPipeline(sunrise::Window& window, vk::CommandBuffer buffer, VirtualGraphicsPipeline* pipeline);
		GraphicsPipeline* getConcretePipeline(sunrise::Window& window, VirtualGraphicsPipeline* pipeline);


		// Inherited via GPUStage
		// 
		//called once 
		virtual void setup() override = 0;
		virtual void cleanup() override = 0;
		// called every frame
		virtual vk::CommandBuffer* encode(RunOptions options) override = 0;

	};


}
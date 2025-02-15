#pragma once

#include "srpch.h"
#include "GpuStage.h"

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
		vk::CommandBuffer* selectAndSetupCommandBuff(uint32_t subpass, sunrise::Window& window);


		// Inherited via GPUStage
		virtual vk::CommandBuffer* encode(uint32_t subpass, sunrise::Window& window) override = 0;

	};


}
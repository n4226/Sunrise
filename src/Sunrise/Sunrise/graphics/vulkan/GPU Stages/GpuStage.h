#pragma once

#include "srpch.h"

//#include "Sunrise/Sunrise/core/Application.h"

namespace sunrise {
	class Window;
	namespace gfx {

		class GPUStageDispatcher;

		/// <summary>
		/// AN abstract base clase for the GPU Stage System.
		/// 
		/// GPU Stages allow for dependancy graphs to be bult for different types of gpu work which cna trhen be efficieanlty scheduled
		/// - this is how most GPU work is submitted to the Sunrise Engine
		/// 
		/// </summary>
		class SUNRISE_API GPUStage
		{
		public:

			GPUStage();
			virtual ~GPUStage();



			//TODO: right now all stages encode into secondar command buffs but this could change

			/// <summary>
			/// 
			/// </summary>
			/// <returns></returns>
			virtual vk::CommandBuffer* encode(uint32_t subpass, Window& window) = 0;

		protected:

		};

	}
}
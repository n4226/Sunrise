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

			GPUStage(Application& app,std::string&& name);

			virtual ~GPUStage();



			//called once 
			virtual void setup() = 0;
			virtual void cleanup() = 0;

			//TODO: right now all stages encode into secondar command buffs but this could change

			/// <summary>
			/// 
			/// </summary>
			/// <returns></returns>
			virtual vk::CommandBuffer* encode(uint32_t subpass, Window& window) = 0;

#if SR_LOGGING
			std::string name;
#endif

		//protected:

			Application& app;

		protected:
			friend GPUStageDispatcher;

			bool _setup = false;

		};

	}
}
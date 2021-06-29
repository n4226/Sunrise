#pragma once

#include "srpch.h"

//#include "Sunrise/Sunrise/core/Application.h"
#include "../renderer/Renderer.h"

namespace sunrise {
	class Window;
	class Scene;
	namespace gfx {

		class GPUStageDispatcher;
		class SceneRenderCoordinator;

		/// <summary>
		/// AN abstract base clase for the GPU Stage System.
		/// 
		/// GPU Stages allow for dependancy graphs to be bult for different types of gpu work which cna trhen be efficieanlty scheduled
		/// - this is how most GPU work is submitted to the Sunrise Engine
		/// 
		/// </summary>
		class SUNRISE_API GPUStage: public RenderResourceTracker
		{
		public:

			GPUStage(SceneRenderCoordinator* coord,std::string&& name);

			virtual ~GPUStage();



			//called once 
			virtual void setup() = 0; 
			/// <summary>
			/// when this is called getConcretePipeline(...) can be used
			/// </summary>
			virtual void lateSetup() {}
			virtual void cleanup() = 0;

			//TODO: right now all stages encode into secondar command buffs but this could change

			struct RunOptions {
				Scene* scene;
				SceneRenderCoordinator* coordinator;
				uint32_t pass;
				Window& window;
			};

			/// <summary>
			/// 
			/// </summary>
			/// <returns></returns>
			virtual vk::CommandBuffer* encode(RunOptions options) = 0;

#if SR_LOGGING
			std::string name;
#endif

		//protected:

			Application& app;
			SceneRenderCoordinator* coord;

		protected:
			friend GPUStageDispatcher;

			bool _setup = false;

		};

	}
}
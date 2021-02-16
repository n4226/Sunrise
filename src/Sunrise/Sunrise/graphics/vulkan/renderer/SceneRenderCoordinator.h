#pragma once

#include "srpch.h"
#include "Renderer.h"

#include "Sunrise/Sunrise/graphics/vulkan/GPU Stages/GPUStageDispatcher.h"

namespace sunrise {

	class Scene;
	class Application;
	class Window;

	namespace gfx {
		class Renderer;

		class SUNRISE_API SceneRenderCoordinator : public GPUStageDispatcher
		{
		public:

			SceneRenderCoordinator(Scene* scene);
			virtual ~SceneRenderCoordinator();

			/// <summary>
			/// called shortly after initilization 
			/// this is where the gpu stages should be registered
			/// </summary>
			virtual void createPasses();

			/// <summary>
			///  righ now this will only encode on one queue
			/// </summary>
			/// <param name="renderer"></param>
			/// <param name="firstLevelCMDBuffer"></param>
			/// <param name="frameID"></param>
			/// <param name="window"></param>
			void encodePassesForFrame(Renderer* renderer,vk::CommandBuffer firstLevelCMDBuffer,size_t frameID, Window& window);

			Scene* const scene;
			Application& app;
		private:

			GPUStage* lastStage;

		};


	}
}
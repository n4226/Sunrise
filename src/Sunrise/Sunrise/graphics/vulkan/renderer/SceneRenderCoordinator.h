#pragma once

#include "srpch.h"
#include "Renderer.h"

#include "Sunrise/Sunrise/graphics/vulkan/GPU Stages/GPUStageDispatcher.h"
#include "Sunrise/Sunrise/graphics/vulkan/ComposableRenderPass.h"

namespace sunrise {

	class Scene;
	class Application;
	class Window;

	namespace gfx {
		class Renderer;
		class VirtualGraphicsPipeline;

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
			///  right now this will only encode on one queue
			/// </summary>
			/// <param name="renderer"></param>
			/// <param name="firstLevelCMDBuffer"></param>
			/// <param name="frameID"></param>
			/// <param name="window"></param>
			void encodePassesForFrame(Renderer* renderer,vk::CommandBuffer firstLevelCMDBuffer,size_t frameID, Window& window);

			Scene* const scene;
			Application& app;

			/// <summary>
			/// Registers the pipeline definition to be instantiuated by all windows rendering the scene
			/// currently only pipelines registered before the createPasses funciton exits are garenteed to be instantiated by all windows.
			/// </summary>
			/// <param name="virtualPipe"></param>
			void registerPipeline(VirtualGraphicsPipeline* virtualPipe);

			// TOOD right now just one per scene but for multi-gpu there will need to be one per scene and device so this will need to be an array
			ComposableRenderPass* sceneRenderpass;
		protected:
			friend Window;
			friend Application;
			std::vector<VirtualGraphicsPipeline*> registeredPipes;

			/// <summary>
			/// his is guaranteed to be called before the createPasses funciton is called 
			/// </summary>
			/// <param name="swapChainFormat">the format the display(s) are expecting, with multiuple windows if formats are different value is undifined</param>
			/// <returns></returns>
			virtual ComposableRenderPass::CreateOptions renderpassConfig(vk::Format swapChainFormat);


			void loadOrGetRegisteredPipesInAllWindows();

			void createRenderpasses();


		private:

			GPUStage* lastStage = nullptr;

		};


	}
}


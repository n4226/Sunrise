#pragma once

#include "srpch.h"
#include "Renderer.h"

#include "Sunrise/Sunrise/graphics/vulkan/GPU Stages/GPUStageDispatcher.h"
#include "Sunrise/Sunrise/graphics/vulkan/CRPHolder.h"

namespace sunrise {

	class Scene;
	class Application;
	class Window;

	namespace gfx {
		class Renderer;
		class VirtualGraphicsPipeline;

		//TODO: figure out how CRV and Gpu-Stages will work with multi gpu
		/// <summary>
		/// make sure that calls inside stages encoding functions are syubncronizsed for objects which require that since calls can be parallelized
		/// 
		/// 
		/// TODO: compute not supported yet
		/// TODO: multi gpu not implemented work will need to be done
		/// TODO: encoding of stages with proper syncronosation for many passes not fully implrmented 
		/// - this can be done with events for more control and less stopping of gpu execution
		/// 
		/// Proper Use:
		/// 
		/// 1. create a subclass of this class
		/// 
		/// 2. overide createPasses():
		///	2.1. call registerPipeline(VirtualGraphicsPipeline* virtualPipe) for each pipeline that will be used
		/// 2.2. call registerStage(GPUStage* stage ...) for each stage
		/// 2.3. call setLastPass(GPUStage* lastStage) for the pass which should be the last to execute before the presentation to the screen
		/// 
		/// 
		/// 
		/// might have to use multiple render passes for passes to work right - good site about vk render passes: https://stackoverflow.com/questions/48300046/why-do-we-need-multiple-render-passes-and-subpasses
		/// 
		/// //TODO: LIMITATIONS: only supports spacific image layout transtions currently see build graph comments
		/// 
		/// when defining a stage dependancy option which will cause a new pass to be needed, the layout of all used attachments must be set
		///  or they will be unavailable in the pass
		/// 
		/// //new GPU-Stage system idea:
		/// 
		/// TODO: syncronosation see below:
		/// TODO: because there might/will be multiple render passes, much like the sugestion in Metal, we could start frame running witnout aquisiiton of swap chain image since it is only needed at the very end of the frame 
		/// 
		/// </summary>
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
			/// must only be called once for a given pipeline and stage
			/// 
			/// the same virtual pipe can be registered for multiple stages but //TODO: figure out if this is okay
			/// </summary>
			/// <param name="virtualPipe"></param>
			void registerPipeline(VirtualGraphicsPipeline* virtualPipe, GPUStage* forStage);

			/// <summary>
			/// must be called once before before the createPasses funciton exits
			/// if called multiple times results are undefined
			/// </summary>
			/// <param name="lastStage"></param>
			void setLastPass(GPUStage* lastStage);

			// TOOD right now just one per scene but for multi-gpu there will need to be one per scene and device so this will need to be an array
			std::vector<CRPHolder*> sceneRenderpassHolders;

			ComposableRenderPass::CreateOptions wholeFrameRenderPassOptions;

		protected:
			friend Window;
			friend Application;
			std::vector<std::pair<VirtualGraphicsPipeline*, GPUStage*>> registeredPipes{};
			ComposableRenderPass::CreateOptions __tempWholeFrameRenderPassOptions;

			

			/// <summary>
			/// his is guaranteed to be called after the createPasses funciton is called 
			/// </summary>
			/// <param name="swapChainFormat">the format the display(s) are expecting, with multiuple windows if formats are different value is undifined</param>
			/// <returns></returns>
			virtual ComposableRenderPass::CreateOptions renderpassConfig(vk::Format swapChainFormat);

			/// <summary>
			/// 
			/// </summary>
			void buildGraph();

		private:

			void loadOrGetRegisteredPipesInAllWindows();

			/// <summary>
			/// called by buildGraph()
			/// </summary>
			void createRenderpasses(const CRPHolder::HolderOptions& holderOptions);

			GPUStage* lastStage = nullptr;

			/// <summary>
			/// once graph is built, this is the stages to execute in the proper order;
			/// </summary>
			std::vector<GPUStage*> stagesInOrder = {};

			std::unordered_map<GPUStage*,size_t> passForStage = {};

			// during frame encoding:

			void startNewPass(int64_t pass, Window& window, vk::CommandBuffer firstLevelCMDBuffer);



		};


	}
}


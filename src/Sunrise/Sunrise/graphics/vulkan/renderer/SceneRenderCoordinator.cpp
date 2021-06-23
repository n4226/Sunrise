#include "srpch.h"
#include "SceneRenderCoordinator.h"

#include "Sunrise/Sunrise/core/Application.h"
#include "Sunrise/Sunrise/core/Window.h"
#include "Sunrise/Sunrise/scene/Scene.h"
#include "Sunrise/Sunrise/graphics/vulkan/GPU Stages/GpuStage.h"
#include "Sunrise/Sunrise/graphics/vulkan/renderer/Renderer.h"
#include "Sunrise/Sunrise/graphics/vulkan/renderPipelines/GraphicsPipeline.h"

namespace sunrise::gfx {


	SceneRenderCoordinator::SceneRenderCoordinator(Scene* scene)
		: scene(scene), app(scene->app)
	{

	}

	SceneRenderCoordinator::~SceneRenderCoordinator()
	{

	}

	void SceneRenderCoordinator::createPasses()
	{

	}


	void SceneRenderCoordinator::registerPipeline(VirtualGraphicsPipeline* virtualPipe)
	{
		virtualPipe->create();
		registeredPipes.push_back(virtualPipe);
	}

	void SceneRenderCoordinator::setLastPass(GPUStage* lastStage)
	{
		this->lastStage = lastStage;
	}

	ComposableRenderPass::CreateOptions SceneRenderCoordinator::renderpassConfig(vk::Format swapChainFormat)
	{
		//This will not create a valid render pass
		return ComposableRenderPass::CreateOptions();
	}

	void SceneRenderCoordinator::buildGraph()
	{
		/* Steps

		TODO: add mutli queue support
		TODO: move encodeing order arrond or posbly use gpu events as synchronosation to keep gpu feed while waiting for dependancies

		traverse dependancies into a lenear list to encode --- right now all of the encoding will be single threaded



		https://visualgo.net/en/dfsbfs
		not as good as first website i had for graphs
		*/


		//TODO: calculate this upfront for performance

		//std::unordered_set<GPUStage*> stages = {};

		/*for (auto& s : individualRunDependencies) {

			if (stages.count(s.first) == 0) {
				if (s.second.size() == 0)
					stages.insert(s.first);
				else {
					bool good = false;
					for (auto& dep : s.second) {

					}
				}
			}

		}*/

		// Breath First Search of all dependancies

		
		std::vector<GPUStage*> stagesNodeQueue = {};
		std::unordered_set<GPUStage*> visited = {};

		//for (auto node : individualRunDependencies) {
		//	if (node.second.size() == 0) {
		//		stagesInOrder.push_back(node);
		//	}
		//}

#if SR_ENABLE_PRECONDITION_CHECKS
		//making sure last stage is properly registered
		SR_ASSERT(individualRunDependencies.find(lastStage) != individualRunDependencies.end());
#endif

		GPUStage* firstNode = lastStage;

		//todo set first node correctly
		stagesNodeQueue.push_back(firstNode);
		stagesInOrder.push_back(firstNode);
		// dont rememb er how this is supposed to work so overidding this for now --- this was the active code
		while (!stagesNodeQueue.empty()) {
			auto node = stagesNodeQueue[stagesNodeQueue.size() - 1];
			stagesNodeQueue.pop_back();
			if (visited.count(node) == 0) {
				visited.insert(node);
				stagesNodeQueue.push_back(node);

				for (auto dependency : individualRunDependencies[node]) {
					if (visited.count(dependency) == 0) {
						stagesNodeQueue.push_back(dependency);
						stagesInOrder.push_back(dependency);
					}
				}
			}
		}
		std::reverse(stagesInOrder.begin(), stagesInOrder.end());

		//todo: fixthis
		//std::vector<GPUStage*> keys;
		//keys.reserve(individualRunDependencies.size());
		////std::vector<Val> vals;
		////vals.reserve(map.size());

		//for (auto kv : individualRunDependencies) {
		//	keys.push_back(kv.first);
		//	//vals.push_back(kv.second);
		//}
		//stagesInOrder.push_back(keys[0]);

		//SR_CORE_TRACE("{}", stagesInOrder.size());

#if SR_ENABLE_PRECONDITION_CHECKS
		for (auto stage : stagesInOrder) {

			//making sure each stage is properly registered
			SR_ASSERT(individualRunDependencies.find(stage) != individualRunDependencies.end());
		}
#endif

		// create atachment options
		{
			vk::Format swapChainFormat = vk::Format::eUndefined;

			//TODO check if windows array is jsut unowned windows
			for (auto win : app.renderers[0]->windows) {
				vk::Format winFormat = static_cast<vk::Format>(win->swapchainImageFormat);
				SR_ASSERT(!(swapChainFormat != vk::Format::eUndefined && winFormat != swapChainFormat));
				swapChainFormat = winFormat;
			}

			auto config = renderpassConfig(swapChainFormat);
			SR_ASSERT(config.attatchments.size() > 0);
			wholeFrameRenderPassOptions = config;
			__tempWholeFrameRenderPassOptions = std::move(config);
		}


		CRPHolder::HolderOptions holderOptions{};
		// see if complex renderpass creation is required
		if (multipleRenderPasses) {

			holderOptions.passes = 1;

			for (size_t stageIndex = 0; stageIndex < stagesInOrder.size(); stageIndex++)
			{
				auto stage = stagesInOrder[stageIndex];
				if (individualRunDependencyOptoins.count(stage) > 0) {
					auto& options = individualRunDependencyOptoins[stage];
					
					

					for (auto& p : options) {
						bool addedPassForOption = false;

						SR_ASSERT(p.newLayout == vk::ImageLayout::eShaderReadOnlyOptimal
							|| p.newLayout == vk::ImageLayout::eUndefined);
						//TODO right now only checking for colorAttachment to and from eShaderReadOnlyOptimal layouts
						if (p.newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
							// the resource index of the dependancy option must be valid
							SR_ASSERT(p.resourceIndex < wholeFrameRenderPassOptions.attatchments.size());

							// the first stage can not have a layout transtion as 
							// this should be defined in the attachment configuration in the renderpassConfig() function
							SR_ASSERT(stage != stagesInOrder[0]);

							if (!addedPassForOption) {
								// its ok to do stageIndex - 1 becuase asserted two lines up that its not the first stage
								passForStage[stagesInOrder[stageIndex - 1]] = holderOptions.passes - 1;
								holderOptions.passes += 1;
								addedPassForOption = true;
							}

							// create empty transitions for all assets 
							if (holderOptions.passStartLayout.size() < holderOptions.passes - 1) {
								holderOptions.passStartLayout.push_back({});
								holderOptions.passStartLayout[holderOptions.passStartLayout.size() - 1].resize(wholeFrameRenderPassOptions.attatchments.size());
								for (auto& layout : holderOptions.passStartLayout[holderOptions.passStartLayout.size() - 1]) {
									layout = vk::ImageLayout::eUndefined;
								}
							}
							
							holderOptions.passStartLayout[holderOptions.passStartLayout.size() - 1][p.resourceIndex] = p.newLayout;
						}
					
					}

				}

			}
		}
		else {
			holderOptions.passes = 1;
			holderOptions.passStartLayout = {};
		}
		// create render pass(es)
		scene->coordinator->createRenderpasses(holderOptions);

		graphBuilt = true;
	}

	void SceneRenderCoordinator::loadOrGetRegisteredPipesInAllWindows()
	{
		// loop through all top level windows: virtual or onowned
		for (auto window : app.renderers[0]->windows) {
			//TODO: check if window already has same virtual pipeLoaded
			for (auto pipe : registeredPipes) {
				//TODO: cash pipelines through vulkan and or seperatly to prevent rebuilding as much
				auto concretePipe = new GraphicsPipeline(window->device, window->swapchainExtent, *window->renderPassManager, pipe->definition);
				window->loadedPipes[pipe] = concretePipe;
			}
		}
	}

	void SceneRenderCoordinator::createRenderpasses(const CRPHolder::HolderOptions& holderOptions)
	{

		sceneRenderpassHolders.push_back(new CRPHolder(std::move(__tempWholeFrameRenderPassOptions), holderOptions, app.renderers[0]));


		// set renderpass pointer on all windows even owned ones
		for (auto win : app.renderers[0]->allWindows) {
			win->renderPassManager = sceneRenderpassHolders[0]->renderPass(sceneRenderpassHolders[0]->passCount() - 1).first;
		}
	}



	void SceneRenderCoordinator::encodePassesForFrame(Renderer* renderer, vk::CommandBuffer firstLevelCMDBuffer, size_t frameID, Window& window)
	{
		/* Steps

		TODO: add mutli queue support
		TODO: move encodeing order arrond or posbly use gpu events as synchronosation to keep gpu feed while waiting for dependancies


		traverse dependancies into a lenear list to encode --- right now all of the encoding will be single threaded

		loop through that vector

			enter new render pass and or stop old one if necacary
			perform any necacry synchronosation for dependencies
			call each encode mthod
			excute indirect commands of the returned buffer



		https://visualgo.net/en/dfsbfs
		not as good as first website i had for graphs
		*/

		int64_t currentPass = -1;

		for (auto stage : stagesInOrder) {

#if SR_ENABLE_PRECONDITION_CHECKS
			//making sure each stage is properly registered
			SR_ASSERT(individualRunDependencies.find(stage) != individualRunDependencies.end());
#endif

			int64_t stagePass = passForStage[stage];

			// enter new render pass and or stop old one if necacary

			if (currentPass < stagePass) {
				//check if in a pass -- if so leave it
				


				// enter new render pass if and only if new pass is a different render pass
				//TODO the currentPass >= 0  is sort of already gaurentied so coujld be removed for performance
				if (currentPass < 0 || currentPass >= 0 && sceneRenderpassHolders[0]->arePassesDifferentRenderPasses(currentPass, stagePass)) {
					startNewPass(++currentPass,window,firstLevelCMDBuffer); 
				}


			}
			else {
				// perform synchronozation - onluy if not new pass because swithing passes implicitly adds synchronozation
				
			}


			auto passInfo = sceneRenderpassHolders[0]->renderPass(currentPass);
			
			auto buff = stage->encode(passInfo.second, window);

			VkDebug::beginRegion(firstLevelCMDBuffer, stage->name.c_str(), glm::vec4(0.7, 0.2, 0.3, 1));

			firstLevelCMDBuffer.executeCommands(*buff);

			VkDebug::endRegion(firstLevelCMDBuffer);

		}


	}

	void SceneRenderCoordinator::startNewPass(int64_t pass, Window& window, vk::CommandBuffer firstLevelCMDBuffer)
	{
		auto passInfo = sceneRenderpassHolders[0]->renderPass(pass);


		vk::RenderPassBeginInfo renderPassInfo{};

		renderPassInfo.renderArea = vk::Rect2D({ 0, 0 }, window.swapchainExtent);

		renderPassInfo.renderPass = passInfo.first->renderPass;
		renderPassInfo.framebuffer = sceneRenderpassHolders[0]->getFrameBuffer(pass, &window, window.currentSurfaceIndex);

		//TODO: make this compatable with depth buffers
		std::vector<vk::ClearValue> clearColors{};
		clearColors.resize(passInfo.first->getTotalAttatchmentCount());

		for (size_t i = 0; i < clearColors.size(); i++)
		{
			clearColors[i] = vk::ClearValue(vk::ClearColorValue(passInfo.first->options.attatchments[i].clearColor));
		}

		renderPassInfo.setClearValues(clearColors);

		firstLevelCMDBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eSecondaryCommandBuffers);
	}


}
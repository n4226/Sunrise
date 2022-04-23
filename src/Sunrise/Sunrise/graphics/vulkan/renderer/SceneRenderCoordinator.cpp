#include "srpch.h"
#include "SceneRenderCoordinator.h"

#include "Sunrise/Sunrise/core/Application.h"
#include "Sunrise/Sunrise/core/Window.h"
#include "Sunrise/Sunrise/scene/Scene.h"
#include "Sunrise/Sunrise/graphics/vulkan/GPU Stages/GpuStage.h"
#include "Sunrise/Sunrise/graphics/vulkan/renderer/Renderer.h"
#include "Sunrise/Sunrise/graphics/vulkan/renderPipelines/GraphicsPipeline.h"

#include "Sunrise/graphics/vulkan/renderer/MaterialManager.h"

#include "backends/imgui_impl_vulkan.h"

namespace sunrise::gfx {


	SceneRenderCoordinator::SceneRenderCoordinator(Scene* scene, Renderer* renderer)
		: scene(scene), app(scene->app), renderer(renderer)
	{
	}

	SceneRenderCoordinator::~SceneRenderCoordinator()
	{
		/*for (auto [pipe, stage] : registeredPipes)
			delete pipe;*/

		for (auto buffer : uniformBuffers)
		{
			for (auto sbuffer : buffer)
				delete sbuffer;
		}
		
		if (imguiStage)
            delete imguiStage;
        
		for (auto holders : sceneRenderpassHolders)
			delete holders;

		destroyIMGUIResources(scene->app);

		ImGui::DestroyContext();

        //deleting stages is done in super class
	}


	void SceneRenderCoordinator::reset()
	{
		//does not own them so can not delete them
		/*	for (auto [pipe, stage] : registeredPipes)
				delete pipe;*/

		registeredPipes.clear();

		lastStage = nullptr;
		__tempWholeFrameRenderPassOptions = ComposableRenderPass::CreateOptions();
		for (auto [stage, others] : individualRunDependencies) {
			stage->cleanup();
			delete stage;
		}
		individualRunDependencies.clear();
		individualRunDependencyOptoins.clear();
		stagesInOrder.clear();
		passForStage.clear();

		{
			auto& des = renderer->materialManager->registeredDescriptors;

			using Item = std::pair<gfx::SceneRenderCoordinator*, std::unordered_map<const Window*, std::vector<gfx::DescriptorSet*>>*>;

			auto newEnd = std::remove_if(des.begin(), des.end(), [this](auto& item) {
				return item.first == this;
				});
			des.erase(newEnd, des.end());

		}

		graphBuilt = false;
	}

	void SceneRenderCoordinator::createPasses()
	{
	}


	void SceneRenderCoordinator::registerPipeline(VirtualGraphicsPipeline* virtualPipe, GPUStage* forStage)
	{
		PROFILE_FUNCTION;
		//coordinator does not own virtual pipelines and so can not delete them
		virtualPipe->create();
		registeredPipes.push_back(std::make_pair(virtualPipe,forStage));
	}

	void SceneRenderCoordinator::setLastStage(GPUStage* lastStage)
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
		PROFILE_FUNCTION;

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
			for (auto win : renderer->windows) {
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
		// see if complex render pass creation is required
		if (multipleRenderPasses) {

			holderOptions.passes = 1;

			for (size_t stageIndex = 0; stageIndex < stagesInOrder.size(); stageIndex++)
			{
				auto stage = stagesInOrder[stageIndex];
				if (individualRunDependencyOptoins.count(stage) > 0) {
					auto& options = individualRunDependencyOptoins[stage];
					
					
					bool addedPassForStage = false;

					for (auto& p : options) {

						SR_ASSERT(p.newLayout == vk::ImageLayout::eShaderReadOnlyOptimal
							|| p.newLayout == vk::ImageLayout::eUndefined || p.newLayout == vk::ImageLayout::eColorAttachmentOptimal || p.newLayout == vk::ImageLayout::eDepthAttachmentOptimal || p.newLayout == vk::ImageLayout::eDepthReadOnlyOptimal);
						//TODO right now only checking for colorAttachment to and from eShaderReadOnlyOptimal layouts
						if (p.newLayout != vk::ImageLayout::eUndefined) {
							// the resource index of the dependancy option must be valid
							SR_ASSERT(p.resourceIndex < wholeFrameRenderPassOptions.attatchments.size());

							// the first stage can not have a layout transtion as 
							// this should be defined in the attachment configuration in the renderpassConfig() function
							SR_ASSERT(stage != stagesInOrder[0]);

							if (!addedPassForStage) {
								// its ok to do stageIndex - 1 becuase asserted two lines up that its not the first stage
								passForStage[stagesInOrder[stageIndex - 1]] = holderOptions.passes - 1;
								holderOptions.passes += 1;
								addedPassForStage = true;
							}

							// create empty transitions for all assets 
							if (holderOptions.passStartLayout.size() < holderOptions.passes - 1) {
								holderOptions.passStartLayout.push_back({});
								holderOptions.attachmentOps.push_back({});
								holderOptions.stencilOps.push_back({});

								holderOptions.passStartLayout[holderOptions.passStartLayout.size() - 1].resize(wholeFrameRenderPassOptions.attatchments.size());
								holderOptions.attachmentOps[holderOptions.attachmentOps.size() - 1].resize(wholeFrameRenderPassOptions.attatchments.size());
								holderOptions.stencilOps[holderOptions.stencilOps.size() - 1].resize(wholeFrameRenderPassOptions.attatchments.size());
								for (auto& layout : holderOptions.passStartLayout[holderOptions.passStartLayout.size() - 1]) {
									layout = vk::ImageLayout::eUndefined;
								}
							}
							
							holderOptions.passStartLayout[holderOptions.passStartLayout.size() - 1][p.resourceIndex] = p.newLayout;
							holderOptions.attachmentOps[holderOptions.attachmentOps.size() - 1][p.resourceIndex] = std::make_pair(p.loadOp,p.storeOp);
							holderOptions.stencilOps[holderOptions.stencilOps.size() - 1][p.resourceIndex] = std::make_pair(p.stencilLoadOp, p.stencilStoreOp);
						}
					
					}

				}

			}
			// set the pass of the last stage
			passForStage[stagesInOrder[stagesInOrder.size() - 1]] = holderOptions.passes - 1;
		}
		else {
			holderOptions.passes = 1;
			holderOptions.passStartLayout = {};
		}
		// create render pass(es)
		//why go throush scene when this is self?
		createRenderpasses(holderOptions);
		
		graphBuilt = true;

		loadOrGetRegisteredPipesInAllWindows();

		for (auto stage : stagesInOrder) {
			stage->lateSetup();
		}

		//Setup ImGui
		if (generateImguiStage)
			imguiStage = new ImGuiStage(this);

		//TODO: fix for MultiGPU
		if (!imguiInitilized) {
			if (renderer != app.renderers[0]) return;

			ImGui::CreateContext();

			//this initializes the core structures of imgui
			generateIMGUIResources(app); //-- Fix for Multi window

			//TODO: move 
			//see: https://twitter.com/ocornut/status/939547856171659264?lang=en
			float SCALE = 2.5f;
			ImFontConfig cfg;
			cfg.SizePixels = 13 * SCALE;
			ImGui::GetIO().Fonts->AddFontDefault(&cfg); //don't know what this is -> ->DisplayOffset.y = SCALE;

			//for (auto window : renderer->physicalWindows) {
			//TODO: fix ui to work on multi window
				setupIMGUIForWindow(renderer->physicalWindows[0], app);
			//}
			imguiInitilized = true;
		}
	}

	void SceneRenderCoordinator::drawableReleased(Window* window, size_t appFrame)
	{
		if (graphBuilt) {
			for (auto stage : stagesInOrder) {
				stage->drawableReleased(window, appFrame);
			}
		}
	}


	void SceneRenderCoordinator::loadOrGetRegisteredPipesInAllWindows()
	{
		PROFILE_FUNCTION;

		// loop through all top level windows: virtual or onowned
		for (auto window : renderer->windows) {
			//TODO: check if window already has same virtual pipeLoaded
			for (auto pipe : registeredPipes) {
				auto pass = passForStage[pipe.second];

				auto renderPass = sceneRenderpassHolders[0]->renderPass(pass).first;

				//TODO: cash pipelines through vulkan and or seperatly to prevent rebuilding as much
				auto concretePipe = new GraphicsPipeline(window->device, window->swapchainExtent,*renderPass, pipe.first->definition);
				window->loadedPipes[pipe.first] = concretePipe;
			}
		}
	}

	void SceneRenderCoordinator::createRenderpasses(const CRPHolder::HolderOptions& holderOptions)
	{
		PROFILE_FUNCTION;

		sceneRenderpassHolders.push_back(new CRPHolder(std::move(__tempWholeFrameRenderPassOptions), holderOptions, renderer));


		// set render pass pointer on all windows even owned ones
		for (auto win : renderer->allWindows) {
			//TODO: this object has to be deleted somewhere???!!
			win->renderPassManager = sceneRenderpassHolders[0]->renderPass(sceneRenderpassHolders[0]->passCount() - 1).first;
		}
	}



	void SceneRenderCoordinator::encodePassesForFrame(vk::CommandBuffer firstLevelCMDBuffer, size_t frameID, Window& window)
	{
		PROFILE_FUNCTION;
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

		// subclass can run code before encoding
		preEncodeUpdate(firstLevelCMDBuffer,frameID,window);

		int64_t currentPass = -1;

		bool inARenderPass = false;

		for (auto stage : stagesInOrder) {

#if SR_ENABLE_PRECONDITION_CHECKS
			//making sure each stage is properly registered
			SR_ASSERT(individualRunDependencies.find(stage) != individualRunDependencies.end());
#endif

			int64_t stagePass = passForStage[stage];

			// enter new render pass and or stop old one if necacary

			if (currentPass < stagePass) {
				//check if in a pass -- if so leave it
				if (inARenderPass) {
					firstLevelCMDBuffer.endRenderPass();
					inARenderPass = false;
				}


				// enter new render pass if and only if new pass is a different render pass
				//TODO the currentPass >= 0  is sort of already gaurentied so coujld be removed for performance
				if (currentPass < 0 || currentPass >= 0 && sceneRenderpassHolders[0]->arePassesDifferentRenderPasses(currentPass, stagePass)) {
					startNewPass(++currentPass,window,firstLevelCMDBuffer); 
					inARenderPass = true;
				}


			}
			else {
				// perform synchronozation - onluy if not new pass because swithing passes implicitly adds synchronozation
				
			}

			//todo: abstract out calling this function on thel ine bellow in all stages #canOptimize
			//auto passInfo = sceneRenderpassHolders[0]->renderPass(currentPass);
			

			//todo this has to be deleted when frame done preoccesing

            GPUStage::RunOptions options = { scene, this, static_cast<uint32_t>(currentPass), window };

			auto buff = stage->encode(options);
#if SR_LOGGING
			renderer->debugObject.beginRegion(firstLevelCMDBuffer, stage->name.c_str(), glm::vec4(0.7, 0.2, 0.3, 1));
#endif
			//if (currentPass == 1)
				firstLevelCMDBuffer.executeCommands(*buff);
#if SR_LOGGING
 			renderer->debugObject.endRegion(firstLevelCMDBuffer);
#endif
			//todo fix limitnign ui to first window and gpu
			if (stage == lastStage && imguiStage && renderer == app.renderers[0] && window.globalIndex == 0) {
				//draw imgui

#if SR_LOGGING
				renderer->debugObject.beginRegion(firstLevelCMDBuffer, "ImGui Pass", glm::vec4(1, 0.1, 0.2, 1));
#endif
                GPUStage::RunOptions options = { scene, this, static_cast<uint32_t>(currentPass), window };

				auto imguiBuff = imguiStage->encode(options);

				//cuaing multi monitor vulkan errors
				firstLevelCMDBuffer.executeCommands(*imguiBuff);

#if SR_LOGGING
				renderer->debugObject.endRegion(firstLevelCMDBuffer);
#endif
			}

				//if(imguideaw
		}

		if (inARenderPass)
			firstLevelCMDBuffer.endRenderPass();

		//if virtual window do necacary copying

		if (window.isVirtual()) {

			auto layout = vk::ImageLayout::ePresentSrcKHR; //TODO: this needs to be set correclty based on what is set in renderpasss attachment options - and if crpholder changes to recognize that virtual windows want tranfersrc than work with that too

			for (uint32_t i = 0; i < window.numSubWindows() ; i++)
			{
				auto child = window.subWindows[i];
				//copy layer i into sub window i's swap image

				ResourceTransferer::ImageTransferTask task{};

				task.src = window.virtualSwapImages[window.currentSurfaceIndex]->vkItem;
				task.srcLayout = layout;
				task.postSRCLayout = vk::ImageLayout::eTransferSrcOptimal;
				
				task.dst = child->swapChainImages[child->currentSurfaceIndex];
				task.dstLayout = vk::ImageLayout::eUndefined;
				task.postDSTLayout = vk::ImageLayout::ePresentSrcKHR;

				vk::ImageCopy region;
				region.srcSubresource = { vk::ImageAspectFlagBits::eColor,0,i,1 };
				region.srcOffset = vk::Offset3D{ 0,0,0 };

				region.dstSubresource = { vk::ImageAspectFlagBits::eColor,0,0,1 };
				region.dstOffset = vk::Offset3D{ 0,0,0 };

				region.extent.width = window.swapchainExtent.width;
				region.extent.height = window.swapchainExtent.height;
				region.extent.depth = 1;

				task.regions = { region };

				renderer->resouceTransferer->performImageTransferTask(task, firstLevelCMDBuffer);

				//layout = vk::ImageLayout::eTransferSrcOptimal;
			}
		}

	}

	void SceneRenderCoordinator::startNewPass(int64_t pass, Window& window, vk::CommandBuffer firstLevelCMDBuffer)
	{
		PROFILE_FUNCTION;

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
			if (passInfo.first->options.attatchments[i].type == ComposableRenderPass::CreateOptions::AttatchmentType::Color)
				clearColors[i] = vk::ClearValue(vk::ClearColorValue(passInfo.first->options.attatchments[i].clearColor));
			else
				clearColors[i] = vk::ClearValue(passInfo.first->options.attatchments[i].clearDepthStencil);
		}

		renderPassInfo.setClearValues(clearColors);

		firstLevelCMDBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eSecondaryCommandBuffers);
	}

	void SceneRenderCoordinator::preEncodeUpdate(vk::CommandBuffer firstLevelCMDBuffer, size_t frameID, Window& window)
	{
		updateSceneUniformBuffer(window);
	}

	void SceneRenderCoordinator::registerForGlobalMaterials(std::unordered_map<const Window*, std::vector<gfx::DescriptorSet*>>* descriptors)
	{
		renderer->materialManager->registeredDescriptors.push_back(std::make_pair(this,descriptors));
	}

}

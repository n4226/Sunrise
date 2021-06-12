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


	void SceneRenderCoordinator::encodePassesForFrame(Renderer* renderer, vk::CommandBuffer firstLevelCMDBuffer, size_t frameID, Window& window)
	{
		/* Steps 
		
		TODO: most important figure out how this will interact with and relate to render passes
		TODO: add mutli queue support
		TODO: move encodeing order arrond or posbly use gpu events as synchronosation to keep gpu feed while waiting for dependancies

		traverse dependancies into a lenear list to encode --- right now all of the encoding will be single threaded 

		loop through that vector

			perform any necacry synchronosation for dependencies
			call each encode mthod 
			excute indirect commands of the returned buffer

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

		std::vector<GPUStage*> stagesInOrder = {};
		std::vector<GPUStage*> stagesNodeQueue = {};
		std::unordered_set<GPUStage*> visited = {};

		//for (auto node : individualRunDependencies) {
		//	if (node.second.size() == 0) {
		//		stagesInOrder.push_back(node);
		//	}
		//}

		GPUStage* firstNode = lastStage;

		stagesNodeQueue.push_back(firstNode);
		// dont rememb er how this is supposed to work so overidding this for now --- this was the active code
		/*while (!stagesNodeQueue.empty()) {
			auto node = stagesNodeQueue[stagesNodeQueue.size() - 1];
			stagesNodeQueue.pop_back();
			if (visited.count(node) == 0) {
				visited.insert(node);
				stagesNodeQueue.push_back(node);

				for (auto dependency : individualRunDependencies[node]) {
					if (visited.count(node) == 0)
						stagesNodeQueue.push_back(dependency);
				}
			}
		}
		std::reverse(stagesInOrder.begin(), stagesInOrder.end());*/
		
		//todo: fixthis
		std::vector<GPUStage*> keys;
		keys.reserve(individualRunDependencies.size());
		//std::vector<Val> vals;
		//vals.reserve(map.size());

		for (auto kv : individualRunDependencies) {
			keys.push_back(kv.first);
			//vals.push_back(kv.second);
		}
		stagesInOrder.push_back(keys[0]);

		//SR_CORE_TRACE("{}", stagesInOrder.size());


		for (auto stage : stagesInOrder) {

			// perform synchronozation



			//TODO: fix that the subpass is always 0 - oractually maybe not if the engine just does not use rnederpasses --
			//this system will replace that --
			//this is ok since right now mobile and or TBDR gpus ate not something that will be targeted
			//they should probably be supported in the future especially becuase of apple silicon if that is ever a target platform
			auto buff = stage->encode(0,window);

			firstLevelCMDBuffer.executeCommands(*buff);

		}


	}

	void SceneRenderCoordinator::registerPipeline(VirtualGraphicsPipeline* virtualPipe)
	{
		registeredPipes.push_back(virtualPipe);
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


}
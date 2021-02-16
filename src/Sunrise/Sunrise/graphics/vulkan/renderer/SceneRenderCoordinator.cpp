#include "srpch.h"
#include "SceneRenderCoordinator.h"

#include "Sunrise/Sunrise/core/Application.h"
#include "Sunrise/Sunrise/scene/Scene.h"
#include "Sunrise/Sunrise/graphics/vulkan/GPU Stages/GpuStage.h"

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

		while (!stagesNodeQueue.empty()) {
			auto node = stagesNodeQueue[stagesNodeQueue.size()];
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
		std::reverse(stagesInOrder.begin(), stagesInOrder.end());

		SR_CORE_TRACE("{}", stagesInOrder);


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


}
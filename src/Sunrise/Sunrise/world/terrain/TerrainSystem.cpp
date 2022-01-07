#include "srpch.h"
#include "TerrainSystem.h"
#include "../../core/Application.h"
#include "../../core/Window.h"
#include "../../graphics/vulkan/generalAbstractions/VkAbstractions.h"
#include "../../fileFormats/binary/BinaryMesh.h"

#include "../../graphics/vulkan/resources/ResourceTransferTask.h"

#include "../../graphics/vulkan/renderer/Renderer.h"

#include "../../graphics/vulkan/renderPipelines/concrete/gpuDriven/GPUGenCommandsPipeline.h"

#include "../WorldScene.h"

#include <marl/defer.h>

namespace sunrise {

	using namespace gfx;

	TerrainSystem::TerrainSystem(Application& app, WorldScene& scene, glm::dvec3* origin)
		: tree(math::dEarthRad), app(app), scene(scene), meshLoader(), origin(origin)
	{
		PROFILE_FUNCTION;
		SR_CORE_TRACE("Initializing Terrain System");

			//TODO: fix for multi gpu to use multiple renderers

		meshLoader.renderer = app.renderers[0];
		meshLoader.terrainSystem = this;

		CreateRenderResources();


		if (scene.terrainMask == nullptr) {
			// initial setup of the base 8 chunks
			for (TerrainQuadTreeNode* child : tree.leafNodes) {
				meshLoader.drawChunk(child, meshLoader.loadMeshPreDrawChunk(child), false);
			}
		}
		else {
			SR_CORE_INFO("Terrain System Initiated in Masked mode. DO NOT change this for the lifetime of thie object");
			maskedMode = true;


			for (auto& chunk : *scene.terrainMask)
			{
				auto leaf = new TerrainQuadTreeNode(chunk, nullptr, &tree, 0);
				tree.leafNodes.insert(leaf);
				meshLoader.drawChunk(leaf, meshLoader.loadMeshPreDrawChunk(leaf), false);
			}
			writePendingDrawOobjects(*app.renderers[0]);
		}

		//writePendingDrawOobjects();
	}

	TerrainSystem::~TerrainSystem()
	{
	/*	for (auto pool : cmdBufferPools)
			for (auto spool : pool)
				app.renderers[0]->device.destroyCommandPool(spool);*/
	}

	void TerrainSystem::CreateRenderResources()
	{
		PROFILE_FUNCTION;

		SR_CORE_TRACE("Creating Terrain System resources");

//
//		cmdBufferPools.resize(app.renderers[0]->windows.size());
//		commandBuffers.resize(app.renderers[0]->windows.size());
//
//		for (size_t i = 0; i < app.renderers[0]->windows.size(); i++)
//			vkHelpers::createPoolsAndCommandBufffers
//			(app.renderers[0]->device, cmdBufferPools[i], commandBuffers[i], app.maxSwapChainImages, app.renderers[0]->queueFamilyIndices.graphicsFamily.value(), vk::CommandBufferLevel::eSecondary);
//
//#if RenderMode == RenderModeCPU2
//
//		cmdBuffsUpToDate.resize(app.renderers[0]->windows.size());
//
//		for (size_t i = 0; i < cmdBuffsUpToDate.size(); i++) {
//
//			cmdBuffsUpToDate[i].resize(app.maxSwapChainImages);
//
//			for (auto& sval : cmdBuffsUpToDate[i])
//				sval = true;
//		}
//#endif
	}

	void TerrainSystem::update()
	{
		PROFILE_FUNCTION;

		if (!maskedMode)
			processTree();
	}



	double TerrainSystem::getRadius()
	{
		return tree.radius;
	}

	void TerrainSystem::invalidateDescriptors()
	{
		SR_CORE_CRITICAL("{} not updatedf for gpu passes ",__FUNCSIG__);
//#if RenderMode == RenderModeCPU2
//		{
//			auto cmdsValid = drawCommandsValid.lock();
//			*cmdsValid = false;
//		}
//#endif
	}

	void TerrainSystem::processTree()
	{
		PROFILE_FUNCTION

		{
			// this is temporary to prevent multiple sets of staging buffer coppies to rech deadlock currently

			// for now tree updating is coursly syn cronized so wonce the tree begins updating all other updates are frozen / forbiden until all new chunks are loaded from disk - might need improvment for streaming
			auto shouldRunLoop = safeToModifyChunks.try_lock_shared();
			if (shouldRunLoop == nullptr || (*shouldRunLoop) == false) return;
		}

		auto adjustedOriginTrackedPos = static_cast<glm::dvec3>(trackedTransform->position) + *origin;

		for (TerrainQuadTreeNode* node : tree.leafNodes) {

			if (node->active && !node->isSplit && node->lodLevel < (lodLevels - 1) && splitFormThreshold(node,adjustedOriginTrackedPos))
			{
				// split node

				toSplit.insert(node);

			}
			else if (node->parent != nullptr && node->parent->isSplit)
			{
				if (combineFromThreshold(node->parent,adjustedOriginTrackedPos))
				{
					//combine node

					auto split = true;

					for (TerrainQuadTreeNode* child : node->parent->children) {
						if (toSplit.count(child) > 0 || child->isSplit)
						{
							split = false;
							break;
						}
					}
					if (split)
						toCombine.insert(node->parent);
				}
			}

		}

		for (TerrainQuadTreeNode* node : toSplit) {
			toDestroyDraw.insert(node);
			//meshLoader.removeDrawChunk(node);

			node->split();
			//std::cout << "nodes created with level: " << node->lodLevel + 1 << std::endl;
			for (TerrainQuadTreeNode* child : node->children) {
				toDrawDraw.insert(child);
				//drawChunk(child);
			}
		}

		for (TerrainQuadTreeNode* node : toCombine) {

			for (TerrainQuadTreeNode* child : node->children) {
				toDestroyDraw.insert(child);
				//meshLoader.removeDrawChunk(child);
				assert(child->children.size() == 0);
			}
			// will have to be combined after the draw is removed 
			//node->combine();

			toDrawDraw.insert(node);
			//drawChunk(node);
		}

		// draw in jobs

		if (toDrawDraw.size() > 0)
		{
			{
				auto shouldRunLoop = safeToModifyChunks.lock();
				*shouldRunLoop = false;
			}

			//auto ticket = ticketQueue.take();

			marl::schedule([this]() {
				PROFILE_SCOPE("create draw draw job");
					//MarlSafeTicketLock lock(ticket);

				marl::WaitGroup preLoadingWaitGroup(toDrawDraw.size());

				for (TerrainQuadTreeNode* chunk : toDrawDraw) {
					marl::schedule([this, chunk, preLoadingWaitGroup]() {
						defer(preLoadingWaitGroup.done());
						meshLoader.loadMeshPreDrawChunk(chunk, true);
					});
				}

				preLoadingWaitGroup.wait();

				// actually modify render structures
				for (TerrainQuadTreeNode* chunk : toDrawDraw) {
					meshLoader.drawChunk(chunk, {}, false);
				}

				//write staging buffer updates to gpu buffs
				writePendingDrawOobjects(*app.renderers[0]);


				toDrawDraw.clear();

				// this here so that "deleted chiunks dont render but THEY ARE NOT ACTUALLY FREED RIGHT NOW"
				for (TerrainQuadTreeNode* chunk : toDestroyDraw) {
					meshLoader.removeDrawChunk(chunk);
				}

				// re encode draws
				// todo ------------------------------------ DO NOT DO THIS EACH FRAME cash the value #fixme
				auto tstage = world->coordinator->getRegisteredStageOfType<TerrainGPUStage>();
				
				// todo make this more robust
				for (auto win : app.renderers[0]->windows) {
					for (size_t surface = 0; surface < win->swapChainImages.size(); surface++)
					{
						tstage->reEncodeBuffer(*win, surface);
					}
				}

				{ // swaping active command buffers
					auto activeHandler = tstage->activeBuffer.lock(); 
					*activeHandler = (*activeHandler + 1) % TerrainGPUStage::setsOfCMDBuffers;
				}

				//todo: wait and then calculate when to delete old chunks that should be see below
				{ //todo fix - don't know that these chunks are still not being used by a drawable
					

					for (TerrainQuadTreeNode* chunk : toCombine) {
						chunk->combine();
					}

					toCombine.clear();
					toDestroyDraw.clear();
				}

				// TODO The tree can posibly be marked as safe to modify before rencoding draw calls
				{
					auto shouldRunLoop = safeToModifyChunks.lock();
					*shouldRunLoop = true;
					destroyAwaitingNodes = true;
				}
			});

		}
		else {
			//for (TerrainQuadTreeNode* chunk : toDestroyDraw) {
			//	meshLoader.removeDrawChunk(chunk);
			//}

			//for (TerrainQuadTreeNode* chunk : toCombine) {
			//	chunk->combine();
			//}

			//toCombine.clear();
			//toDestroyDraw.clear();
		}




		toSplit.clear();
		//toCombine.clear();

		//if (destroyAwaitingNodes) {
		//	for (TerrainQuadTreeNode* node : toDestroyDraw) {
		//		removeDrawChunk(node);
		//	}
		//	destroyAwaitingNodes = false;
		//}

		//writePendingDrawOobjects();
	}


	double TerrainSystem::threshold(const TerrainQuadTreeNode* node)
	{
		auto nodeRad = math::llaDistance(node->frame.start, node->frame.getEnd(), tree.radius);
		//      return  radius / (node.lodLevel + 1).double * 1;
		return nodeRad;
	}

	bool sunrise::TerrainSystem::splitFormThreshold(const TerrainQuadTreeNode* node, const glm::dvec3& trackedPos)
	{
		auto distance = glm::distance(trackedPos, node->center_geo);

		auto threshold = TerrainSystem::threshold(node);

		threshold = threshold / 1.5;

		return distance < threshold;
	}

	bool sunrise::TerrainSystem::combineFromThreshold(const TerrainQuadTreeNode* node, const glm::dvec3& trackedPos)
	{
		auto distance = glm::distance(trackedPos, node->center_geo);

		auto threshold = TerrainSystem::threshold(node);

		threshold = threshold / 1.5;

		return distance > (threshold * 1.05);
	}

	bool TerrainSystem::determinActive(const TerrainQuadTreeNode* node)
	{
		return true;
	}

	void TerrainSystem::setActiveState(TerrainQuadTreeNode* node)
	{
		//TODO: fix this
	}

	void TerrainSystem::writePendingDrawOobjects(Renderer& renderer)
	{
		// copy from staging buffers to gpu ones - asynchronously

		ResourceTransferer::BufferTransferTask vertexTask;

		vertexTask.srcBuffer = renderer.globalMeshStagingBuffer->vertBuffer->vkItem;
		vertexTask.dstBuffer = renderer.globalMeshBuffer->vertBuffer->vkItem;


		ResourceTransferer::BufferTransferTask indexTask;

		indexTask.srcBuffer = renderer.globalMeshStagingBuffer->indexBuffer->vkItem;
		indexTask.dstBuffer = renderer.globalMeshBuffer->indexBuffer->vkItem;



		ResourceTransferer::BufferTransferTask modelTask;

		modelTask.srcBuffer = renderer.globalModelBufferStaging->vkItem;
		modelTask.dstBuffer = renderer.globalModelBuffers[renderer.gpuActiveGlobalModelBuffer]->vkItem;

		{
			auto drawQueue = pendingDrawObjects.lock_shared();
			if (drawQueue->size() == 0) return;

			for (std::pair<TerrainQuadTreeNode*, TreeNodeDrawData> objectData : *drawQueue) {

				for (size_t i = 0; i < objectData.second.meshRecipt.vertexLocations.size(); i++)
				{
					BindlessMeshBuffer::WriteLocation& vertRecpt = objectData.second.meshRecipt.vertexLocations[i];
					vertexTask.regions.emplace_back(vertRecpt.offset, vertRecpt.offset, vertRecpt.size);
				}

				BindlessMeshBuffer::WriteLocation& meshRecpt = objectData.second.meshRecipt.indexLocation;
				indexTask.regions.emplace_back(meshRecpt.offset, meshRecpt.offset, meshRecpt.size);

				BindlessMeshBuffer::WriteLocation& modelRecpt = objectData.second.modelRecipt;
				modelTask.regions.emplace_back(modelRecpt.offset, modelRecpt.offset, modelRecpt.size);
			}
		}

		auto taskType = ResourceTransferer::TaskType::bufferTransfers;

		std::vector<ResourceTransferer::Task> tasks = { {taskType,vertexTask},{taskType,indexTask},{taskType, modelTask} };

		renderer.resouceTransferer->newTask(tasks, [&]() {
			{
				PROFILE_SCOPE("terrain system ResourceTransferer::shared->newTask completion callback")
				{
					auto drawQueue = pendingDrawObjects.lock();
					auto drawObjects = this->drawObjects.lock();
					drawObjects->insert(drawQueue->begin(), drawQueue->end());
					drawQueue->clear();
					//using namespace std::chrono_literals;
					//std::this_thread::sleep_for(1s);
				}
			}
			}, true,false);

	}

}
#include "srpch.h"
#include "TerrainSystem.h"
#include "../../core/Application.h"
#include "../../core/Window.h"
#include "../../graphics/vulkan/generalAbstractions/VkAbstractions.h"
#include "../../fileFormats/binary/BinaryMesh.h"

#include "../../graphics/vulkan/resources/ResourceTransferTask.h"

#include "../../graphics/vulkan/renderer/Renderer.h"


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


		for (TerrainQuadTreeNode* child : tree.leafNodes) {
			meshLoader.drawChunk(child, meshLoader.loadMeshPreDrawChunk(child), false);
		}

		//writePendingDrawOobjects();
	}

	TerrainSystem::~TerrainSystem()
	{
		for (auto pool : cmdBufferPools)
			for (auto spool : pool)
				app.renderers[0]->device.destroyCommandPool(spool);
	}

	void TerrainSystem::CreateRenderResources()
	{
		PROFILE_FUNCTION;

		SR_CORE_TRACE("Creating Terrain System resources");


		cmdBufferPools.resize(app.renderers[0]->windows.size());
		commandBuffers.resize(app.renderers[0]->windows.size());

		for (size_t i = 0; i < app.renderers[0]->windows.size(); i++)
			vkHelpers::createPoolsAndCommandBufffers
			(app.renderers[0]->device, cmdBufferPools[i], commandBuffers[i], app.maxSwapChainImages, app.renderers[0]->queueFamilyIndices.graphicsFamily.value(), vk::CommandBufferLevel::eSecondary);

#if RenderMode == RenderModeCPU2

		cmdBuffsUpToDate.resize(app.renderers[0]->windows.size());

		for (size_t i = 0; i < cmdBuffsUpToDate.size(); i++) {

			cmdBuffsUpToDate[i].resize(app.maxSwapChainImages);

			for (auto sval : cmdBuffsUpToDate[i])
				sval = true;
		}
#endif
	}

	void TerrainSystem::update()
	{
		PROFILE_FUNCTION;

		processTree();
	}

	vk::CommandBuffer* TerrainSystem::renderSystem(uint32_t subpass, Window& window)
	{
		PROFILE_FUNCTION

			uint32_t bufferIndex = window.currentSurfaceIndex;

		auto renderer = app.renderers[0];

		// if CPU mode 2 than only re encode commands for this surface's command buffer when changes occor otherwise just return the cmd buff for hte current surface
#if RenderMode == RenderModeCPU2
		{
			//TODO: posibly make this a try lock to improve performance
			auto cmdsValid = drawCommandsValid.lock();

			if (*cmdsValid == false) {
				for (size_t i = 0; i < cmdBuffsUpToDate.size(); i++) {
					for (auto sval : cmdBuffsUpToDate[i])
						sval = false;
				}
				*cmdsValid = true;
			}
			if (cmdBuffsUpToDate[window.indexInRenderer][bufferIndex] == true)
				return &commandBuffers[window.indexInRenderer][bufferIndex];
			else
				cmdBuffsUpToDate[window.indexInRenderer][bufferIndex] = true;
		}

#endif

		renderer->device.resetCommandPool(cmdBufferPools[window.indexInRenderer][bufferIndex], {});

		vk::CommandBuffer* buffer = &commandBuffers[window.indexInRenderer][bufferIndex];


		vk::CommandBufferInheritanceInfo inheritanceInfo{};
		inheritanceInfo.renderPass = window.renderPassManager->renderPass;
		inheritanceInfo.subpass = subpass;
		inheritanceInfo.framebuffer = window.swapChainFramebuffers[bufferIndex];

		vk::CommandBufferBeginInfo beginInfo{};
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eRenderPassContinue; // Optional
		beginInfo.pInheritanceInfo = &inheritanceInfo; // Optional

		buffer->begin(beginInfo);

		buffer->bindPipeline(vk::PipelineBindPoint::eGraphics, window.pipelineCreator->vkItem);

		// setup descriptor and buffer bindings

		buffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, window.pipelineCreator->pipelineLayout, 0, { renderer->descriptorSets[window.indexInRenderer][window.currentSurfaceIndex] }, {});

		//// temp using cpu buffs  FIX THIS SOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOON
		//renderer->globalMeshStagingBuffer->bindVerticiesIntoCommandBuffer(*buffer, 0);
		//renderer->globalMeshBuffer->bindIndiciesIntoCommandBuffer(*buffer);

		renderer->globalMeshBuffer->bindVerticiesIntoCommandBuffer(*buffer, 0);
		renderer->globalMeshBuffer->bindIndiciesIntoCommandBuffer(*buffer);
			

		// encode draws
		{

			auto drawObjects = this->drawObjects.lock();
			//printf("number of draws = %d \n", drawObjects->size());
			for (auto it = drawObjects->begin(); it != drawObjects->end(); it++)
			{

				// frustrom cull
#if RenderMode == RenderModeCPU1
				if (!renderer->camFrustroms[window.indexInRenderer].IsBoxVisible(it->second.aabbMin, it->second.aabbMax)) {
					continue;
				}
#endif

				auto modelUnSize = sizeof(glm::uint32);
				//buffer->pushConstants(window.pipelineCreator->pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, modelUnSize, &it->second.drawDatas[0]);
				for (size_t i = 0; i < it->second.indexCounts.size(); i++)
				{
					auto indexCount = it->second.indexCounts[i];
					auto indexOffset = it->second.indIndicies[i];
					//buffer->pushConstants(window.pipelineCreator->pipelineLayout, vk::ShaderStageFlagBits::eFragment, modelUnSize, sizeof(DrawPushData) - modelUnSize, reinterpret_cast<char*>(&(it->second.drawDatas[i])) + modelUnSize);

					buffer->pushConstants(window.pipelineCreator->pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(DrawPushData), &it->second.drawDatas[i]);
					buffer->drawIndexed(indexCount, 1, indexOffset, it->second.vertIndex, 0);
				}
			}
		}
		buffer->end();

		return buffer;
	}

	double TerrainSystem::getRadius()
	{
		return tree.radius;
	}

	void TerrainSystem::invalidateDescriptors()
	{
#if RenderMode == RenderModeCPU2
		{
			auto cmdsValid = drawCommandsValid.lock();
			*cmdsValid = false;
		}
#endif
	}

	void TerrainSystem::processTree()
	{
		PROFILE_FUNCTION

		{
			// his is temporary to prevent multiple sets of staging buffer coppies to rech deadlock currently
			auto shouldRunLoop = safeToModifyChunks.try_lock_shared();
			if (shouldRunLoop == nullptr || (*shouldRunLoop) == false) return;
		}

		auto adjustedOriginTrackedPos = static_cast<glm::dvec3>(trackedTransform->position) + *origin;

		for (TerrainQuadTreeNode* node : tree.leafNodes) {

			auto threshold = this->threshold(node);

			auto distance = glm::distance(adjustedOriginTrackedPos, node->center_geo);

			if (node->active && !node->isSplit && node->lodLevel < (lodLevels - 1) && distance < threshold)
			{
				// split node

				toSplit.insert(node);

			}
			else if (node->parent != nullptr && node->parent->isSplit)
			{
				auto nextThreshold = this->threshold(node->parent);

				auto nextDistance = glm::distance(adjustedOriginTrackedPos, node->parent->center_geo);

				if (nextDistance > (nextThreshold * 1.05))
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

			//TODO - thread this again in marl task this is jsut for testing
			marl::schedule([this]() {
				PROFILE_SCOPE("create draw draw job")
					//MarlSafeTicketLock lock(ticket);

					marl::WaitGroup preLoadingWaitGroup(toDrawDraw.size());

				for (TerrainQuadTreeNode* chunk : toDrawDraw) {
					marl::schedule([this, chunk, preLoadingWaitGroup]() {
						defer(preLoadingWaitGroup.done());
						meshLoader.loadMeshPreDrawChunk(chunk, true);
						});
				}

				preLoadingWaitGroup.wait();

				for (TerrainQuadTreeNode* chunk : toDrawDraw) {
					meshLoader.drawChunk(chunk, {}, false);
				}

				//TODO - fix
				writePendingDrawOobjects(*app.renderers[0]);

				for (TerrainQuadTreeNode* chunk : toDestroyDraw) {
					meshLoader.removeDrawChunk(chunk);
				}

				for (TerrainQuadTreeNode* chunk : toCombine) {
					chunk->combine();
				}

				toCombine.clear();
				toDestroyDraw.clear();
				toDrawDraw.clear();


				{
					auto shouldRunLoop = safeToModifyChunks.lock();
					*shouldRunLoop = true;
					destroyAwaitingNodes = true;
				}
#if RenderMode == RenderModeCPU2
				{
					auto cmdsValid = drawCommandsValid.lock();
					*cmdsValid = false;
				}
#endif
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
		return nodeRad * 1;
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
			}, false,false);

	}

}
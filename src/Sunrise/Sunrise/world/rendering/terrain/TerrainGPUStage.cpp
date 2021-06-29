#include "srpch.h"
#include "TerrainGPUStage.h"

#include"Sunrise/Sunrise/math/mesh/MeshPrimatives.h"
#include"Sunrise/Sunrise/core/Application.h"
#include"Sunrise/Sunrise/core/Window.h"
#include"Sunrise/Sunrise/scene/Scene.h"
#include"Sunrise/Sunrise/graphics/vulkan/renderer/Renderer.h"
#include"Sunrise/Sunrise/graphics/vulkan/renderer/SceneRenderCoordinator.h"
#include "../../gfxPipelines/WorldTerrainPipeline.h"
#include "../WorldSceneRenderCoordinator.h"
#include "../../WorldScene.h"


namespace sunrise {

	TerrainGPUStage::TerrainGPUStage(WorldSceneRenderCoordinator* coord)
		: GPURenderStage(coord,"World Terrain Render Stage"), worldCoord(coord)
	{

	}

	void TerrainGPUStage::setup()
	{
		using DescriptorTypeAllocOptions = gfx::DescriptorPool::CreateOptions::DescriptorTypeAllocOptions;
		//gfx::DescriptorPool::CreateOptions::DescriptorTypeAllocOptions sampledImageAllocOptions = { vk::DescriptorType::eSampledImage, app.maxSwapChainImages };


		DescriptorTypeAllocOptions globalUniformPoolSize{};
		globalUniformPoolSize.type = vk::DescriptorType::eUniformBuffer;
		globalUniformPoolSize.maxNum = static_cast<uint32_t>(app.maxSwapChainImages);

		// the total max number of this descriptor allocated - if 2 sets and each one has 2 of this descriptor than thes would have to be 4 in order to allocate both sets
		DescriptorTypeAllocOptions modelAndMatUniformPoolSize{};
		modelAndMatUniformPoolSize.type = vk::DescriptorType::eStorageBuffer;
		// dont know why *2
		modelAndMatUniformPoolSize.maxNum = static_cast<uint32_t>(app.maxSwapChainImages * 2);

		DescriptorTypeAllocOptions materialTexturesPoolSize{};
		materialTexturesPoolSize.type = vk::DescriptorType::eCombinedImageSampler;
		//  this is not the array count 
		materialTexturesPoolSize.maxNum = 1;

		descriptorPool = new gfx::DescriptorPool(app.renderers[0]->device, 
			{ app.maxSwapChainImages * app.renderers[0]->windows.size(),{ globalUniformPoolSize, modelAndMatUniformPoolSize, materialTexturesPoolSize} });


		for (size_t i = 0; i < setsOfCMDBuffers; i++)
		{
			cmdBufferPools[i].resize(app.renderers[0]->windows.size());
			commandBuffers[i].resize(app.renderers[0]->windows.size());

			for (size_t j = 0; j < app.renderers[0]->windows.size(); j++) {
				gfx::vkHelpers::createPoolsAndCommandBufffers
				(app.renderers[0]->device, cmdBufferPools[i][j], commandBuffers[i][j], app.maxSwapChainImages, app.renderers[0]->queueFamilyIndices.graphicsFamily.value(), vk::CommandBufferLevel::eSecondary);
				
			}

		}

	}

	void TerrainGPUStage::lateSetup()
	{
		selfPass = coord->getPass(this);

		auto renderer = app.renderers[0];
		for (auto window : renderer->windows)
		{
			setUsedBySurface.insert(std::make_pair(window, std::vector<size_t>()));
			setUsedBySurface.at(window).resize(window->swapChainImages.size());


			descriptorSets[window] = {};
			for (size_t swap = 0; swap < window->swapChainImages.size(); swap++)
			{
				auto pipeline = getConcretePipeline(*window, worldTerrainPipeline);
				auto des = descriptorPool->allocate(pipeline->descriptorSetLayouts);

				descriptorSets[window].push_back(des[0]);

				//todo check if it is valid to bind descriptor for a whole buffer in which a shader only uses the first fiew bytes of (not hwole buffer)
				VkDescriptorBufferInfo globalUniformBufferInfo{};
				globalUniformBufferInfo.buffer =  worldCoord->uniformBuffers[window->indexInRenderer][swap]->vkItem;
				globalUniformBufferInfo.offset = 0;
				globalUniformBufferInfo.range = VK_WHOLE_SIZE;


				VkDescriptorBufferInfo modelUniformBufferInfo{};
				modelUniformBufferInfo.buffer =
					renderer->globalModelBuffers[0]->vkItem;
				//globalModelBufferStaging->vkItem;
				modelUniformBufferInfo.offset = 0;
				modelUniformBufferInfo.range = VK_WHOLE_SIZE;


				VkDescriptorBufferInfo matUniformBufferInfo{};
				matUniformBufferInfo.buffer = renderer->globalMaterialUniformBuffer->vkItem;
				matUniformBufferInfo.offset = 0;
				matUniformBufferInfo.range = VK_WHOLE_SIZE;

				gfx::DescriptorPool::UpdateOperation sceneUniformsUpdateOp = { gfx::DescriptorPool::UpdateOperation::Type::write, 
					des[0]->makeBinding(0),0,1, gfx::DescriptorPool::UpdateOperation::ReferenceType(globalUniformBufferInfo) };


				gfx::DescriptorPool::UpdateOperation modelUniformsUpdateOp = { gfx::DescriptorPool::UpdateOperation::Type::write,
					des[0]->makeBinding(1),0,1, gfx::DescriptorPool::UpdateOperation::ReferenceType(modelUniformBufferInfo) };


				gfx::DescriptorPool::UpdateOperation matUniformsUpdateOp = { gfx::DescriptorPool::UpdateOperation::Type::write,
					des[0]->makeBinding(2),0,1, gfx::DescriptorPool::UpdateOperation::ReferenceType(matUniformBufferInfo) };


				descriptorPool->update({sceneUniformsUpdateOp, modelUniformsUpdateOp, matUniformsUpdateOp });
			}
		}


		// encode commands buffs with empty commands
		for (auto win : app.renderers[0]->windows) {
			for (size_t surface = 0; surface < win->swapChainImages.size(); surface++)
			{
				reEncodeBuffer(*win, surface);
			}
		}
		{

			//swap buffers so that the ones just set up are active
			auto handle = activeBuffer.lock();

			*handle = 1;
			mainThreadLocalCopyOfActiveBuffer = 1;
		}
	}

	void TerrainGPUStage::cleanup()
	{
		descriptorPool->reset();

		delete descriptorPool;
	}


	vk::CommandBuffer* TerrainGPUStage::encode(RunOptions options)
	{
		PROFILE_FUNCTION;

		//auto buff = selectAndSetupCommandBuff(options);

		uint32_t bufferIndex = options.window.currentSurfaceIndex;


		// check if back command buffers are done and ready to be swaped
		// if so swap

		{
			auto handle = activeBuffer.try_lock_shared();
			if (handle != nullptr) {
				if (*handle != mainThreadLocalCopyOfActiveBuffer) {
					mainThreadLocalCopyOfActiveBuffer = *handle;
					//SR_CORE_INFO("Swaping command buf sets to send set {} to the gpu", mainThreadLocalCopyOfActiveBuffer);
				}
			}
		}

		//return active buff
		size_t activeBUff = mainThreadLocalCopyOfActiveBuffer;

		// mark this buffer as being used
		{ 
			auto handle = this->commandBuffersInUse.lock();

			//https://stackoverflow.com/questions/17172080/insert-vs-emplace-vs-operator-in-c-map

			(*handle)[activeBUff] += 1;

			//SR_CORE_TRACE("ref count for set {} is now {}", activeBUff, (*handle)[activeBUff]);

			setUsedBySurface[&options.window][bufferIndex] = activeBUff;
		}


		return &commandBuffers[activeBUff][options.window.indexInRenderer][bufferIndex];
	}

	void TerrainGPUStage::drawableReleased(Window* window, size_t surface)
	{
		// todo: now usage is only tracked for a whole set so no encodig can be done if only one frame in flihgt is using any of the buffers in that set
		auto buffSet = setUsedBySurface.at(window)[surface];
		{
			auto handle = this->commandBuffersInUse.lock();

			(*handle)[buffSet] -= 1;
			//SR_CORE_TRACE("ref count for set {} is now {}", buffSet, (*handle)[buffSet]);
		}
	}


	void TerrainGPUStage::reEncodeBuffer(const Window& window, size_t surface)
	{
		PROFILE_FUNCTION;

		// bad synch problem

		// this funciton must not be called if a frame  is currently still inflight with the associteted command buffer

		auto renderer = app.renderers[0];

		// intentianally locked for whole creation time so that the buffers are never swaped while work is happening
		auto handle = activeBuffer.lock_shared();

		size_t buffSet = *handle == 0 ? 1 : 0;

		// wait if buffer in flight is using this set
		while (true) {
			{
				auto handle = this->commandBuffersInUse.lock();

				// todo: make this waiting much better
				if ((*handle)[buffSet] <= 0) {
					break;
				}
			}
			SR_CORE_INFO("sleeping waiting for terrain sys buff to leave inflight");
			Sleep(1);
		}


		//SR_CORE_TRACE("encoding terrain using set {}", buffSet);


		renderer->device.resetCommandPool(cmdBufferPools[buffSet][window.indexInRenderer][surface], {});

		vk::CommandBuffer buffer = commandBuffers[buffSet][window.indexInRenderer][surface];

		setupCommandBuff(buffer, coord, selfPass, window, surface);

		setPipeline(window, buffer, worldTerrainPipeline);

		// setup descriptor and buffer bindings
		auto pipeline = getConcretePipeline(window, worldTerrainPipeline);


		buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->pipelineLayout, 0, { descriptorSets.find(&window)->second[surface]->vkItem }, {});

		renderer->globalMeshBuffer->bindVerticiesIntoCommandBuffer(buffer, 0);
		renderer->globalMeshBuffer->bindIndiciesIntoCommandBuffer (buffer);


#pragma region CPU Driven Encoding

		// encode draws
		{
			PROFILE_SCOPE("encode draws");
			auto drawObjects = worldCoord->worldScene->terrainSystem->drawObjects.lock_shared();
			//printf("number of draws = %d \n", drawObjects->size());
			for (auto it = drawObjects->begin(); it != drawObjects->end(); it++)
			{

				// frustrom cull
				// disabled because it doesnt work wit hpre encoded commands
				//if (!renderer->camFrustroms[window.indexInRenderer].IsBoxVisible(it->second.aabbMin, it->second.aabbMax)) {
				//	continue;
				//}

				auto modelUnSize = sizeof(glm::uint32);
				//buffer->pushConstants(window.pipelineCreator->pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, modelUnSize, &it->second.drawDatas[0]);
				for (size_t i = 0; i < it->second.indexCounts.size(); i++)
				{
					auto indexCount = it->second.indexCounts[i];
					auto indexOffset = it->second.indIndicies[i];
					//buffer->pushConstants(window.pipelineCreator->pipelineLayout, vk::ShaderStageFlagBits::eFragment, modelUnSize, sizeof(DrawPushData) - modelUnSize, reinterpret_cast<char*>(&(it->second.drawDatas[i])) + modelUnSize);

					buffer.pushConstants(pipeline->pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(gfx::DrawPushData), &it->second.drawDatas[i]);
					buffer.drawIndexed(indexCount, 1, indexOffset, it->second.vertIndex, 0);
				}
			}
		}
#pragma endregion
		buffer.end();

	}

}
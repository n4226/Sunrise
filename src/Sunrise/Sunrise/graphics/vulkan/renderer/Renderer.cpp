#include "srpch.h"
#include "Renderer.h"
#include "../../../world/WorldScene.h"
#include "../../../core/Application.h"

#include "MaterialManager.h"
#include "Sunrise/Sunrise/world/terrain/TerrainSystem.h"
#include "Sunrise/Sunrise/graphics/vulkan/renderer/SceneRenderCoordinator.h"

#include "Sunrise/fileSystem/FileManager.h"

#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"

namespace sunrise::gfx {

	Renderer::Renderer(Application& app, vk::Device device, vk::PhysicalDevice physicalDevice,
		VmaAllocator allocator, std::vector<Window*> windows, GPUQueues& deviceQueues, QueueFamilyIndices& queueFamilyIndices, VkDebug debugObject)
		: app(app), device(device), physicalDevice(physicalDevice),
		allocator(allocator), windows(windows), deviceQueues(deviceQueues), queueFamilyIndices(queueFamilyIndices), debugObject(debugObject)
	{
		PROFILE_FUNCTION;


		//TODO: check if gpu supports aftermath before doing this
		debugObject.initAftermath();

		resouceTransferer = new ResourceTransferer(device, *this);
		//TODO: for multi gpu this maybe should not be owned by a rednerer but the application
		materialManager = new MaterialManager(*this);

		debugDraw = new DebugDrawer(this, {});
	}

	void Renderer::createAllResources()
	{
		printVMAAllocattedStats();

		for (size_t i = 0; i < windows.size(); i++)
			windows[i]->indexInRenderer = i;

		for (size_t i = 0; i < allWindows.size(); i++)
			allWindows[i]->allIndexInRenderer = i;

		for (size_t i = 0; i < physicalWindows.size(); i++)
			physicalWindows[i]->physicalIndexInRenderer = i;

		// setup camera frustrums
		camFrustroms.reserve(physicalWindows.size());
		for (auto win : physicalWindows)
			camFrustroms.emplace_back(win->camera.view());

		createRenderResources();

		createDynamicRenderCommands();

		printVMAAllocattedStats();
	}

	Renderer::~Renderer()
	{
		PROFILE_FUNCTION

        
        device.waitIdle();
        
		delete materialManager;
		delete resouceTransferer;


		for (auto pool : dynamicCommandPools)
			for (auto spool : pool)
				device.destroyCommandPool(spool);

		delete globalMeshStagingBuffer;

		for (auto buff : threadLocalGlobalMeshStagingBuffers) {
			buff->vertBuffer->unmapMemory();
			buff->indexBuffer->unmapMemory();
			delete buff;
		}

		delete globalMeshBuffer;
		for (auto buffer : globalModelBuffers)
		{
			delete buffer;
		}

		for (auto buff : threadLocalGlobalModelStagingBuffers) {
			buff->unmapMemory();
			delete buff;
		}

		delete globalModelBufferStaging;

		delete gloablIndAllocator;
		delete gloablVertAllocator;
		delete globalModelBufferAllocator;

		delete globalMaterialUniformBuffer;
		delete globalMaterialUniformBufferStaging;

//        char** stats = new char*;
//        vmaBuildStatsString(allocator, stats, VK_TRUE);
//        std::string statString(*stats);
//        
//        FileManager::saveStringToFile(statString, "vmaAlloc.json");
        
		//delete windows

		//TODO: not sure if this is correct for MVR
		for (auto window : allWindows) {
			delete window;
		}

		printVMAAllocattedStats();

        
        //used bytes: 1122043300
        //used bytes: 0681641380
        //used bytes: 0094438840
        //used bytes: 0094438840
        
		//now:
		//83867392
		//83867392
		//83067136
		//83067136
		//83067136
		//256
		//0!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! :)

		//startup:
		//8036002304


		//zero leak without loading scene :)

        // delete memory - will crash if somethign is still allocated with it
        vmaDestroyAllocator(allocator);
        
		//TODO: destryoy descirptors registered to material manager correctly --------------------------------------------------------------------
        
        device.destroy();
	}

	void Renderer::printVMAAllocattedStats()
	{
		auto stats = new VmaStats;

		vmaCalculateStats(allocator, stats);

		SR_CORE_WARN("{} of unalockated vulkan object bytes", stats->total.usedBytes);
	}

	void Renderer::windowSizeChanged(size_t allWindowIndex)
	{
		//TODO: save performance by only reseting the descriptors for the resized window
		resetDescriptorPools();

	}

	
	BufferCreationOptions Renderer::newBufferOptions()
	{
		return { ResourceStorageType::cpu,{ }, vk::SharingMode::eConcurrent,
			{ queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.resourceTransferFamily.value() } };;
	}

	void Renderer::createRenderResources()
	{
		PROFILE_FUNCTION;


#pragma region Create Global vert and index buffers

		//todo: make this more configureable and make sure it is supported by the gpu

		VkDeviceSize vCount = 70'000'000 * 5;
		VkDeviceSize indexCount = 220'000'000 * 0.5;

#if SR_RenderDocCompatible
		// debug tools like renderdocf and nsight graphics have real trouble with huge allocations so they have to be reduced
		//makeGlobalMeshBuffers(vCount / 10, indexCount / 10);
		makeGlobalMeshBuffers(vCount / 5, indexCount / 5);
#else
		makeGlobalMeshBuffers(vCount, indexCount);
#endif
#pragma endregion


		//createDescriptorPoolAndSets();


	}

	void Renderer::makeGlobalMeshBuffers(const VkDeviceSize& vCount, const VkDeviceSize& indexCount)
	{
		BufferCreationOptions options =
		{ ResourceStorageType::cpu,{ vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferSrc }, vk::SharingMode::eConcurrent,
			{ queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.resourceTransferFamily.value() } };

		globalMeshStagingBuffer = new BindlessMeshBuffer(device, allocator, options, vCount, indexCount);


		//TODO Extract this comman code somewhere

		auto workerThreads = std::thread::hardware_concurrency() * 0;

		threadLocalGlobalMeshStagingBuffers.resize(workerThreads);

		auto threadMeshModelInd = freeThreadLocalGlobalMeshandModelStagingBufferIndicies.lock();

		for (size_t thread = 0; thread < workerThreads; thread++)
		{
			threadMeshModelInd->push_back(thread);
			threadLocalGlobalMeshStagingBuffers[thread] = new BindlessMeshBuffer(device, allocator, options, vCount, indexCount);
			threadLocalGlobalMeshStagingBuffers[thread]->indexBuffer->mapMemory();
			threadLocalGlobalMeshStagingBuffers[thread]->vertBuffer->mapMemory();
		}


		options.usage = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc;


		globalModelBufferStaging = new Buffer(device, allocator, sizeof(ModelUniforms) * maxModelUniformDescriptorArrayCount, options);

		globalMaterialUniformBufferStaging = new Buffer(device, allocator, sizeof(MaterialUniforms) * maxMaterialTextureDescriptorArrayCount, options);

		//globalMaterialUniformBufferStaging->name("Staging - Material Uniform Buffer", debugObject);


		threadLocalGlobalModelStagingBuffers.resize(workerThreads);
		for (size_t thread = 0; thread < workerThreads; thread++)
		{
			threadLocalGlobalModelStagingBuffers[thread] = new Buffer(device, allocator, sizeof(ModelUniforms) * maxModelUniformDescriptorArrayCount, options);
			threadLocalGlobalModelStagingBuffers[thread]->mapMemory();
		}

		options.storage = ResourceStorageType::gpu;

		options.usage = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst;

		globalMaterialUniformBuffer = new Buffer(device, allocator, sizeof(MaterialUniforms) * maxMaterialTextureDescriptorArrayCount, options);

		//globalMaterialUniformBuffer->name("Material Uniform Buffer",debugObject);

		globalModelBuffers = {
			new Buffer(device, allocator, sizeof(ModelUniforms) * maxModelUniformDescriptorArrayCount, options),
			new Buffer(device, allocator, sizeof(ModelUniforms) * maxModelUniformDescriptorArrayCount, options),
		};

		options.usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
		globalMeshBuffer = new BindlessMeshBuffer(device, allocator, options, vCount, indexCount);

		gloablVertAllocator = new VaribleIndexAllocator(globalMeshBuffer->vCount);
		gloablIndAllocator = new VaribleIndexAllocator(globalMeshBuffer->indexCount);

		matUniformAllocator = new IndexAllocator(maxModelUniformDescriptorArrayCount, sizeof(MaterialUniforms));
		globalModelBufferAllocator = new IndexAllocator(maxModelUniformDescriptorArrayCount, sizeof(ModelUniforms));
	}

	void Renderer::resetDescriptorPools()
	{
		SR_CORE_CRITICAL("reseting descripters needs to be implimented in GPU-Stages for WorldSys and others");
		SR_ASSERT(false);
		//device.resetDescriptorPool(descriptorPool);
		//device.resetDescriptorPool(deferredDescriptorPool);
	}

	void Renderer::createDynamicRenderCommands()
	{
		PROFILE_FUNCTION;
		dynamicCommandPools.resize(windows.size());
		dynamicCommandBuffers.resize(windows.size());
		for (size_t i = 0; i < windows.size(); i++)
			vkHelpers::createPoolsAndCommandBufffers
			(device, dynamicCommandPools[i], dynamicCommandBuffers[i], app.maxSwapChainImages, queueFamilyIndices.graphicsFamily.value(), vk::CommandBufferLevel::ePrimary);
	}

	void Renderer::drawableReleased(Window* window, size_t appFrame)
	{
		// todo make better way to get active scene
		//auto scene = app.loadedScenes[0];

		for (auto scene : app.loadedScenes)
			scene->coordinators.at(this)->drawableReleased(window, appFrame);
	}


	void Renderer::beforeRenderScene()
	{
		//updateSceneUniformBuffer();

		//for (auto win : windows)
		//	debugDraw->sendToBuffer(win);

	}

	void Renderer::renderFrame(Window& window)
	{
		PROFILE_FUNCTION;

		debugDraw->sendToBuffer(&window);


		/*
			Render Pass layout -- PBR-Deferred Pipeline

			if gpu driven: a compute pre pass

			for now i will use render sub passes but this does not allow pixels to acces their neighbors

			gbuffer pass

				- attatchments

				- output: for now just rgb color buffer

			deferred lighting pass

				- input: all outputed gbuffer buffers - memory synchronised

				- output: an () formatted color output texture


			a varible number of post passes

				- input: previus output or deferred output

				- output: new texture same format as input

		*/



			// update descriptors to link drawable bound objects eg comd buffs to frame bound objets eg uniform buffers 


			//updateRunTimeDescriptors(window);

		// this is nolonger the renderer's responcability
		//updateSceneUniformBuffer(window);

#pragma region CreateRootCMDBuffer

	// create root cmd buffer

	device.resetCommandPool(dynamicCommandPools[window.indexInRenderer][window.currentSurfaceIndex], {});

	auto cmdBuff = dynamicCommandBuffers[window.indexInRenderer][window.currentSurfaceIndex];

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; //VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional

	cmdBuff.begin(beginInfo);


#pragma endregion

		//ImGui_ImplVulkan_NewFrame();
		//ImGui_ImplGlfw_NewFrame();
		//ImGui::NewFrame();
		//ImGui::ShowDemoWindow();
		//ImGui::Render();

		
		auto coord = app.loadedScenes[0]->coordinators.at(this);
		//needs to do special coping if this is a virtual window
		coord->encodePassesForFrame(cmdBuff, app.currentFrameID, window);

		
		// end encoding 
		cmdBuff.end();

		// submit frame
		submitFrameQueue(window, &cmdBuff, 1);
	}


	void Renderer::submitFrameQueue(Window& window, vk::CommandBuffer* buffers, uint32_t bufferCount)
	{
		vk::SubmitInfo submitInfo{};

		std::vector<vk::Semaphore> waitSemaphores = {};
		waitSemaphores.reserve(4);

		if (window.isVirtual()) {
			for each (auto child in window.subWindows)
			{
				waitSemaphores.push_back(child->imageAvailableSemaphores[app.currentFrame]);
			}
		}
		else {
			waitSemaphores.push_back(window.imageAvailableSemaphores[app.currentFrame]);
		}

		std::vector<vk::PipelineStageFlags> waitStages = { vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput) };

		if (window.isVirtual())
			waitStages.push_back(vk::PipelineStageFlagBits::eTransfer);

		submitInfo.waitSemaphoreCount = waitSemaphores.size();
		submitInfo.pWaitSemaphores = waitSemaphores.data();
		submitInfo.setWaitDstStageMask(waitStages);

		submitInfo.commandBufferCount = bufferCount;
		submitInfo.pCommandBuffers = buffers;

		std::vector<vk::Semaphore> signalSemaphores = { window.renderFinishedSemaphores[app.currentFrame] };
		submitInfo.setSignalSemaphores(signalSemaphores);

			 

		vkResetFences(device, 1, &window.inFlightFences[app.currentFrame]);

		// submit queue

		deviceQueues.graphics.submit({ submitInfo }, window.inFlightFences[app.currentFrame]);
	}

	void Renderer::afterRenderScene()
	{
		debugDraw->flushData();
	}

}

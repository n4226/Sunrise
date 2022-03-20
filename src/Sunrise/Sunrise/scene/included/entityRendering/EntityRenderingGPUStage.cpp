#include "srpch.h"
#include "EntityRenderingGPUStage.h"

#include "Sunrise/graphics/vulkan/resources/uniforms.h"
#include <Sunrise/Sunrise/world/gfxPipelines/WorldTerrainPipeline.h>
#include "Sunrise/core/Window.h"
#include "Sunrise/graphics/vulkan/resources/ResourceTransferTask.h"

namespace sunrise {

	EntityRenderingGPUStage::EntityRenderingGPUStage(gfx::SceneRenderCoordinator* coord)
		: GPURenderStage(coord, "Scene Entity Render Stage")
	{

	}

	void EntityRenderingGPUStage::setup()
	{
		auto renderer = coord->app.renderers[0];

		auto& reg = coord->scene->registry;

		auto view = reg.view<Mesh, Transform, MeshRenderer>();

		gfx::BufferCreationOptions options = renderer->newBufferOptions();

		//TODO: --------- have to have staging buffers and copy over to gpu ---------------------------------------------------
		options.storage = gfx::ResourceStorageType::cpuToGpu;
		options.usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer;

		for (auto [ent, mesh, transform, meshRender] : view.each()) {
			//create mesh buffer from mesh

			auto meshBuff = new gfx::MeshBuffer(renderer->device,renderer->allocator,options,&mesh);
			meshBuff->clearBaseMesh();
			meshBuff->writeMeshToBuffer(true, &mesh);
			meshBuffers.emplace(std::make_pair(ent,meshBuff));


			gfx::ModelUniforms mtrans;
			mtrans.model = transform.matrix();


			auto modelIndex = renderer->globalModelBufferAllocator->alloc();
			auto modelAllocSize = renderer->globalModelBufferAllocator->allocSize;

			renderer->globalModelBufferStaging->tempMapAndWrite(&mtrans, modelIndex, modelAllocSize, true);
			
			gfx::ResourceTransferer::Task transferTask{};

			transferTask.type = gfx::ResourceTransferer::bufferTransfers;
			transferTask.bufferTransferTask.srcBuffer = renderer->globalModelBufferStaging->vkItem;
			transferTask.bufferTransferTask.dstBuffer = renderer->globalModelBuffers[renderer->gpuActiveGlobalModelBuffer]->vkItem;
			transferTask.bufferTransferTask.regions = { { modelIndex * modelAllocSize, modelIndex * modelAllocSize,modelAllocSize} };
			
			//have to copy that to actual buff line above

			renderer->resouceTransferer->newTask({ transferTask }, [] {});


			modelUniformIndicies.emplace(std::make_pair(ent, modelIndex));

			//auto transformBuff = new gfx::Buffer(renderer->device, renderer->allocator, sizeof(glm::mat4),options);
			//auto mat = transform.matrix();
			//transformBuff->tempMapAndWrite(&mat);

			//transBuffers.insert(std::make_pair(ent, transformBuff));
		}

		createDescriptorPool();

	}

	void EntityRenderingGPUStage::lateSetup()
	{

		AllocateDescriptors();


	}

	void EntityRenderingGPUStage::AllocateDescriptors()
	{

		auto renderer = app.renderers[0];
		for (auto window : renderer->windows)
		{
			descriptorSets[window] = {};
			for (size_t swap = 0; swap < window->swapChainImages.size(); swap++)
			{
				auto pipeline = getConcretePipeline(*window, worldTerrainPipeline);


				// allocating for all static materials
				//TODO: update to not hardcode 5 textures per material crashes on mac ->// SWtaticMaterialTable::reverseEntries.size()*5
				auto des = descriptorPool->allocate(pipeline->descriptorSetLayouts, { static_cast<uint32_t>(maxMaterialTextureDescriptorArrayCount) });

				descriptorSets[window].push_back(des[0]);

				//todo check if it is valid to bind descriptor for a whole buffer in which a shader only uses the first few bytes of (not whole buffer)
				VkDescriptorBufferInfo globalUniformBufferInfo{};
				globalUniformBufferInfo.buffer = coord->uniformBuffers[window->indexInRenderer][swap]->vkItem;
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


				descriptorPool->update({ sceneUniformsUpdateOp, modelUniformsUpdateOp, matUniformsUpdateOp });
			}
		}
	}

	void EntityRenderingGPUStage::createDescriptorPool()
	{
		using DescriptorTypeAllocOptions = gfx::DescriptorPool::CreateOptions::DescriptorTypeAllocOptions;
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
		materialTexturesPoolSize.maxNum = maxMaterialTextureDescriptorArrayCount * app.maxSwapChainImages;//1;


		//TODO: understand the point of variable descriptor arrays if you still have to pass length
		descriptorPool = new gfx::DescriptorPool(app.renderers[0]->device,
			{ app.maxSwapChainImages * app.renderers[0]->windows.size(),{ globalUniformPoolSize, modelAndMatUniformPoolSize, materialTexturesPoolSize} });
	}

	void EntityRenderingGPUStage::cleanup()
	{
		for (auto [ent, buff] : meshBuffers) {
			delete buff;
		}
		auto renderer = coord->app.renderers[0];

		delete descriptorPool;
	}

	vk::CommandBuffer* EntityRenderingGPUStage::encode(RunOptions options)
	{
		auto cmdBuff = selectAndSetupCommandBuff(options);


		setPipeline(options.window, *cmdBuff, worldTerrainPipeline);

		// setup descriptor and buffer bindings
		auto pipeline = getConcretePipeline(options.window, worldTerrainPipeline);

		cmdBuff->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->pipelineLayout,
			0, { descriptorSets.find(&options.window)->second[options.window.currentSurfaceIndex]->vkItem }, {});



		//entities are rendered if they have the following components: Transform, Mesh, MeshRenderer

		auto& reg = coord->scene->registry;

		//have to deal with converting meshes to mesh buffers


		auto view = reg.view<Mesh, Transform, MeshRenderer>();

		for (auto [ent, mesh, transform, renderer]: view.each()) {

			//right now - assuming all objects created before first frame and not modified

			//if buffers for mesh and transform do not exist/need updating do so

			//set cmdbuff vert and index buffers
			//draw object with push constant material proper
			//id and item index of 0 so it will use the translation buffer as single object not bindless buffer


			if (meshBuffers.find(ent) != meshBuffers.end() && modelUniformIndicies.find(ent) != modelUniformIndicies.end()) {

				auto meshBuff = meshBuffers.at(ent);
				auto model = modelUniformIndicies.at(ent);
				auto material = renderer.material;

				meshBuff->bindVerticiesIntoCommandBuffer(*cmdBuff, 0,&mesh);
				meshBuff->bindIndiciesIntoCommandBuffer(*cmdBuff, &mesh);

				
				for (size_t i = 0; i < mesh.indicies.size(); i++)
				{
					gfx::DrawPushData pushConsts = {model, material};
					cmdBuff->pushConstants(pipeline->pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
						0, sizeof(gfx::DrawPushData), &pushConsts);
					size_t startVert = 0;
					cmdBuff->drawIndexed(mesh.indiciesSize(i) / sizeof(glm::uint32), 1, 0, startVert, 0);
				}

			}


		}

		cmdBuff->end();

		return cmdBuff;
	}

}
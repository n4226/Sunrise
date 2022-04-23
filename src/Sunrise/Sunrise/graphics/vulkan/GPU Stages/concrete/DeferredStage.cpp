#include "srpch.h"
#include "DeferredStage.h"

#include"Sunrise/Sunrise/math/mesh/MeshPrimatives.h"
#include"Sunrise/Sunrise/core/Application.h"
#include"Sunrise/Sunrise/core/Window.h"
#include"Sunrise/Sunrise/scene/Scene.h"
#include"Sunrise/Sunrise/graphics/vulkan/renderer/Renderer.h"
#include"Sunrise/Sunrise/graphics/vulkan/renderer/SceneRenderCoordinator.h"
#include"Sunrise/Sunrise/graphics/vulkan/renderPipelines/concrete/GPUStages/DeferredPipeline.h"

#include"Sunrise/Sunrise/world/rendering/WorldSceneRenderCoordinator.h"

namespace sunrise {

	DeferredStage::DeferredStage(gfx::SceneRenderCoordinator* coord, AttachOptions attachments)
		: GPURenderStage(coord, "Sunrise Deferred Render Stage"), attachments(attachments)
	{

	}

	void DeferredStage::setup()
	{
		square = new Basic2DMesh(MeshPrimatives::Basic2D::screenQuad());

		// TODO: make staging buffer so faster for simple draw
		meshBuff = new gfx::Basic2DMeshBuffer(coord->renderer->device, coord->renderer->allocator,
			{ gfx::ResourceStorageType::cpuToGpu,vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer,
			vk::SharingMode::eExclusive }
		, square);
		meshBuff->writeMeshToBuffer(true);


		inputImageSampler = new gfx::Sampler(coord->renderer->device, {});

		size_t estimatedSurfaceCount = app.maxSwapChainImages * app.windows.size();

		gfx::DescriptorPool::CreateOptions::DescriptorTypeAllocOptions sampledImageAllocOptions = { vk::DescriptorType::eSampledImage, estimatedSurfaceCount};
        
        gfx::DescriptorPool::CreateOptions::DescriptorTypeAllocOptions combinedSampledImageAllocOptions = { vk::DescriptorType::eCombinedImageSampler, estimatedSurfaceCount * 4};
        
		gfx::DescriptorPool::CreateOptions::DescriptorTypeAllocOptions uniformAllocOptions = { vk::DescriptorType::eUniformBuffer, estimatedSurfaceCount };

		descriptorPool = new gfx::DescriptorPool(coord->renderer->device, { estimatedSurfaceCount * 6, { sampledImageAllocOptions, uniformAllocOptions, combinedSampledImageAllocOptions} });

	}

	void DeferredStage::lateSetup()
	{
		//TODO: label why this has to be done
		for (auto window : coord->renderer->windows)
		{
			descriptorSets[window] = {};
			for (int childI = 0; childI < (window->isVirtual() ? window->numSubWindows() : 1); childI++)
			{
				for (size_t swap = 0; swap < window->numSwapImages(); swap++)
				{
					auto pipeline = getConcretePipeline(*window, deferredPipeline);
					auto des = descriptorPool->allocate(pipeline->descriptorSetLayouts);

					descriptorSets[window].push_back(des[0]);

					uint32_t layer = childI;

					vk::DescriptorImageInfo imageInfo1 = { inputImageSampler->vkItem ,
						coord->sceneRenderpassHolders[0]->getImage(attachments.gbuffAlbedoMetalicIndex,window)->createView(1,layer),vk::ImageLayout::eShaderReadOnlyOptimal };

					vk::DescriptorImageInfo imageInfo2 = { inputImageSampler->vkItem ,
						coord->sceneRenderpassHolders[0]->getImage(attachments.gbuffNormalSpecularIndex,window)->createView(1,layer),vk::ImageLayout::eShaderReadOnlyOptimal };

					vk::DescriptorImageInfo imageInfo3 = { inputImageSampler->vkItem ,
						coord->sceneRenderpassHolders[0]->getImage(attachments.gbuffAoIndex,window)->createView(1,layer),vk::ImageLayout::eShaderReadOnlyOptimal };

					vk::DescriptorImageInfo imageInfo4 = { inputImageSampler->vkItem ,
						coord->sceneRenderpassHolders[0]->getImage(attachments.gbuffDepthIndex,window)->createView(1,layer),vk::ImageLayout::eShaderReadOnlyOptimal };


					VkDescriptorBufferInfo globalUniformBufferInfo{};
					globalUniformBufferInfo.buffer = coord->uniformBuffers[window->indexInRenderer][swap]->vkItem;
					globalUniformBufferInfo.offset = 0;
					globalUniformBufferInfo.range = VK_WHOLE_SIZE;

					//this is the descriptor array's index not images which is why it should be zero
					uint32_t dstArrayLayerStart = 0;

					gfx::DescriptorPool::UpdateOperation updateOp1 = { gfx::DescriptorPool::UpdateOperation::Type::write,
						des[0]->makeBinding(0),dstArrayLayerStart,1, gfx::DescriptorPool::UpdateOperation::ReferenceType(imageInfo1) };

					gfx::DescriptorPool::UpdateOperation updateOp2 = { gfx::DescriptorPool::UpdateOperation::Type::write,
						des[0]->makeBinding(1),dstArrayLayerStart,1, gfx::DescriptorPool::UpdateOperation::ReferenceType(imageInfo2) };

					gfx::DescriptorPool::UpdateOperation updateOp3 = { gfx::DescriptorPool::UpdateOperation::Type::write,
						des[0]->makeBinding(2),dstArrayLayerStart,1, gfx::DescriptorPool::UpdateOperation::ReferenceType(imageInfo3) };

					gfx::DescriptorPool::UpdateOperation updateOp4 = { gfx::DescriptorPool::UpdateOperation::Type::write,
						des[0]->makeBinding(3),dstArrayLayerStart,1, gfx::DescriptorPool::UpdateOperation::ReferenceType(imageInfo4) };


					gfx::DescriptorPool::UpdateOperation sceneUniformsUpdateOp = { gfx::DescriptorPool::UpdateOperation::Type::write,
						des[0]->makeBinding(4),0,1, gfx::DescriptorPool::UpdateOperation::ReferenceType(globalUniformBufferInfo) };

					descriptorPool->update({ updateOp1, updateOp2, updateOp3, updateOp4, sceneUniformsUpdateOp });
				}
			}
		}
	}

	void DeferredStage::cleanup()
	{
		delete square;
		delete meshBuff;
	}

	vk::CommandBuffer* DeferredStage::encode(RunOptions options)
	{

		auto buff = selectAndSetupCommandBuff(options);

		setPipeline(options.window, *buff, deferredPipeline);

		auto pipeline = getConcretePipeline(options.window, deferredPipeline);

		meshBuff->bindVerticiesIntoCommandBuffer(*buff, 0);
		meshBuff->bindIndiciesIntoCommandBuffer(*buff);

		for (uint32_t childI = 0; childI < (options.window.isVirtual() ? options.window.numSubWindows() : 1); childI++)
		{

			buff->bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
				pipeline->pipelineLayout, 0,
				{ descriptorSets[&options.window][((size_t)childI * options.window.numSwapImages()) + options.window.currentSurfaceIndex]->vkItem,}, {});

			buff->pushConstants(pipeline->pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(uint32_t), &childI);

			buff->drawIndexed(square->indicies.size(), 1, 0, 0, 0);

		}

		buff->end();

		return buff;
	}

}

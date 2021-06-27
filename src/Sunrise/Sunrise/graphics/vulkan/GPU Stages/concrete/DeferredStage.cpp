#include "srpch.h"
#include "DeferredStage.h"

#include"Sunrise/Sunrise/math/mesh/MeshPrimatives.h"
#include"Sunrise/Sunrise/core/Application.h"
#include"Sunrise/Sunrise/graphics/vulkan/renderer/Renderer.h"
#include"Sunrise/Sunrise/graphics/vulkan/renderPipelines/concrete/GPUStages/DeferredPipeline.h"

namespace sunrise {

	DeferredStage::DeferredStage(Application& app)
		: GPURenderStage(app, "Sunrise Deferred Render Stage")
	{

	}

	void DeferredStage::setup()
	{
		square = new Basic2DMesh(MeshPrimatives::Basic2D::screenQuad());

		// todo make staging buffer so faster for simple draw
		meshBuff = new gfx::Basic2DMeshBuffer(app.renderers[0]->device, app.renderers[0]->allocator,
			{ gfx::ResourceStorageType::cpuToGpu,vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer,
			vk::SharingMode::eExclusive }
		, square);
		meshBuff->writeMeshToBuffer(true);
	}

	void DeferredStage::cleanup()
	{

	}

	vk::CommandBuffer* DeferredStage::encode(RunOptions options)
	{

		auto buff = selectAndSetupCommandBuff(options);

		setPipeline(options.window, *buff, deferredPipeline);

		meshBuff->bindVerticiesIntoCommandBuffer(*buff, 0);
		meshBuff->bindIndiciesIntoCommandBuffer(*buff);

		//auto pipeline = getConcretePipeline(options.window, deferredPipeline);

		buff->drawIndexed(square->indicies.size(), 1, 0, 0, 0);

		buff->end();

		return buff;
	}

}
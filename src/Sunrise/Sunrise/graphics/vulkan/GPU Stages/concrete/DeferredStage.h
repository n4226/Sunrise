#pragma once

#include "srpch.h"
#include "Sunrise/Sunrise/graphics/vulkan/GPU Stages/GPURenderStage.h"
#include "Sunrise/Sunrise/math/mesh/Mesh.h"
#include "Sunrise/Sunrise/graphics/vulkan/resources/MeshBuffers.h"
#include "Sunrise/Sunrise/graphics/vulkan/generalAbstractions/VkAbstractions.h"

namespace sunrise {

	class DeferredStage : public gfx::GPURenderStage
	{
	public:
		struct AttachOptions {
			size_t gbuffAlbedoMetalicIndex;
			size_t gbuffNormalSpecularIndex;
			size_t gbuffAoIndex;
			size_t gbuffDepthIndex;
		};

		DeferredStage(gfx::SceneRenderCoordinator* coord, AttachOptions attachments);


		void setup() override;
		void lateSetup() override;
		void cleanup() override;

		vk::CommandBuffer* encode(RunOptions options) override;

	protected:

		Basic2DMesh* square{};
		gfx::Basic2DMeshBuffer* meshBuff{};

		//descriptors

		gfx::DescriptorPool* descriptorPool;

		std::unordered_map<Window*, std::vector<gfx::DescriptorSet*>> descriptorSets{};

		gfx::Sampler* inputImageSampler;


		AttachOptions attachments;
	};

}
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

		/// <summary>
		/// for normal windows - outer map = windows, inner array = surfaces
		/// for virtual windows - outer map same but inner array = surfaces * children i.e first (surface count) represent child one's surfaces
		///	they dont directly reference surfaces but instead reference the fact that for virtual iwndows the post porccesing will have to happen once per child window and hence will need descriptors for each
		/// </summary>
		std::unordered_map<Window*, std::vector<gfx::DescriptorSet*>> descriptorSets{};

		gfx::Sampler* inputImageSampler;


		AttachOptions attachments;
	};

}
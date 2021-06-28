#pragma once

#include "srpch.h"
#include "Sunrise/Sunrise/graphics/vulkan/GPU Stages/GPURenderStage.h"
#include "../WorldSceneRenderCoordinator.h"

namespace sunrise {

	class WorldSceneRenderCoordinator;

	class TerrainGPUStage: public gfx::GPURenderStage
	{
	public:
		TerrainGPUStage(WorldSceneRenderCoordinator* coord);
		
		void setup() override;
		void lateSetup() override;
		void cleanup() override;

		vk::CommandBuffer* encode(RunOptions options) override;

	protected:
		friend gfx::Renderer;
		friend TerrainSystem;
		friend MaterialManager;

		void reEncodeBuffer(const Window& window, size_t surface);

		/// <summary>
		/// callled in main rander loop so any non trivial actions should be cojmpleted on a worker thread
		/// </summary>
		/// <param name="window"></param>
		/// <param name="surface"></param>
		void drawableReleased(Window* window, size_t appFrame) override;

		static const size_t setsOfCMDBuffers = 2;
		/// <summary>
		/// two sets of one for each drawable
		/// one of the two elements in the outer std::array is active and so will be sent to gpu to execute the other can be filled by worker thread
		/// </summary>
		std::array<std::vector<std::vector<vk::CommandPool  >>, setsOfCMDBuffers> cmdBufferPools{};
		std::array<std::vector<std::vector<vk::CommandBuffer>>, setsOfCMDBuffers> commandBuffers{};

		size_t mainThreadLocalCopyOfActiveBuffer = 0;
		libguarded::shared_guarded<size_t> activeBuffer = libguarded::shared_guarded<size_t>(0);

		gfx::DescriptorPool* descriptorPool;

		std::unordered_map<const Window*, std::vector<gfx::DescriptorSet*>> descriptorSets{};

		WorldSceneRenderCoordinator* worldCoord;

		/// <summary>
		/// TODO: not garentied that this will stay the same by render scene coordintaor but asuming it is
		/// </summary>
		size_t selfPass = 0;

	};

}

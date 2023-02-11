#pragma once

#include "srpch.h"
#include "Sunrise/materialSystem/MaterialSystem.h"
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

		std::unordered_map<const Window*, std::vector<gfx::DescriptorSet*>> descriptorSets{};

		static const size_t setsOfCMDBuffers = 2;

	protected:
		friend gfx::Renderer;
		friend TerrainSystem;
		friend MaterialManager;
		friend WorldSceneRenderCoordinator;

		void reEncodeBuffer(const Window& window, size_t surface);

		/// <summary>
		/// callled in main rander loop so any non trivial actions should be completed on a worker thread
		/// </summary>
		/// <param name="window"></param>
		/// <param name="surface"></param>
		void drawableReleased(Window* window, size_t surface) override;

		/// <summary>
		/// two sets of one for each drawable
		/// one of the two elements in the outer std::array is active and so will be sent to gpu to execute the other can be filled by worker thread
		/// </summary>
		std::array<std::vector<std::vector<vk::CommandPool  >>, setsOfCMDBuffers> cmdBufferPools{};
		std::array<std::vector<std::vector<vk::CommandBuffer>>, setsOfCMDBuffers> commandBuffers{};

		/// <summary>
		/// indicies of map keys: set (array), window (outer vector), swapchain image (inner vector)
		/// indicies of map values: window, swapchain image
		/// </summary>
		//libguarded::shared_guarded<std::unordered_map<std::tuple<Window*, size_t>,std::tuple<size_t,size_t,size_t>>> commandBuffersInUse{};

		/// <summary>
		/// keys are the sets of command buffs (std::array for commandBuffers varible)
		/// ref sys
		/// </summary>
		libguarded::shared_guarded<std::unordered_map<size_t, size_t>> commandBuffersInUse{};

		// used on single thread for ref system above
		std::unordered_map<Window*, std::vector<size_t>> setUsedBySurface{};


		size_t mainThreadLocalCopyOfActiveBuffer = 0;
		libguarded::shared_guarded<size_t> activeBuffer = libguarded::shared_guarded<size_t>(0);

		gfx::DescriptorPool* descriptorPool;


		WorldSceneRenderCoordinator* worldCoord;



		/// <summary>
		/// TODO: not garentied that this will stay the same by render scene coordintaor but asuming it is
		/// </summary>
		size_t selfPass = 0;

		MaterialSystem::System matSys;
	};

}

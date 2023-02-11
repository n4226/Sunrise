#pragma once

#include "srpch.h"

#include "Sunrise/Sunrise/graphics/vulkan/renderer/SceneRenderCoordinator.h"
#include "../gfxPipelines/StandardPBRPipeline.h"
#include "terrain/TerrainGPUStage.h"

#include "Sunrise/Sunrise/graphics/vulkan/GPU Stages/concrete/DeferredStage.h"
#include "../../graphics/vulkan/renderer/WorldUniformsCreator.h"

namespace sunrise {

	class TerrainGPUStage;

	struct RenderCommandMetrics {
		size_t vertCountMetric = 0;
		size_t triCountMetric = 0;
	};

	class WorldSceneRenderCoordinator: public gfx::SceneRenderCoordinator
	{
	public:
		WorldSceneRenderCoordinator(WorldScene* scene, gfx::Renderer* renderer);
		~WorldSceneRenderCoordinator();

		void createPasses() override;

		void createUniforms() override;

		//TODO: cant set array size to  TerrainGPUStage::setsOfCMDBuffers because circular dependancy
		std::array<std::vector<std::vector<libguarded::plain_guarded<RenderCommandMetrics>*>>,5> terrainCommandMetrics{};
		bool terrainMetricsGood = false;
		size_t geActiveTerrainCommandIndex();
	private:
		friend TerrainGPUStage;
		friend DeferredStage;
		
		gfx::ComposableRenderPass::CreateOptions renderpassConfig(vk::Format swapChainFormat) override;

		void preEncodeUpdate(vk::CommandBuffer firstLevelCMDBuffer, size_t frameID, Window& window) override;


		void updateSceneUniformBuffer(Window& window) override;

		TerrainGPUStage* terrainStage;

		WorldScene* worldScene;

	};

}
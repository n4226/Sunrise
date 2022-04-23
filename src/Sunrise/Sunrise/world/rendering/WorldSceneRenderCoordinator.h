#pragma once

#include "srpch.h"

#include "Sunrise/Sunrise/graphics/vulkan/renderer/SceneRenderCoordinator.h"
#include "../gfxPipelines/WorldTerrainPipeline.h"
#include "terrain/TerrainGPUStage.h"

#include "Sunrise/Sunrise/graphics/vulkan/GPU Stages/concrete/DeferredStage.h"
#include "../../graphics/vulkan/renderer/WorldUniformsCreator.h"

namespace sunrise {

	class TerrainGPUStage;

	class WorldSceneRenderCoordinator: public gfx::SceneRenderCoordinator
	{
	public:
		WorldSceneRenderCoordinator(WorldScene* scene, gfx::Renderer* renderer);
		~WorldSceneRenderCoordinator();

		void createPasses() override;

		void createUniforms() override;
	private:
		friend TerrainGPUStage;
		friend DeferredStage;
		
		gfx::ComposableRenderPass::CreateOptions renderpassConfig(vk::Format swapChainFormat) override;

		void preEncodeUpdate(vk::CommandBuffer firstLevelCMDBuffer, size_t frameID, Window& window) override;


		void updateSceneUniformBuffer(Window& window) override;

		WorldScene* worldScene;


	};

}
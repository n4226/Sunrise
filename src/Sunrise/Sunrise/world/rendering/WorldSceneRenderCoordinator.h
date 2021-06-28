#pragma once

#include "srpch.h"

#include "Sunrise/Sunrise/graphics/vulkan/renderer/SceneRenderCoordinator.h"
#include "../gfxPipelines/WorldTerrainPipeline.h"
#include "terrain/TerrainGPUStage.h"

#include "Sunrise/Sunrise/graphics/vulkan/GPU Stages/concrete/DeferredStage.h"

namespace sunrise {

	class TerrainGPUStage;

	class WorldSceneRenderCoordinator: public gfx::SceneRenderCoordinator
	{
	public:
		WorldSceneRenderCoordinator(WorldScene* scene);
		~WorldSceneRenderCoordinator();

		virtual void createPasses() override;

		void createUniforms();
	private:
		friend TerrainGPUStage;
		friend DeferredStage;
		
		gfx::ComposableRenderPass::CreateOptions renderpassConfig(vk::Format swapChainFormat) override;

		void preEncodeUpdate(gfx::Renderer* renderer, vk::CommandBuffer firstLevelCMDBuffer, size_t frameID, Window& window) override;


		void updateSceneUniformBuffer(Window& window);

		WorldScene* worldScene;

		// scene uniforms
		
		// one per surface
		std::vector<std::vector<gfx::Buffer*>> uniformBuffers;

	};

}
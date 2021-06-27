#pragma once

#include "srpch.h"

#include "Sunrise/Sunrise/graphics/vulkan/renderer/SceneRenderCoordinator.h"

namespace sunrise {

	class WorldSceneRenderCoordinator: public gfx::SceneRenderCoordinator
	{
	public:
		using gfx::SceneRenderCoordinator::SceneRenderCoordinator;
		~WorldSceneRenderCoordinator();

		virtual void createPasses() override;

	protected:
		
		gfx::ComposableRenderPass::CreateOptions renderpassConfig(vk::Format swapChainFormat) override;

		void preFrameUpdate() override;

	};

}
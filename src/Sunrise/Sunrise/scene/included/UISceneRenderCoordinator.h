#pragma once

#include "srpch.h"

#include "Sunrise/graphics/vulkan/renderer/SceneRenderCoordinator.h"

namespace sunrise {
	class UISceneRenderCoordinator: public gfx::SceneRenderCoordinator {

	public:

		using gfx::SceneRenderCoordinator::SceneRenderCoordinator;

		virtual void createPasses() override;

		gfx::ComposableRenderPass::CreateOptions renderpassConfig(vk::Format swapChainFormat) override;

	};
}

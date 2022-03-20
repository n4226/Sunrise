#pragma once

#include "srpch.h"

#include "Sunrise/graphics/vulkan/renderer/SceneRenderCoordinator.h"

namespace sunrise {
	/*nnamespace gfx {
		class SceneRenderCoordinator;
	}*/
	class DefaultSceneRenderCoordinator : public gfx::SceneRenderCoordinator {

	public:

		using gfx::SceneRenderCoordinator::SceneRenderCoordinator;

		virtual void createPasses() override;

		void createUniforms() override;
		void updateSceneUniformBuffer(Window& window) override;

		gfx::ComposableRenderPass::CreateOptions renderpassConfig(vk::Format swapChainFormat) override;

	};
}

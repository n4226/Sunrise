#pragma once

#include "srpch.h"

#include "../vulkan/GPU Stages/GPURenderStage.h"

namespace sunrise::gfx {
	/// <summary>
	/// special - no lifecycle events are gaurented to be called - only encode
	/// </summary>
	class ImGuiStage: public GPURenderStage
	{
	public:

		ImGuiStage(gfx::SceneRenderCoordinator* coord);

	protected:

		friend SceneRenderCoordinator;

		void setup() override;
		void cleanup() override;
		// called every frame
		vk::CommandBuffer* encode(RunOptions options) override;

	private:

	};
}

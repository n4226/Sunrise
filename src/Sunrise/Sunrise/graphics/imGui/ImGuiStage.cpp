#include "srpch.h"

#include "ImGuiStage.h"

#include "Sunrise/math/mesh/MeshPrimatives.h"
#include "Sunrise/core/Application.h"

#include "backends/imgui_impl_vulkan.h"

namespace sunrise::gfx {

	ImGuiStage::ImGuiStage(gfx::SceneRenderCoordinator* coord)
		: GPURenderStage(coord, "ImGUI Render Stage")
	{

	}

	void ImGuiStage::setup()
	{
		
	}

	void ImGuiStage::cleanup()
	{

	}

	vk::CommandBuffer* ImGuiStage::encode(RunOptions options)
	{
		auto buff = selectAndSetupCommandBuff(options);

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *buff);

		buff->end();

		return buff;

	}

}
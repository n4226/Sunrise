#include "srpch.h"
#include "WorldSceneRenderCoordinator.h"

#include "../gfxPipelines/WorldTerrainPipeline.h"
#include "terrain/TerrainGPUStage.h"
#include "Sunrise/Sunrise/graphics/vulkan/GPU Stages/concrete/DeferredStage.h"

namespace sunrise {
	WorldSceneRenderCoordinator::~WorldSceneRenderCoordinator()
	{
		
	}
	
	void WorldSceneRenderCoordinator::createPasses()
	{
		//https://stackoverflow.com/questions/8192185/using-stdarray-with-initialization-lists
		std::vector<GPUStageDispatcher::DependencyOptions> gbuffToDeferredOptions = { {
			{0, vk::ImageLayout::eColorAttachmentOptimal, vk::AttachmentLoadOp::eClear},
			{1, vk::ImageLayout::eShaderReadOnlyOptimal, vk::AttachmentLoadOp::eClear},
			{2, vk::ImageLayout::eShaderReadOnlyOptimal, vk::AttachmentLoadOp::eClear},
			{3, vk::ImageLayout::eShaderReadOnlyOptimal, vk::AttachmentLoadOp::eClear}
		} };

		//todo: store this to then delete it in destructor or cleanup method
		auto terrainStage = new TerrainGPUStage(app);
		auto deferredStage = new DeferredStage(app);


		registerStage(terrainStage, {}, {}, {});
		registerStage(deferredStage, { terrainStage }, std::move(gbuffToDeferredOptions), {});

		setLastStage(deferredStage);

	}


	void WorldSceneRenderCoordinator::preFrameUpdate()
	{

	}


	gfx::ComposableRenderPass::CreateOptions WorldSceneRenderCoordinator::renderpassConfig(vk::Format swapChainFormat)
	{
		auto deferredA = gfx::ComposableRenderPass::CreateOptions::VAttatchment();

		deferredA.type = gfx::ComposableRenderPass::CreateOptions::AttatchmentType::Color;
		deferredA.format = swapChainFormat;
		deferredA.loadOp = vk::AttachmentLoadOp::eClear;
		deferredA.initialLayout = vk::ImageLayout::eUndefined;
		deferredA.transitionalToAtStartLayout = vk::ImageLayout::eUndefined;//vk::ImageLayout::eColorAttachmentOptimal;
		//deferredA.transitionalToAtStartLayout = vk::ImageLayout::eColorAttachmentOptimal;//vk::ImageLayout::eColorAttachmentOptimal;
		deferredA.finalLayout = vk::ImageLayout::ePresentSrcKHR;
		deferredA.clearColor = { 0.8f, 0.2f, 0.0f, 1.0f };
		deferredA.name = "FinalRenderTarget";


		auto gbuffer_albedo_metallic = gfx::ComposableRenderPass::CreateOptions::VAttatchment();

		gbuffer_albedo_metallic.type = gfx::ComposableRenderPass::CreateOptions::AttatchmentType::Color;
		gbuffer_albedo_metallic.format = vk::Format::eB8G8R8A8Unorm;
		gbuffer_albedo_metallic.loadOp = vk::AttachmentLoadOp::eClear;
		gbuffer_albedo_metallic.initialLayout = vk::ImageLayout::eUndefined;
		gbuffer_albedo_metallic.transitionalToAtStartLayout = vk::ImageLayout::eColorAttachmentOptimal;
		gbuffer_albedo_metallic.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		gbuffer_albedo_metallic.clearColor = { 0.0f, 0.8f, 0.0f, 1.0f };
		gbuffer_albedo_metallic.usage |= vk::ImageUsageFlagBits::eSampled;
		//gbuffer_albedo_metallic.usage |= vk::ImageUsageFlagBits::eStorage;
		gbuffer_albedo_metallic.name = "gbuffer: albedo (b,g,r) and metallic (a)";


		auto gbuffer_normal_roughness = gfx::ComposableRenderPass::CreateOptions::VAttatchment();

		gbuffer_normal_roughness.type = gfx::ComposableRenderPass::CreateOptions::AttatchmentType::Color;
		gbuffer_normal_roughness.format = vk::Format::eB8G8R8A8Unorm;
		gbuffer_normal_roughness.loadOp = vk::AttachmentLoadOp::eClear;
		gbuffer_normal_roughness.initialLayout = vk::ImageLayout::eUndefined;
		gbuffer_normal_roughness.transitionalToAtStartLayout = vk::ImageLayout::eColorAttachmentOptimal;
		gbuffer_normal_roughness.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		gbuffer_normal_roughness.clearColor = { 0.0f, 0.8f, 0.0f, 1.0f };
		gbuffer_normal_roughness.usage |= vk::ImageUsageFlagBits::eSampled;
		//attach2.usage |= vk::ImageUsageFlagBits::eStorage;
		gbuffer_normal_roughness.name = "gbuffer: normal (b,g,r) and roughness (a)";


		auto gbuffer_ao = gfx::ComposableRenderPass::CreateOptions::VAttatchment();

		gbuffer_ao.type = gfx::ComposableRenderPass::CreateOptions::AttatchmentType::Color;
		gbuffer_ao.format = vk::Format::eR8Unorm;
		gbuffer_ao.loadOp = vk::AttachmentLoadOp::eClear;
		gbuffer_ao.initialLayout = vk::ImageLayout::eUndefined;
		gbuffer_ao.transitionalToAtStartLayout = vk::ImageLayout::eColorAttachmentOptimal;
		gbuffer_ao.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		gbuffer_ao.clearColor = { 0.0f, 0.8f, 0.0f, 1.0f };
		gbuffer_ao.usage |= vk::ImageUsageFlagBits::eSampled;
		//attach2.usage |= vk::ImageUsageFlagBits::eStorage;
		gbuffer_ao.name = "gbuffer: ao";

		//TODO: add depth buffer

		gfx::ComposableRenderPass::CreateOptions options = { {deferredA, gbuffer_albedo_metallic, gbuffer_normal_roughness, gbuffer_ao}, 0 };

		return options;
	}

}


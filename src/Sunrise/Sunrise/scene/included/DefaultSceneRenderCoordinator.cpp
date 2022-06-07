#include "srpch.h"

#include "DefaultSceneRenderCoordinator.h"
#include "entityRendering/EntityRenderingGPUStage.h"
#include "../../graphics/vulkan/GPU Stages/concrete/DeferredStage.h"
#include "../../world/gfxPipelines/StandardPBRPipeline.h"
#include "../../graphics/vulkan/renderer/WorldUniformsCreator.h"
#include "../../graphics/vulkan/renderPipelines/concrete/GPUStages/DeferredPipeline.h"

namespace sunrise {


	void sunrise::DefaultSceneRenderCoordinator::createPasses()
	{
		
		//https://stackoverflow.com/questions/8192185/using-stdarray-with-initialization-lists
		std::vector<GPUStageDispatcher::DependencyOptions> gbuffToDeferredOptions = { {
			{0, vk::ImageLayout::eColorAttachmentOptimal, vk::AttachmentLoadOp::eClear},
			{1, vk::ImageLayout::eShaderReadOnlyOptimal, vk::AttachmentLoadOp::eLoad},
			{2, vk::ImageLayout::eShaderReadOnlyOptimal, vk::AttachmentLoadOp::eLoad},
			{3, vk::ImageLayout::eShaderReadOnlyOptimal, vk::AttachmentLoadOp::eLoad},
			{4, vk::ImageLayout::eShaderReadOnlyOptimal, vk::AttachmentLoadOp::eLoad}
		} };

		auto entStage = new EntityRenderingGPUStage(this);
		auto deferredStage = new DeferredStage(this, { 1,2,3,4 });


		registerStage(entStage, {}, {}, {});
		registerStage(deferredStage, { entStage }, std::move(gbuffToDeferredOptions), {});

		setLastStage(deferredStage);

		generateImguiStage = true;
	}

	gfx::ComposableRenderPass::CreateOptions sunrise::DefaultSceneRenderCoordinator::renderpassConfig(vk::Format swapChainFormat)
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
		gbuffer_albedo_metallic.clearColor = { 0.6f, 0.0f, 0.0f, 1.0f };
		gbuffer_albedo_metallic.usage |= vk::ImageUsageFlagBits::eSampled;
		//gbuffer_albedo_metallic.usage |= vk::ImageUsageFlagBits::eStorage;
		gbuffer_albedo_metallic.name = "gBuffer: albedo (b,g,r) and metallic (a)";


		auto gbuffer_normal_roughness = gfx::ComposableRenderPass::CreateOptions::VAttatchment();

		gbuffer_normal_roughness.type = gfx::ComposableRenderPass::CreateOptions::AttatchmentType::Color;
		gbuffer_normal_roughness.format = vk::Format::eB8G8R8A8Unorm;
		gbuffer_normal_roughness.loadOp = vk::AttachmentLoadOp::eClear;
		gbuffer_normal_roughness.initialLayout = vk::ImageLayout::eUndefined;
		gbuffer_normal_roughness.transitionalToAtStartLayout = vk::ImageLayout::eColorAttachmentOptimal;
		gbuffer_normal_roughness.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		gbuffer_normal_roughness.clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
		gbuffer_normal_roughness.usage |= vk::ImageUsageFlagBits::eSampled;
		//attach2.usage |= vk::ImageUsageFlagBits::eStorage;
		gbuffer_normal_roughness.name = "gBuffer: normal (b,g,r) and roughness (a)";


		auto gbuffer_ao = gfx::ComposableRenderPass::CreateOptions::VAttatchment();

		gbuffer_ao.type = gfx::ComposableRenderPass::CreateOptions::AttatchmentType::Color;
		gbuffer_ao.format = vk::Format::eR8Unorm;
		gbuffer_ao.loadOp = vk::AttachmentLoadOp::eClear;
		gbuffer_ao.initialLayout = vk::ImageLayout::eUndefined;
		gbuffer_ao.transitionalToAtStartLayout = vk::ImageLayout::eColorAttachmentOptimal;
		gbuffer_ao.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		gbuffer_ao.clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
		gbuffer_ao.usage |= vk::ImageUsageFlagBits::eSampled;
		//attach2.usage |= vk::ImageUsageFlagBits::eStorage;
		gbuffer_ao.name = "gBuffer: ao";


		auto gbuffer_depth = gfx::ComposableRenderPass::CreateOptions::VAttatchment();

		auto depthBufferFormat =
			gfx::GPUSelector::findSupportedFormat(renderer->physicalDevice, { vk::Format::eD32Sfloat }, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);

		gbuffer_depth.type = gfx::ComposableRenderPass::CreateOptions::AttatchmentType::Depth;
		gbuffer_depth.format = depthBufferFormat;
		gbuffer_depth.loadOp = vk::AttachmentLoadOp::eClear;
		gbuffer_depth.initialLayout = vk::ImageLayout::eUndefined;
		gbuffer_depth.transitionalToAtStartLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
		gbuffer_depth.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		gbuffer_depth.clearDepthStencil = { { 1.f, 0 } };
		gbuffer_depth.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
		gbuffer_depth.name = "gBuffer: depth";


		//not using defered target currently;
		gfx::ComposableRenderPass::CreateOptions options = { {deferredA, gbuffer_albedo_metallic, gbuffer_normal_roughness, gbuffer_ao, gbuffer_depth}, 0 };

		return options;
	}

	void DefaultSceneRenderCoordinator::createUniforms()
	{
		WorldUniformCreator::createUniforms(renderer, uniformBuffers);
	}

	void DefaultSceneRenderCoordinator::updateSceneUniformBuffer(Window& window)
	{
		WorldUniformCreator::updateSceneUniformBuffer(window, { scene->sunLL,0 }, { 0,-math::fEarthRad,0 }, 100, uniformBuffers);
	}

}
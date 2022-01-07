#include "srpch.h"
#include "WorldSceneRenderCoordinator.h"


#include "Sunrise/Sunrise/graphics/vulkan/GPU Stages/concrete/DeferredStage.h"
#include "Sunrise/Sunrise/graphics/vulkan/resources/uniforms.h"
#include "Sunrise/Sunrise/core/Window.h"
#include "Sunrise/Sunrise/core/Application.h"
#include "../WorldScene.h"
#include"Sunrise/Sunrise/graphics/vulkan/renderPipelines/concrete/GPUStages/DeferredPipeline.h"
#include "Sunrise/Sunrise/graphics/vulkan/generalAbstractions/GPUSelector.h"

namespace sunrise {

	WorldSceneRenderCoordinator::WorldSceneRenderCoordinator(WorldScene* scene)
		: SceneRenderCoordinator(scene), worldScene(scene)
	{
	}

	WorldSceneRenderCoordinator::~WorldSceneRenderCoordinator()
	{
		for (auto buffer : uniformBuffers)
		{
			for (auto sbuffer : buffer)
				delete sbuffer;
		}
	}
	
	void WorldSceneRenderCoordinator::createPasses()
	{


		//https://stackoverflow.com/questions/8192185/using-stdarray-with-initialization-lists
		std::vector<GPUStageDispatcher::DependencyOptions> gbuffToDeferredOptions = { {
			{0, vk::ImageLayout::eColorAttachmentOptimal, vk::AttachmentLoadOp::eClear},
			{1, vk::ImageLayout::eShaderReadOnlyOptimal, vk::AttachmentLoadOp::eLoad},
			{2, vk::ImageLayout::eShaderReadOnlyOptimal, vk::AttachmentLoadOp::eLoad},
			{3, vk::ImageLayout::eShaderReadOnlyOptimal, vk::AttachmentLoadOp::eLoad},
			{4, vk::ImageLayout::eShaderReadOnlyOptimal, vk::AttachmentLoadOp::eLoad}
		} };

		//todo: store this to then delete it in destructor or cleanup method
		auto terrainStage = new TerrainGPUStage(this);
		auto deferredStage = new DeferredStage(this, { 1,2,3,4 });

		registerPipeline(worldTerrainPipeline,terrainStage);
		registerPipeline(deferredPipeline,deferredStage);

		registerStage(terrainStage, {}, {}, {});
		registerStage(deferredStage, { terrainStage }, std::move(gbuffToDeferredOptions), {});

		setLastStage(deferredStage);
	}


	void WorldSceneRenderCoordinator::preEncodeUpdate(gfx::Renderer* renderer, vk::CommandBuffer firstLevelCMDBuffer, size_t frameID, Window& window)
	{
		// updateDescriptors
		updateSceneUniformBuffer(window);
	}

	void WorldSceneRenderCoordinator::createUniforms()
	{
		using namespace gfx;
		PROFILE_FUNCTION;

		auto renderer = app.renderers[0];

		//TODO test using a staging buff

		// make uniforms
		VkDeviceSize uniformBufferSize = sizeof(SceneUniforms) + sizeof(PostProcessEarthDatAndUniforms);


		BufferCreationOptions uniformOptions = { ResourceStorageType::cpuToGpu,{vk::BufferUsageFlagBits::eUniformBuffer}, vk::SharingMode::eExclusive };

		uniformBuffers.resize(renderer->physicalWindows.size());


		for (size_t i = 0; i < uniformBuffers.size(); i++) {
			uniformBuffers[i].resize(app.MAX_FRAMES_IN_FLIGHT);

			for (size_t e = 0; e < uniformBuffers[i].size(); e++)
				uniformBuffers[i][e] = new Buffer(renderer->device, renderer->allocator, uniformBufferSize, uniformOptions);
		}
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
		gbuffer_albedo_metallic.clearColor = { 0.6f, 0.0f, 0.0f, 1.0f };
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
		gbuffer_normal_roughness.clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
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
		gbuffer_ao.clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
		gbuffer_ao.usage |= vk::ImageUsageFlagBits::eSampled;
		//attach2.usage |= vk::ImageUsageFlagBits::eStorage;
		gbuffer_ao.name = "gbuffer: ao";

		
		auto gbuffer_depth = gfx::ComposableRenderPass::CreateOptions::VAttatchment();

		auto depthBufferFormat =
			gfx::GPUSelector::findSupportedFormat(app.renderers[0]->physicalDevice, { vk::Format::eD32Sfloat }, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);

		gbuffer_depth.type = gfx::ComposableRenderPass::CreateOptions::AttatchmentType::Depth;
		gbuffer_depth.format = depthBufferFormat;
		gbuffer_depth.loadOp = vk::AttachmentLoadOp::eClear;
		gbuffer_depth.initialLayout = vk::ImageLayout::eUndefined;
		gbuffer_depth.transitionalToAtStartLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
		gbuffer_depth.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		gbuffer_depth.clearDepthStencil = { { 1.f, 0 } };
		gbuffer_depth.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
		gbuffer_depth.name = "gbuffer: depth";


		gfx::ComposableRenderPass::CreateOptions options = { {deferredA, gbuffer_albedo_metallic, gbuffer_normal_roughness, gbuffer_ao, gbuffer_depth}, 0 };

		return options;
	}


	void WorldSceneRenderCoordinator::updateSceneUniformBuffer(Window& window)
	{
		using namespace gfx;
		PROFILE_FUNCTION;
		// update uniform buffer

		auto renderer = window.renderer;

		auto globalIndex = window.globalIndex;
		auto& camera = window.camera;

		SceneUniforms uniforms;

		uniforms.viewProjection = camera.viewProjection(window.swapchainExtent.width, window.swapchainExtent.height);

		auto buffer = uniformBuffers[globalIndex][window.currentSurfaceIndex];

		buffer->mapMemory();

		buffer->tempMapAndWrite(&uniforms, 0, sizeof(uniforms), false);

		PostProcessEarthDatAndUniforms postUniforms;

		// in floated origin (after float)
		postUniforms.camFloatedGloabelPos = glm::vec4(camera.transform.position, 1);
		glm::qua<glm::float32> sunRot = glm::angleAxis(glm::radians(45.f), glm::vec3(0, 1, 0));

		//todo abstract this out
		postUniforms.sunDir =
			//glm::angleAxis(glm::radians(sin(scene->timef * 0.1f) * 80.f), glm::vec3(-1, 0, 0)) *
			glm::vec4(glm::normalize(math::LlatoGeo(worldScene->initialPlayerLLA, glm::dvec3(0), worldScene->terrainSystem->getRadius())), 0);

		postUniforms.earthCenter = glm::vec4(static_cast<glm::vec3>(-(worldScene->origin)), 1);

		postUniforms.viewMat = camera.view();
		postUniforms.projMat = camera.projection(window.swapchainExtent.width, window.swapchainExtent.height);
		postUniforms.invertedViewMat = glm::inverse(camera.view());
		postUniforms.invertedProjMat = glm::inverse(camera.projection(window.swapchainExtent.width, window.swapchainExtent.height));
		postUniforms.renderTargetSize.x = window.swapchainExtent.width;
		postUniforms.renderTargetSize.y = window.swapchainExtent.height;

		buffer->tempMapAndWrite(&postUniforms, sizeof(uniforms), sizeof(postUniforms), false);

		buffer->unmapMemory();

		//todo abstract this somewhere else
		renderer->camFrustroms[globalIndex] = std::move(math::Frustum(uniforms.viewProjection));
	}
}


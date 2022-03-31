#include <srpch.h>
#include "WorldUniformsCreator.h"
#include "../resources/uniforms.h"
#include "Sunrise/core/Application.h"

namespace sunrise {

	void WorldUniformCreator::updateSceneUniformBuffer(Window& window, glm::dvec3 sunLLA, glm::dvec3 earthCenter, double earchRadius, std::vector<std::vector<gfx::Buffer*>>& uniformBuffers)
	{
		using namespace gfx;
		PROFILE_FUNCTION;
		// update uniform buffer

		auto renderer = window.renderer;

		auto winIndex = window.indexInRenderer;
		//auto& camera = window.isVirtual() ? window.subWindows[0]->camera : window.camera;

		auto count = window.isVirtual() ? window.subWindows.size() : 1;
		//TODO: pass matricides for each sub window	

		SceneUniforms uniforms;
		for (int i = 0; i < count; i++)
		{
			auto& refWindow = window.isVirtual() ? *window.subWindows[i] : window;
			auto& refCamera = refWindow.camera;

			uniforms.viewProjection[i] = refCamera.viewProjection(refWindow.swapchainExtent.width, refWindow.swapchainExtent.height);
		}

		auto buffer = uniformBuffers[winIndex][window.currentSurfaceIndex];

		buffer->mapMemory();

		buffer->tempMapAndWrite(&uniforms, 0, sizeof(uniforms), false);

		PostProcessEarthDatAndUniforms postUniforms;

		for (int i = 0; i < count; i++)
		{
			auto& refWindow = window.isVirtual() ? *window.subWindows[i] : window;
			auto& refCamera = refWindow.camera;

			// in floated origin (after float)
			postUniforms.camFloatedGloabelPos[i] = glm::vec4(refCamera.transform.position, 1);
			glm::qua<glm::float32> sunRot = glm::angleAxis(glm::radians(45.f), glm::vec3(0, 1, 0));



			postUniforms.viewMat[i] = refCamera.view();
			postUniforms.projMat[i] = refCamera.projection(window.swapchainExtent.width, window.swapchainExtent.height);
			postUniforms.invertedViewMat[i] = glm::inverse(refCamera.view());
			postUniforms.invertedProjMat[i] = glm::inverse(refCamera.projection(window.swapchainExtent.width, window.swapchainExtent.height));

		}

		//todo abstract this out
		postUniforms.sunDir =
			//glm::angleAxis(glm::radians(sin(scene->timef * 0.1f) * 80.f), glm::vec3(-1, 0, 0)) *
			glm::vec4(glm::normalize(math::LlatoGeo(sunLLA, glm::dvec3(0), earchRadius)), 0);
		postUniforms.earthCenter = glm::vec4(static_cast<glm::vec3>((earthCenter)), 1);

		postUniforms.renderTargetSize.x = window.swapchainExtent.width;
		postUniforms.renderTargetSize.y = window.swapchainExtent.height;

		buffer->tempMapAndWrite(&postUniforms, sizeof(uniforms), sizeof(postUniforms), false);

		buffer->unmapMemory();

		//todo abstract this somewhere else
		// TODO: fix for virtual windows
		//renderer->camFrustroms[globalIndex] = std::move(math::Frustum(uniforms.viewProjection));
	}

	void WorldUniformCreator::createUniforms(Application& app, std::vector<std::vector<gfx::Buffer*>>& uniformBuffers)
{
		using namespace gfx;
		PROFILE_FUNCTION;

		auto renderer = app.renderers[0];

		//TODO test using a staging buff

		// make uniforms
		VkDeviceSize uniformBufferSize = sizeof(SceneUniforms) + sizeof(PostProcessEarthDatAndUniforms);


		BufferCreationOptions uniformOptions = { ResourceStorageType::cpuToGpu,{ vk::BufferUsageFlagBits::eUniformBuffer }, vk::SharingMode::eExclusive };

		//for virtual windows there will just be data for cams in sub windows
		uniformBuffers.resize(renderer->windows.size());


		for (size_t i = 0; i < uniformBuffers.size(); i++) {
			uniformBuffers[i].resize(app.MAX_FRAMES_IN_FLIGHT);

			for (size_t e = 0; e < uniformBuffers[i].size(); e++)
				uniformBuffers[i][e] = new Buffer(renderer->device, renderer->allocator, uniformBufferSize, uniformOptions);
		}
	}

}
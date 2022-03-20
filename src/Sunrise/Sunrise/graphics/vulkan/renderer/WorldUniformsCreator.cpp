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
			glm::vec4(glm::normalize(math::LlatoGeo(sunLLA, glm::dvec3(0), earchRadius)), 0);

		postUniforms.earthCenter = glm::vec4(static_cast<glm::vec3>((earthCenter)), 1);

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

	void WorldUniformCreator::createUniforms(Application& app, std::vector<std::vector<gfx::Buffer*>>& uniformBuffers)
{
		using namespace gfx;
		PROFILE_FUNCTION;

		auto renderer = app.renderers[0];

		//TODO test using a staging buff

		// make uniforms
		VkDeviceSize uniformBufferSize = sizeof(SceneUniforms) + sizeof(PostProcessEarthDatAndUniforms);


		BufferCreationOptions uniformOptions = { ResourceStorageType::cpuToGpu,{ vk::BufferUsageFlagBits::eUniformBuffer }, vk::SharingMode::eExclusive };

		uniformBuffers.resize(renderer->physicalWindows.size());


		for (size_t i = 0; i < uniformBuffers.size(); i++) {
			uniformBuffers[i].resize(app.MAX_FRAMES_IN_FLIGHT);

			for (size_t e = 0; e < uniformBuffers[i].size(); e++)
				uniformBuffers[i][e] = new Buffer(renderer->device, renderer->allocator, uniformBufferSize, uniformOptions);
		}
	}

}
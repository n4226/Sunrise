#pragma once

#include "srpch.h"

#include "Sunrise/graphics/vulkan/GPU Stages/GPURenderStage.h"

#include "Sunrise/graphics/vulkan/renderer/SceneRenderCoordinator.h"
#include "Sunrise/scene/Scene.h"
#include "Sunrise/scene/Transform.h"
#include "Sunrise/core/Application.h"
#include "Sunrise/graphics/vulkan/resources/uniforms.h"

namespace sunrise {


	/*
	* USING CPU MEMLORY FOR INDEX AND VERTEX BUFFERS ----- FIX ME SOON
	*/
	class EntityRenderingGPUStage: public gfx::GPURenderStage {
	public:
		EntityRenderingGPUStage(gfx::SceneRenderCoordinator* coord);

		std::unordered_map<const Window*, std::vector<gfx::DescriptorSet*>> descriptorSets{};
	protected:

		void setup() override;

		void loadMeshResources();

		void CreateTransformInGlobal(gfx::Renderer* renderer, Entity ent, const Transform& transform);

		void updateModelUniform(gfx::Renderer* renderer, gfx::ModelUniforms mtrans, size_t modelIndex, Entity ent);

		virtual void lateSetup() override;

		void AllocateDescriptors();

		void createDescriptorPool();

		void cleanup() override;
		// called every frame
		vk::CommandBuffer* encode(RunOptions options) override;

	private:

		gfx::DescriptorPool* descriptorPool;


		std::unordered_map<Entity, gfx::MeshBuffer*> meshBuffers;
		std::unordered_map<Entity, size_t> modelUniformIndicies;
		std::unordered_map<Entity, glm::mat4> modelMatricies;
	};

}
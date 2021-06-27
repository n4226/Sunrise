#pragma once

#include "srpch.h"
#include "../../graphics/vulkan/renderer/RenderSystem.h"
#include "TerrainQuadTree.h"
#include "TerrainQuadTreeNode.h"
#include "TerrainMeshLoader.h"
#include "../../scene/Transform.h"
#include "../../graphics/vulkan/generalAbstractions/VkAbstractions.h"


namespace sunrise {

	class FloatingOriginSystem;
	class Window;

	namespace gfx {
		class Renderer;
	}

	class TerrainSystem : public System
	{
	public:

#pragma region base Methods
		TerrainSystem(Application& app, WorldScene& scene, glm::dvec3* origin);
		~TerrainSystem();

		void CreateRenderResources();

		// todo remove this function as moving to GPU-stage system
		vk::CommandBuffer* renderSystem(uint32_t subpass, Window& window);
		
		void update() override;
		void invalidateDescriptors();

		double getRadius();

#pragma endregion

		Transform* trackedTransform = nullptr;
		glm::dvec3* origin = nullptr;

		WorldScene& scene;
		Application& app;

		//Renderer* renderer;

		// max lod levels for 
		const uint16_t lodLevels = 13;

	private:

#pragma region Resources

		// render Resources

		/// <summary>
		/// one for each drawable
		/// </summary>
		std::vector<std::vector<vk::CommandPool  >> cmdBufferPools;
		std::vector<std::vector<vk::CommandBuffer>> commandBuffers;

		libguarded::plain_guarded<std::map<TerrainQuadTreeNode*, TreeNodeDrawData>> drawObjects;
		libguarded::shared_guarded<std::unordered_map<TerrainQuadTreeNode*, TreeNodeDrawResaourceToCoppy>> loadedMeshesToDraw = {};

		bool destroyAwaitingNodes = false;
		libguarded::shared_guarded<bool> safeToModifyChunks = libguarded::shared_guarded<bool>(true);

#if RenderMode == RenderModeCPU2
		libguarded::shared_guarded<bool> drawCommandsValid = libguarded::shared_guarded<bool>(false);
		std::vector<std::vector<bool>> cmdBuffsUpToDate;
#endif
		libguarded::shared_guarded<std::map<TerrainQuadTreeNode*, TreeNodeDrawData>> pendingDrawObjects;

		//async resources
		marl::Ticket::Queue ticketQueue;

		// temp resources
		std::set<TerrainQuadTreeNode*> toSplit = {};
		std::set<TerrainQuadTreeNode*> toCombine = {};
		std::set<TerrainQuadTreeNode*> toDestroyDraw = {};
		std::set<TerrainQuadTreeNode*> toDrawDraw = {};

#pragma endregion

#pragma region Tree Traversal and Updating Methods

		void processTree();
		
		double threshold(const TerrainQuadTreeNode* node);

		bool determinActive(const TerrainQuadTreeNode* node);

		void setActiveState(TerrainQuadTreeNode* node);
		
#pragma endregion

		void writePendingDrawOobjects(gfx::Renderer& renderer);

		TerrainQuadTree tree;
		TerrainMeshLoader meshLoader;


		friend FloatingOriginSystem;
		friend TerrainMeshLoader;


	};

}
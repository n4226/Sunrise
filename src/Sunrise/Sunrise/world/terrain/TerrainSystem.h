#pragma once

#include "srpch.h"
#include "../../graphics/vulkan/renderer/RenderSystem.h"
#include "TerrainQuadTree.h"
#include "TerrainQuadTreeNode.h"
#include "TerrainMeshLoader.h"
#include "../../scene/Transform.h"
#include "../../graphics/vulkan/generalAbstractions/VkAbstractions.h"
#include "../rendering/terrain/TerrainGPUStage.h"

namespace sunrise {

	class FloatingOriginSystem;
	class Window;

	namespace gfx {
		class Renderer;
	}


	/// <summary>
	/// in charge of managing terrain tree and dispatching chunk loading jobs
	/// render encoding is done by gpustage
	/// </summary>
	class TerrainSystem : public System
	{
	public:

#pragma region base Methods
		TerrainSystem(Application& app, WorldScene& scene, glm::dvec3* origin);
		~TerrainSystem();

		void CreateRenderResources();

		// todo remove this function as moving to GPU-stage system
		//vk::CommandBuffer* renderSystem(uint32_t subpass, Window& window);
		
		void update() override;
		void invalidateDescriptors();

		double getRadius();

		void CreateTerrainInMask(sunrise::WorldScene& scene, sunrise::Application& app);
		void reloadTerrainInMask();
#pragma endregion

		Transform* trackedTransform = nullptr;
		glm::dvec3* origin = nullptr;

		WorldScene& scene;
		Application& app;

		//Renderer* renderer;

		// max lod levels for terrain - max lod is this - 1
		const uint16_t lodLevels = 14;

	private:

		friend TerrainGPUStage;

#pragma region Resources

		// render Resources

	/*	/// <summary>
		/// one for each drawable
		/// </summary>
		std::vector<std::vector<vk::CommandPool  >> cmdBufferPools;
		std::vector<std::vector<vk::CommandBuffer>> commandBuffers;*/

		libguarded::shared_guarded<std::map<TerrainQuadTreeNode*, TreeNodeDrawData>> drawObjects;
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

		bool splitFormThreshold(const TerrainQuadTreeNode* node,const glm::dvec3& trackedPos);
		bool combineFromThreshold(const TerrainQuadTreeNode* node,const glm::dvec3& trackedPos);

		bool determinActive(const TerrainQuadTreeNode* node);

		void setActiveState(TerrainQuadTreeNode* node);
		
#pragma endregion

		void writePendingDrawOobjects(gfx::Renderer& renderer);

		TerrainQuadTree tree;
		TerrainMeshLoader meshLoader;


		friend FloatingOriginSystem;
		friend TerrainMeshLoader;
		friend TerrainGPUStage;

		/// <summary>
		/// callled in main rander loop so any non trivial actions should be cojmpleted on a worker thread
		/// </summary>
		/// <param name="window"></param>
		/// <param name="surface"></param>
		void resourcesReleased(Window* window, size_t surface);


		bool maskedMode = false;
	};

}
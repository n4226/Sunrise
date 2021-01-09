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

	class TerrainSystem : public RenderSystem
	{
	public:

		TerrainSystem(Application& app, WorldScene& scene, glm::dvec3* origin);
		~TerrainSystem();

		void CreateRenderResources();

		void update() override;
		vk::CommandBuffer* renderSystem(uint32_t subpass, Window& window) override;

		Transform* trackedTransform = nullptr;
		glm::dvec3* origin = nullptr;

		WorldScene& scene;
		Application& app;

		//Renderer* renderer;

		const uint16_t lodLevels = 13;

		double getRadius();

		void invalidateDescriptors();

	private:

		// temp resources

		std::set<TerrainQuadTreeNode*> toSplit = {};
		std::set<TerrainQuadTreeNode*> toCombine = {};
		std::set<TerrainQuadTreeNode*> toDestroyDraw = {};
		std::set<TerrainQuadTreeNode*> toDrawDraw = {};

		libguarded::shared_guarded<std::unordered_map<TerrainQuadTreeNode*, TreeNodeDrawResaourceToCoppy>> loadedMeshesToDraw = {};

		bool destroyAwaitingNodes = false;
		libguarded::shared_guarded<bool> safeToModifyChunks = libguarded::shared_guarded<bool>(true);

#if RenderMode == RenderModeCPU2
		libguarded::shared_guarded<bool> drawCommandsValid = libguarded::shared_guarded<bool>(false);
		std::vector<std::vector<bool>> cmdBuffsUpToDate;
#endif
		libguarded::shared_guarded<std::map<TerrainQuadTreeNode*, TreeNodeDrawData>> pendingDrawObjects;

		TerrainQuadTree tree;

		TerrainMeshLoader meshLoader;

		void processTree();


		double threshold(const TerrainQuadTreeNode* node);

		bool determinActive(const TerrainQuadTreeNode* node);

		void setActiveState(TerrainQuadTreeNode* node);

		//updating descriptors

		void writePendingDrawOobjects(gfx::Renderer& renderer);

		//async resources
		marl::Ticket::Queue ticketQueue;

		// Render Resources

		/// <summary>
		/// one for each drawable
		/// </summary>
		std::vector<std::vector<vk::CommandPool  >> cmdBufferPools;
		std::vector<std::vector<vk::CommandBuffer>> commandBuffers;

		/*vk::DescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;*/

		libguarded::plain_guarded<std::map<TerrainQuadTreeNode*, TreeNodeDrawData>> drawObjects;

		friend FloatingOriginSystem;
		friend TerrainMeshLoader;
	};

}
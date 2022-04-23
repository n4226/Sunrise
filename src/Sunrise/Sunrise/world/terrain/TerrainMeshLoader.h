#pragma once

#include "srpch.h"
#include "../../Math.h"
#include "../../graphics/vulkan/generalAbstractions/VkAbstractions.h"
#include "../../fileFormats/binary/BinaryMesh.h"
#include "TerrainQuadTreeNode.h"
#include "../../graphics/vulkan/resources/uniforms.h"
#include "../../graphics/vulkan/resources/MeshBuffers.h"

namespace sunrise {

	namespace gfx {
		class Renderer;
	}

	class TerrainSystem;

	struct SUNRISE_API TreeNodeDrawResaourceToCoppy
	{
		BinaryMeshSeirilizer* binMesh = nullptr;
		BinaryMeshAttrributes* binMeshAttributes = nullptr;
		Mesh* mesh = nullptr;
	};

	struct SUNRISE_API TreeNodeDrawData
	{
		size_t vertIndex;
		size_t vertcount;
		std::vector<size_t> indIndicies;
		std::vector<size_t> indexCounts;
		size_t totalIndexCount;

		std::vector<gfx::DrawPushData> drawDatas;

		//buffer recipts
		gfx::BindlessMeshBuffer::WriteTransactionReceipt meshRecipt;
		gfx::BindlessMeshBuffer::WriteLocation modelRecipt;

		//CullInfo
		glm::vec3 aabbMin;
		glm::vec3 aabbMax;
	};

	/// <summary>
	/// one per renderer/device
	/// </summary>
	class SUNRISE_API TerrainMeshLoader
	{
	public:
		Mesh* createChunkMesh(const TerrainQuadTreeNode& chunk);


		TreeNodeDrawResaourceToCoppy loadMeshPreDrawChunk(TerrainQuadTreeNode* node, bool inJob = false, bool diskOnly = false);
		void drawChunk(TerrainQuadTreeNode* node, TreeNodeDrawResaourceToCoppy preLoadedMesh, bool inJob = false);
		void removeDrawChunk(TerrainQuadTreeNode* node, bool inJob = false);

		gfx::Renderer* renderer;
	private:

		TerrainSystem* terrainSystem;


		friend TerrainSystem;
	};

}
#pragma once

#include "srpch.h"
#include "../../Math.h"
#include "../../graphics/vulkan/generalAbstractions/VkAbstractions.h"
#include "../../fileFormats/binary/BinaryMesh.h"
#include "TerrainQuadTreeNode.h"
#include "../../graphics/vulkan/resources/uniforms.h"
#include "../../graphics/vulkan/resources/MeshBuffers.h"

/* 
	TODO: make notes more perminent

	sctructure of terrain file systen(s)
	
	each module is defined as a folder with an index.terrain file
	modules can have sub modules
	
	in the engine dir is a file denoting all known modules

	the engine will load this adn acodiated modules to build an efficient tree to konw very quickly where the terrain for a spacific chunk is located

	one or sevral modules can be linked up to be for remote chunks loaded elswhere


	
	there can be extenral modules - should  be an external modules reference file type one of which hosted on official website to get current options
	engine funcitn to validate local modules for broken files: can fix module files and point out terrain files which have errors or are corrupt
	

*/



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
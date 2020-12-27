#include "srpch.h"
#include "TerrainQuadTree.h"
#include "TerrainQuadTreeNode.h"

namespace sunrise {

	TerrainQuadTree::TerrainQuadTree(double radius)
		: radius(radius)
	{
		glm::dvec2 rootTileSize(90, 90);
		rootNodes = {
				new TerrainQuadTreeNode(Box(glm::dvec2(-90,-90),  rootTileSize), nullptr, this),
				new TerrainQuadTreeNode(Box(glm::dvec2(-90,-180), rootTileSize), nullptr, this),
				new TerrainQuadTreeNode(Box(glm::dvec2(-90,   0), rootTileSize), nullptr, this),
				new TerrainQuadTreeNode(Box(glm::dvec2(-90,  90), rootTileSize), nullptr, this),
				new TerrainQuadTreeNode(Box(glm::dvec2(0,  90), rootTileSize), nullptr, this),
				new TerrainQuadTreeNode(Box(glm::dvec2(0,   0), rootTileSize), nullptr, this),
				new TerrainQuadTreeNode(Box(glm::dvec2(0, -90), rootTileSize), nullptr, this),
				new TerrainQuadTreeNode(Box(glm::dvec2(0,-180), rootTileSize), nullptr, this),
		};



		for (TerrainQuadTreeNode* node : rootNodes)
		{
			this->leafNodes.insert(node);
		}

	}

	TerrainQuadTree::~TerrainQuadTree()
	{
		for (TerrainQuadTreeNode* node : rootNodes)
		{
			delete node;
		}
	}

}
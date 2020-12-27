#pragma once

#include "srpch.h"

namespace sunrise {


	class TerrainQuadTreeNode;

	class TerrainQuadTree
	{
	public:

		TerrainQuadTree(double radius);
		~TerrainQuadTree();

		double radius;

		std::vector<TerrainQuadTreeNode*> rootNodes;

		std::set<TerrainQuadTreeNode*> leafNodes;


	};

}
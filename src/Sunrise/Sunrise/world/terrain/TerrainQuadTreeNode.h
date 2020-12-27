#pragma once

#include "srpch.h"
#include "../../Math.h"


namespace sunrise {

	class TerrainQuadTree;

	class SUNRISE_API TerrainQuadTreeNode
	{
	public:

		TerrainQuadTreeNode(math::Box frame, TerrainQuadTreeNode* parent, TerrainQuadTree* tree, uint16_t lodLevel = 0);
		~TerrainQuadTreeNode();
		math::Box frame;
		glm::dvec3 center_geo;
		glm::dvec3 start_geo;
		glm::dvec3 end_geo;

		TerrainQuadTreeNode* parent;
		bool isSplit = false;
		//TODO: it might be better to not use pointers for all children but i'm doing this since set is acting weird with deleting node objects - definitly user error though
		std::vector<TerrainQuadTreeNode*> children;

		// stored properties 

		uint16_t lodLevel = 0;
		bool visable;
		bool active = true;

		TerrainQuadTree* tree;

		// render resources

		bool hasdraw = false;

		// lyfcycle events

		void split();
		void combine();

		void willBeCombined();

		bool operator<(const TerrainQuadTreeNode& rhs) const;
	};

}
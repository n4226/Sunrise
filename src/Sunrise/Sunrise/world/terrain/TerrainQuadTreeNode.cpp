#include "srpch.h"
#include "TerrainQuadTreeNode.h"
#include "TerrainQuadTree.h"

namespace sunrise {

	using namespace math;

	TerrainQuadTreeNode::TerrainQuadTreeNode(Box frame, TerrainQuadTreeNode* parent, TerrainQuadTree* tree, uint16_t lodLevel)
		: frame(frame),
		center_geo(LlatoGeo(glm::dvec3(frame.getCenter(), 0), {}, tree->radius)),
		start_geo (LlatoGeo(glm::dvec3(frame.start, 0), {}, tree->radius)),
		end_geo   (LlatoGeo(glm::dvec3(frame.getEnd(), 0), {}, tree->radius)),
		parent(parent), tree(tree), lodLevel(lodLevel), children(), visable(frame.size.x < 30)
	{

	}

	TerrainQuadTreeNode::~TerrainQuadTreeNode()
	{
		for (TerrainQuadTreeNode* node : children)
		{
			delete node;
		}

	}

	void TerrainQuadTreeNode::split()
	{
		PROFILE_FUNCTION;

		if (isSplit) return;
		isSplit = true;

		auto chldSize = frame.size / 2.0;

		children.reserve(4);

		children.push_back(new TerrainQuadTreeNode(Box(frame.start, chldSize), this, tree, lodLevel + 1));
		children.push_back(new TerrainQuadTreeNode(Box(frame.start + glm::dvec2(0, frame.size.y / 2), chldSize), this, tree, lodLevel + 1));
		children.push_back(new TerrainQuadTreeNode(Box(frame.start + glm::dvec2(frame.size.x / 2, frame.size.y / 2), chldSize), this, tree, lodLevel + 1));
		children.push_back(new TerrainQuadTreeNode(Box(frame.start + glm::dvec2(frame.size.x / 2, 0), chldSize), this, tree, lodLevel + 1));

		if (tree->leafNodes.count(this) > 0) {
			tree->leafNodes.erase(this);
			tree->leafNodes.insert(children[0]);
			tree->leafNodes.insert(children[1]);
			tree->leafNodes.insert(children[2]);
			tree->leafNodes.insert(children[3]);
		}
	}

	void TerrainQuadTreeNode::combine()
	{
		PROFILE_FUNCTION
			if (!isSplit) return;
		isSplit = false;

		for (TerrainQuadTreeNode* child : children)
		{
			child->willBeCombined();
			tree->leafNodes.erase(child);
			assert(child->children.size() == 0);
			delete child;
		}
		children.clear();

		tree->leafNodes.insert(this);
	}

	void TerrainQuadTreeNode::willBeCombined()
	{

	}

	bool TerrainQuadTreeNode::operator<(const TerrainQuadTreeNode& rhs) const
	{
		return
			frame.start.x < rhs.frame.start.x&& frame.size.x < rhs.frame.size.x&&
			frame.start.y < rhs.frame.start.y&& frame.size.y < rhs.frame.size.y;
	}

}
#include "srpch.h"
#include "Mesh.h"

#include "../../graphics/vulkan/generalAbstractions/AttributeHelpers.h"
#include "../../scene/Transform.h"
#include "../../graphics/vulkan/renderer/Renderer.h"

namespace sunrise {

	using namespace gfx;

	size_t Mesh::vertsSize() const
	{
		return verts.size() * sizeof(glm::vec3);
	}

	size_t Mesh::uvsSize() const
	{
		return uvs.size() * sizeof(glm::vec2);
	}

	size_t Mesh::normalsSize() const
	{
		return normals.size() * sizeof(glm::vec3);
	}

	size_t Mesh::tangentsSize() const
	{
		return tangents.size() * sizeof(glm::vec3);
	}

	size_t Mesh::bitangentsSize() const
	{
		return bitangents.size() * sizeof(glm::vec3);
	}

	/*size_t Mesh::indiciesSize()
	{
		return indicies.size() * sizeof(glm::uint32);
	}*/


	size_t Mesh::indiciesSize(size_t subMesh) const
	{
		return (indicies[subMesh].size()) * sizeof(glm::uint32);
	}

	size_t Mesh::AllSubMeshIndiciesSize() const {
		size_t total = 0;

		for (size_t i = 0; i < indicies.size(); i++)
		{
			total += indiciesSize(i);
		}
		return total;
	}

	size_t Mesh::vertsOffset() const
	{
		return 0;
	}

	size_t Mesh::uvsOffset() const
	{
		return vertsOffset() + vertsSize();
	}

	size_t Mesh::normalsOffset() const
	{
		return uvsOffset() + uvsSize();
	}

	size_t Mesh::tangentsOffset() const 
	{
		return normalsOffset() + normalsSize();
	}

	size_t Mesh::bitangentsOffset() const
	{
		return tangentsOffset() + tangentsSize();
	}

	size_t Mesh::indiciesOffset() const
	{
		return bitangentsOffset() + bitangentsSize();
	}

	size_t Mesh::indiciesOffset(int submesh) const
	{
		SR_ASSERT(indicies.size() > 0);
		auto totalSizeBefore = 0;
		for (size_t i = 0; i < submesh; i++)
		{
			totalSizeBefore += indiciesSize(i);
		}
		return indiciesOffset() + totalSizeBefore;
	}

	size_t Mesh::fullSize() const
	{
		return indiciesOffset() + AllSubMeshIndiciesSize();
	}

	void Mesh::calculateTangentsAndBitangents()
	{
		tangents.clear();
		bitangents.clear();

		SR_ASSERT(normals.size() == verts.size() && uvs.size() == verts.size());

		tangents.resize(verts.size(), glm::vec3(0));
		bitangents.resize(verts.size(), glm::vec3(0));

		//TODO: test for multi submesh meshes
		for (int subMesh = 0; subMesh < indicies.size(); subMesh++)
		{
			std::vector<glm::vec3> tan1(indicies[subMesh].size());
			std::vector<glm::vec3> tan2(indicies[subMesh].size());

			for (int tri = 0; tri < indicies[subMesh].size(); tri += 3) {

				auto i1 = indicies[subMesh][tri + 0];
				auto i2 = indicies[subMesh][tri + 1];
				auto i3 = indicies[subMesh][tri + 2];

				// Shortcuts for vertices
				glm::vec3& v1 = verts[i1];
				glm::vec3& v2 = verts[i2];
				glm::vec3& v3 = verts[i3];

				// Shortcuts for UVs
				glm::vec2& w1 = uvs[i1];
				glm::vec2& w2 = uvs[i2];
				glm::vec2& w3 = uvs[i3];

				//http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/
				//// Edges of the triangle : position delta
				//glm::vec3 deltaPos1 = v1 - v0;
				//glm::vec3 deltaPos2 = v2 - v0;

				//// UV delta
				//glm::vec2 deltaUV1 = uv1 - uv0;
				//glm::vec2 deltaUV2 = uv2 - uv0;


				//float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
				//glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
				//glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;
				
				
				//https://gamedev.stackexchange.com/questions/68612/how-to-compute-tangent-and-bitangent-vectors
				float x1 = v2.x - v1.x;
				float x2 = v3.x - v1.x;
				float y1 = v2.y - v1.y;
				float y2 = v3.y - v1.y;
				float z1 = v2.z - v1.z;
				float z2 = v3.z - v1.z;

				float s1 = w2.x - w1.x;
				float s2 = w3.x - w1.x;
				float t1 = w2.y - w1.y;
				float t2 = w3.y - w1.y;

				float r = 1.0F / (s1 * t2 - s2 * t1);
				glm::vec3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
					(t2 * z1 - t1 * z2) * r);
				glm::vec3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
					(s1 * z2 - s2 * z1) * r);

				tan1[i1] += sdir;
				tan1[i2] += sdir;
				tan1[i3] += sdir;

				tan2[i1] += tdir;
				tan2[i2] += tdir;
				tan2[i3] += tdir;
			}

			for (long a = 0; a < verts.size(); a++)
			{
				const glm::vec3& n = normals[a];
				const glm::vec3& t = tan1[a];

				// Gram-Schmidt orthogonalized
				tangents[a] = glm::normalize(t - n * glm::dot(n, t));

				//????????
				//// Calculate handedness
				//tangents[a].w = (Dot(Cross(n, t), tan2[a]) < 0.0F) ? -1.0F : 1.0F;
			}

			for (size_t v = 0; v < verts.size(); v++)
			{
				//i dont know why - is needed but tbn matrix was messed up without this
				//i removed the - sign in form this statement it was inverting result
				bitangents[v] = glm::cross(normals[v], tangents[v]);
			}

		}
	}


	void Mesh::debugDrawNormals(gfx::Renderer* renderer, const Transform& modelTransform)
	{
		/*
			for every triangle, find it's midpoint, convert to world coords and debug draw a vector in direction of normal
		*/

		//triangle normals
		for (int subMesh = 0; subMesh < indicies.size(); subMesh++)
		{
			for (int tri = 0; tri < indicies[subMesh].size(); tri += 3)
			{
				auto v1 = verts[indicies[subMesh][tri]];
				auto v2 = verts[indicies[subMesh][tri + 1]];
				auto v3 = verts[indicies[subMesh][tri + 2]];

				auto n1 = normals[indicies[subMesh][tri]];
				auto n2 = normals[indicies[subMesh][tri + 1]];
				auto n3 = normals[indicies[subMesh][tri + 2]];
				auto avgN = glm::normalize(n1 + n2 + n3);

				auto midPoint = (v1 + v2 + v3) / 3.f;

				glm::vec3 transformedNormal = modelTransform.matrix() * glm::vec4(avgN, 0);
				glm::vec3 transformedPosition = modelTransform.matrix() * glm::vec4(midPoint, 1);

				renderer->debugDraw->drawVector(transformedNormal, transformedPosition);
			}
		}

		//vertextNormals

		for (int i = 0; i < verts.size() ; i++)
		{
			//tangent should be +u axis and bitangent should be +v axis
			glm::vec3 transformedNormal = modelTransform.matrix() * glm::vec4(bitangents[i], 0);
			glm::vec3 transformedPosition = modelTransform.matrix() * glm::vec4(verts[i], 1);

			//renderer->debugDraw->drawVector(transformedNormal, transformedPosition);
		}

	}


	std::array<VkVertexInputBindingDescription, 5> Mesh::getBindingDescription() {
		std::array<VkVertexInputBindingDescription, 5> bindingDescriptions = {
			makeVertBinding(0, sizeof(glm::vec3)),
			makeVertBinding(1, sizeof(glm::vec2)),
			makeVertBinding(2, sizeof(glm::vec3)),
			makeVertBinding(3, sizeof(glm::vec3)),
			makeVertBinding(4, sizeof(glm::vec3)),
		};


		return bindingDescriptions;
	}

	std::array<VkVertexInputAttributeDescription, 5> Mesh::getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions = {
			makeVertAttribute(0,0,VertexAttributeFormat::vec3,0),
			makeVertAttribute(1,1,VertexAttributeFormat::vec2,0),
			makeVertAttribute(2,2,VertexAttributeFormat::vec3,0),
			makeVertAttribute(3,3,VertexAttributeFormat::vec3,0),
			makeVertAttribute(4,4,VertexAttributeFormat::vec3,0),
		};

		return attributeDescriptions;
	}


	void makeLibiglMesh(const Mesh& mesh, size_t subMesh, Eigen::MatrixXd& verts, Eigen::MatrixXi& indicies)
	{
		verts.resize(mesh.verts.size(), 3);
		for (size_t i = 0; i < mesh.verts.size(); i++)
		{
			verts(i, 0) = mesh.verts[i].x;
			verts(i, 1) = mesh.verts[i].y;
			verts(i, 2) = mesh.verts[i].z;
		}
		indicies.resize(mesh.indicies[subMesh].size() / 3, 3);
		//indicies(0, 0) = 0;
		//indicies(0, 1) = 1;
		//indicies(0, 2) = 2;

		//size_t i = 0;

		for (size_t t = 0; t < mesh.indicies[subMesh].size() / 3; t++)
		{
			indicies(t, 0) = mesh.indicies[subMesh][t * 3 + 0];
			indicies(t, 1) = mesh.indicies[subMesh][t * 3 + 1];
			indicies(t, 2) = mesh.indicies[subMesh][t * 3 + 2];
		}
	}

	void makeMeshFromLibigl(Mesh& mesh, size_t subMesh, const Eigen::MatrixXd& verts, const Eigen::MatrixXi& indicies)
	{
		auto baseVert = mesh.verts.size();
		for (size_t i = 0; i < verts.rows(); i++) {
			mesh.verts.emplace_back(verts(i, 0), verts(i, 1), verts(i, 2));
		}

		if (mesh.indicies.size() == subMesh)
			mesh.indicies.emplace_back();

		for (size_t t = 0; t < indicies.rows(); t++)
		{
			mesh.indicies[subMesh][t * 3 + 0] = indicies(t, 0) + baseVert;
			mesh.indicies[subMesh][t * 3 + 1] = indicies(t, 1) + baseVert;
			mesh.indicies[subMesh][t * 3 + 2] = indicies(t, 2) + baseVert;
		}
	}
	std::array<VkVertexInputBindingDescription, 1> Basic2DMesh::getBindingDescription()
	{
		return { makeVertBinding(0, sizeof(glm::vec2)) };
	}
	std::array<VkVertexInputAttributeDescription, 1> Basic2DMesh::getAttributeDescriptions()
	{
		return { makeVertAttribute(0, 0, VertexAttributeFormat::vec2, 0) };
	}
	size_t Basic2DMesh::vertsSize()
	{
		return verts.size() * sizeof(glm::vec2);
	}
	size_t Basic2DMesh::indiciesSize()
	{
		return indicies.size() * sizeof(glm::uint32);
	}
	size_t Basic2DMesh::vertsOffset()
	{
		return 0;
	}
	size_t Basic2DMesh::indiciesOffset()
	{
		return vertsSize();
	}
	size_t Basic2DMesh::fullSize()
	{
		return indiciesOffset() + indiciesSize();
	}
}
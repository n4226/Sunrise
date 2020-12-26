#include "srpch.h"
#include "Mesh.h"


namespace sunrise {

	size_t Mesh::vertsSize()
	{
		return verts.size() * sizeof(glm::vec3);
	}

	size_t Mesh::uvsSize()
	{
		return uvs.size() * sizeof(glm::vec2);
	}

	size_t Mesh::normalsSize()
	{
		return normals.size() * sizeof(glm::vec3);
	}

	size_t Mesh::tangentsSize()
	{
		return tangents.size() * sizeof(glm::vec3);
	}

	size_t Mesh::bitangentsSize()
	{
		return bitangents.size() * sizeof(glm::vec3);
	}

	size_t Mesh::indiciesSize()
	{
		return indicies.size() * sizeof(glm::uint32);
	}

	size_t Mesh::vertsOffset()
	{
		return 0;
	}

	size_t Mesh::uvsOffset()
	{
		return vertsOffset() + vertsSize();
	}

	size_t Mesh::normalsOffset()
	{
		return uvsOffset() + uvsSize();
	}

	size_t Mesh::tangentsOffset()
	{
		return normalsOffset() + normalsSize();
	}

	size_t Mesh::bitangentsOffset()
	{
		return tangentsOffset() + tangentsSize();
	}

	size_t Mesh::indiciesOffset()
	{
		return bitangentsOffset() + bitangentsSize();
	}

	size_t Mesh::fullSize()
	{
		return indiciesOffset() + indiciesSize();
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
}
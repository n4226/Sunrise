#pragma once

#include "igl/AABB.h"

namespace sunrise {

	class BinaryMeshAttrributes;

	/// <summary>
	/// an abstract representation of a mesh
	/// </summary>
	struct SUNRISE_API Mesh
	{
		std::vector<glm::vec3> verts;
		std::vector<glm::vec2> uvs;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec3> tangents;
		std::vector<glm::vec3> bitangents;

		std::vector<std::vector<glm::uint32>> indicies;



		static std::array<VkVertexInputBindingDescription, 5> getBindingDescription();

		static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions();

		// offset and size functions

		size_t         vertsSize();
		size_t           uvsSize();
		size_t       normalsSize();
		size_t      tangentsSize();
		size_t    bitangentsSize();
		//size_t      indiciesSize();
		size_t indiciesSize(size_t subMesh);
		size_t AllSubMeshIndiciesSize();

		size_t       vertsOffset();
		size_t         uvsOffset();
		size_t     normalsOffset();
		size_t    tangentsOffset();
		size_t  bitangentsOffset();
		size_t    indiciesOffset();

		size_t    fullSize();

		/// <summary>
		/// optinal link to atributes
		/// </summary>
		BinaryMeshAttrributes* attributes = nullptr;
	};


	void SUNRISE_API makeLibiglMesh(const Mesh& mesh, size_t subMesh, Eigen::MatrixXd& verts, Eigen::MatrixXi& indicies);
	void SUNRISE_API makeMeshFromLibigl(Mesh& mesh, size_t subMesh, const Eigen::MatrixXd& verts, const Eigen::MatrixXi& indicies);

}
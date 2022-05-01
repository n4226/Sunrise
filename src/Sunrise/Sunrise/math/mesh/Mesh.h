#pragma once

#include "igl/AABB.h"
#include "../../scene/Transform.h"

namespace sunrise {

	class BinaryMeshAttrributes;

	namespace gfx {
		class Renderer;
	}

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

		size_t         vertsSize() const;
		size_t           uvsSize() const;
		size_t       normalsSize() const;
		size_t      tangentsSize() const;
		size_t    bitangentsSize() const;
		//size_t      indiciesSize();
		size_t indiciesSize(size_t subMesh) const;
		size_t AllSubMeshIndiciesSize() const;

		size_t       vertsOffset() const;
		size_t         uvsOffset() const;
		size_t     normalsOffset() const;
		size_t    tangentsOffset() const;
		size_t  bitangentsOffset() const;
		size_t    indiciesOffset() const;
		/// <summary>
		/// not very fast
		/// </summary>
		/// <param name="submesh"></param>
		/// <returns></returns>
		size_t    indiciesOffset(int submesh) const;

		size_t    fullSize() const;

		/// <summary>
		/// optinal link to atributes
		/// </summary>
		BinaryMeshAttrributes* attributes = nullptr;

		void calculateTangentsAndBitangents();

		//debug
		void debugDrawNormals(gfx::Renderer* renderer,const Transform& modelTransform);
	};


	void SUNRISE_API makeLibiglMesh(const Mesh& mesh, size_t subMesh, Eigen::MatrixXd& verts, Eigen::MatrixXi& indicies);
	void SUNRISE_API makeMeshFromLibigl(Mesh& mesh, size_t subMesh, const Eigen::MatrixXd& verts, const Eigen::MatrixXi& indicies);


	/// <summary>
	/// an abstract representation of a simple 2d mesh - helpful for full screan quad generation for pos proccesing
	/// </summary>
	struct SUNRISE_API Basic2DMesh
	{
		std::vector<glm::vec2> verts;
		std::vector<glm::uint32> indicies;

		static std::array<VkVertexInputBindingDescription, 1> getBindingDescription();

		static std::array<VkVertexInputAttributeDescription, 1> getAttributeDescriptions();

		// offset and size functions

		size_t         vertsSize();
		//size_t           uvsSize();
		//size_t       normalsSize();
		//size_t      tangentsSize();
		//size_t    bitangentsSize();
		size_t      indiciesSize();
		//size_t indiciesSize(size_t subMesh);
		//size_t AllSubMeshIndiciesSize();

		size_t       vertsOffset();
		//size_t         uvsOffset();
		//size_t     normalsOffset();
		//size_t    tangentsOffset();
		//size_t  bitangentsOffset();
		size_t    indiciesOffset();

		size_t    fullSize();

	};

}
#include "srpch.h"
#include "BinaryMesh.h"
#include <glm/glm.hpp>

namespace sunrise {

	BinaryMeshSeirilizer::BinaryMeshSeirilizer(Mesh& originalMesh)
	{
		assert(originalMesh.verts.size() == originalMesh.uvs.size() && originalMesh.normals.size() == originalMesh.tangents.size() && originalMesh.bitangents.size());

		auto headerLength = sizeof(uint32_t) * (2 + originalMesh.indicies.size());
		// vert size * 56
		auto verticiesLength = originalMesh.indiciesOffset();
		auto indiciesLength = 0;

		for (std::vector<uint32_t>& subMesh : originalMesh.indicies) {
			indiciesLength += (subMesh.size() * sizeof(uint32_t));
		}

		size_t meshLength = headerLength + verticiesLength + indiciesLength;

		this->headerLength = headerLength;

		this->meshLength = meshLength;
		mesh = malloc(meshLength);

		//populate the data

		//header
		vertCount = (reinterpret_cast<uint32_t*>(mesh) + 0);
		*vertCount = originalMesh.verts.size();

		subMeshCount = (reinterpret_cast<uint32_t*>(mesh) + 1);
		*subMeshCount = originalMesh.indicies.size();

		subMeshIndexCounts = (reinterpret_cast<uint32_t*>(mesh) + 2);


		size_t i = 0;
		for (std::vector<uint32_t>& subMesh : originalMesh.indicies) {
			*(subMeshIndexCounts + i) = subMesh.size();
			i++;
		}


		//verticies
		// this is the byte immediately after the proceidng array of all submesh lengths  
		glm::vec3* vertPosStart = reinterpret_cast<glm::vec3*>(reinterpret_cast<uint32_t*>(mesh) + 2 + i);
		memcpy(vertPosStart, originalMesh.verts.data(), originalMesh.vertsSize());

		glm::vec2* uvStart = reinterpret_cast<glm::vec2*>(vertPosStart + originalMesh.verts.size());
		memcpy(uvStart, originalMesh.uvs.data(), originalMesh.uvsSize());

		glm::vec3* normalStart = reinterpret_cast<glm::vec3*>(uvStart + originalMesh.uvs.size());
		memcpy(normalStart, originalMesh.normals.data(), originalMesh.normalsSize());

		glm::vec3* tangentStart = normalStart + originalMesh.normals.size();
		memcpy(tangentStart, originalMesh.tangents.data(), originalMesh.tangentsSize());

		glm::vec3* bitangentStart = tangentStart + originalMesh.tangents.size();
		memcpy(bitangentStart, originalMesh.bitangents.data(), originalMesh.bitangentsSize());
		//start of mesh adr =  0x00000271181C3070 -- 2684759060592 decimal, end of mehs adress - 0x000002711847A278// indixies Start mem adr = 2684761404600 decimal
		//indices
		{
			uint32_t* indiciesStart = reinterpret_cast<uint32_t*>(bitangentStart + originalMesh.bitangents.size());
			// in bytes
			size_t offset = 0;
			for (std::vector<uint32_t>& subMesh : originalMesh.indicies)
			{
				size_t size = sizeof(uint32_t) * subMesh.size();
				memcpy(indiciesStart + (offset / sizeof(uint32_t)), subMesh.data(), size);
				offset += size;
			}
			assert((reinterpret_cast<char*>(indiciesStart) - reinterpret_cast<char*>(mesh)) + offset == meshLength);
		}
	}

	BinaryMeshSeirilizer::BinaryMeshSeirilizer(void* encodedMesh, size_t encodedMeshLength)
		: mesh(encodedMesh), meshLength(encodedMeshLength),
		vertCount((reinterpret_cast<uint32_t*>(encodedMesh))),
		subMeshCount((reinterpret_cast<uint32_t*>(encodedMesh) + 1)),
		subMeshIndexCounts((reinterpret_cast<uint32_t*>(encodedMesh) + 2)),
		headerLength(sizeof(uint32_t)* (2 + (*subMeshCount)))
	{

	}

	BinaryMeshSeirilizer::BinaryMeshSeirilizer(const char* filePath)
	{
		std::ifstream file(filePath, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			assert(false);
			//throw std::runtime_error("failed to open file!");
		}

		meshLength = (size_t)file.tellg();

		mesh = malloc(meshLength);

		file.seekg(0);
		file.read(reinterpret_cast<char*>(mesh), meshLength);

		file.close();

		vertCount = (reinterpret_cast<uint32_t*>(mesh));
		subMeshCount = (reinterpret_cast<uint32_t*>(mesh) + 1);
		subMeshIndexCounts = (reinterpret_cast<uint32_t*>(mesh) + 2);
		headerLength = sizeof(uint32_t) * (2 + (*subMeshCount));
	}

	BinaryMeshSeirilizer::~BinaryMeshSeirilizer()
	{
		free(mesh);
	}



	size_t BinaryMeshSeirilizer::vertsSize()
	{
		return (*vertCount) * sizeof(glm::vec3);
	}

	size_t BinaryMeshSeirilizer::uvsSize()
	{
		return (*vertCount) * sizeof(glm::vec2);
	}

	size_t BinaryMeshSeirilizer::normalsSize()
	{
		return (*vertCount) * sizeof(glm::vec3);
	}

	size_t BinaryMeshSeirilizer::tangentsSize()
	{
		return (*vertCount) * sizeof(glm::vec3);
	}

	size_t BinaryMeshSeirilizer::bitangentsSize()
	{
		return (*vertCount) * sizeof(glm::vec3);
	}

	size_t BinaryMeshSeirilizer::indiciesSize(size_t subMesh)
	{
		return (subMeshIndexCounts[subMesh]) * sizeof(glm::uint32);
	}

	size_t BinaryMeshSeirilizer::AllSubMeshIndiciesSize() {
		size_t total = 0;

		for (size_t i = 0; i < *subMeshCount; i++)
		{
			total += indiciesSize(i);
		}
		return total;
	}







	void* BinaryMeshSeirilizer::vertsPtr()
	{
		return reinterpret_cast<void*>(reinterpret_cast<char*>(mesh) + headerLength);
	}

	void* BinaryMeshSeirilizer::uvsPtr()
	{
		return reinterpret_cast<void*>(reinterpret_cast<char*>(vertsPtr()) + vertsSize());
	}

	void* BinaryMeshSeirilizer::normalsPtr()
	{
		return reinterpret_cast<void*>(reinterpret_cast<char*>(uvsPtr()) + uvsSize());
	}

	void* BinaryMeshSeirilizer::tangentsPtr()
	{
		return reinterpret_cast<void*>(reinterpret_cast<char*>(normalsPtr()) + normalsSize());
	}

	void* BinaryMeshSeirilizer::bitangentsPtr()
	{
		return reinterpret_cast<void*>(reinterpret_cast<char*>(tangentsPtr()) + tangentsSize());
	}

	void* BinaryMeshSeirilizer::indiciesPtr(size_t subMesh)
	{
		//TODO allow multiple submeshes 
		assert(subMesh == 0);
		return reinterpret_cast<void*>(reinterpret_cast<char*>(bitangentsPtr()) + bitangentsSize());
	}

}
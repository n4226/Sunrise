#include "srpch.h"
#include "BinaryMeshAttrributes.h"

namespace sunrise {

	BinaryMeshAttrributes::BinaryMeshAttrributes()
	{

	}

	BinaryMeshAttrributes::BinaryMeshAttrributes(std::string& filePath)
	{
		/*  Binary Format Def

			glm::vec3 aabbSize
			uint32_t subMeshMatsCount
			uint32_t* subMeshMats - array with length of subMeshMatsCount

		*/


		std::ifstream file(filePath.c_str(), std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			assert(false);
			//throw std::runtime_error("failed to open file!");
		}

		auto attrSize = (size_t)file.tellg();

		auto meshAttr = malloc(attrSize);

		file.seekg(0);
		file.read(reinterpret_cast<char*>(meshAttr), attrSize);

		file.close();

		aabbRadius = *reinterpret_cast<glm::vec3*>(meshAttr);

		auto pSubMeshMatsCount = reinterpret_cast<uint32_t*>(reinterpret_cast<glm::vec3*>(meshAttr) + 1);
		auto pSubMeshMatsStart = pSubMeshMatsCount + 1;
		auto subMeshMatsCount = *pSubMeshMatsCount;

		subMeshMats = { pSubMeshMatsStart, pSubMeshMatsStart + subMeshMatsCount };

		free(meshAttr);
	}

	void BinaryMeshAttrributes::saveTo(std::string& filePath)
	{

		std::ofstream out;
		out.open(filePath.c_str(), std::fstream::out | std::fstream::binary);


		size_t attrSize = sizeof(glm::vec3) + (sizeof(uint32_t) * (subMeshMats.size() + 1));

		void* meshAttr = malloc(attrSize);

		*reinterpret_cast<glm::vec3*>(meshAttr) = aabbRadius;
		*reinterpret_cast<uint32_t*>(reinterpret_cast<glm::vec3*>(meshAttr) + 1) = subMeshMats.size();
		if (subMeshMats.size() > 0)
			memcpy(reinterpret_cast<uint32_t*>(reinterpret_cast<glm::vec3*>(meshAttr) + 1) + 1, subMeshMats.data(), subMeshMats.size() * sizeof(uint32_t));


		out.write(reinterpret_cast<char*>(meshAttr), attrSize);
		out.close();

		free(meshAttr);
	}

}
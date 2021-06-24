#pragma once

#include "srpch.h"

namespace sunrise {

	class SUNRISE_API BinaryMeshAttrributes
	{

	private:

		struct SnapShort
		{
			glm::vec3 aabbRadius;
			uint32_t* subMeshMats;
		};

	public:
		BinaryMeshAttrributes();
		/// <summary>
		/// load from file
		/// </summary>
		BinaryMeshAttrributes(std::string& filePath);

		/// <summary>
		/// save to file
		/// </summary>
		/// <param name="filePath"></param>
		void saveTo(std::string& filePath);

		glm::vec3 aabbRadius;
		std::vector<uint32_t> subMeshMats;

	};

}
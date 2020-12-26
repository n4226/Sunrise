#pragma once

#include "srpch.h"

namespace sunrise {

	class SUNRISE_API StaticMaterialTable
	{

	public:
		static std::unordered_map<std::string, glm::uint32> entries;
		static std::map<glm::uint32, std::string> reverseEntries;
	};

}


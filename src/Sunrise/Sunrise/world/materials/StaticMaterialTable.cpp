#include "srpch.h"
#include "StaticMaterialTable.h"


namespace sunrise {

	std::unordered_map<std::string, glm::uint32> StaticMaterialTable::entries = {

			{"ocean1", 0},
			{"grass1", 1},
			{"building1", 2},

	};

	std::map<glm::uint32, std::string> StaticMaterialTable::reverseEntries = {

			{0, "ocean1"},
			{1, "grass1"},
			{2, "building1"},
	};

}
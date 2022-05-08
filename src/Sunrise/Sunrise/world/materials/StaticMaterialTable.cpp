#include "srpch.h"
#include "StaticMaterialTable.h"


namespace sunrise {

	std::unordered_map<std::string, glm::uint32> StaticMaterialTable::entries = {
		// doenst seem linke anytihg is useing this list currenlty
			{"ocean1", 0},
			{"grass1", 1},
			{"building1", 2},
			{"asphalt1", 3},
			//{"concrete1", 4},
			{"grass3", 6},
			{"TestSunriseMaterial",9},
			{"SunriseTestWoodMat", 10},
	};

	std::map<glm::uint32, std::string> StaticMaterialTable::reverseEntries = {

			{0, "ocean1"},
			{1, "grass1"},
            {2, "building1"},
			{3, "asphalt1"},
			//{4, "concrete1"},
			//{5, "grass2"},
			{6, "grass3"},
			{9, "TestSunriseMaterial"},
			{10, "SunriseTestWoodMat"},
	};

}

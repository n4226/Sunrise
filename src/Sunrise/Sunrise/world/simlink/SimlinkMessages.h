#pragma once

namespace sunrise::SimlinkMessages {


	struct SUNRISE_API simpleUpdate
	{
		uint32_t id;
		glm::quat rot;
		glm::dvec3 lla;

		static uint32_t msgID;
	}; // size = 44 bytes i think

}
#include "srpch.h"
#include "CameraSystem.h"
#include "../../core/Application.h"
#include "../../world/WorldScene.h"

#include "Sunrise/Sunrise/world/simlink/SimlinkMessages.h"
#include "Sunrise/Sunrise/networking/networking.h"


namespace sunrise {

	using namespace math;

	CameraSystem::CameraSystem()
	{
		
	}



	void CameraSystem::update()
	{
		PROFILE_FUNCTION;
		

		world->playerTrans.position = LlatoGeo(world->playerLLA, world->origin, world->terrainSystem->getRadius());

		// rotation ------



		// sin form [0,1] - * (sin(world->timef) * 0.5f + 1)

		auto N = glm::normalize(world->playerTrans.position + glm::vec3(world->origin));

		auto QuatAroundX = glm::angleAxis(glm::radians(static_cast<float>(world->playerLLA.x)), glm::vec3(1.0, 0.0, 0.0));
		// when at 0 lat and 0 lon to look directly at sphere the quataroundy must have an angle of .pi/2 rad
		auto QuatAroundY = glm::angleAxis(-glm::pi<float>() / 2 - glm::radians(static_cast<float>(world->playerLLA.y)), glm::vec3(0.0, 1.0, 0.0));
		auto QuatAroundZ = glm::angleAxis(0.f, glm::vec3(0.0, 0.0, 1.0));
		auto finalOrientation = QuatAroundZ * QuatAroundY * QuatAroundX;

		world->playerTrans.rotation = finalOrientation * world->playerLLARotation;

		for (size_t i = 0; i < world->app.windows.size(); i++)
		{
			auto& window = world->app.windows[i];
			window->camera.transform.rotation = finalOrientation * window->camera.transform.rotation;
		}
	}

	void CameraSystem::setup()
	{
	}


}
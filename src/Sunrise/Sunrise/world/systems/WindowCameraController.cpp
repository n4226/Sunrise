#include "srpch.h"
#include "WindowCameraController.h"

#include "../../core/Application.h"
#include "../../world/WorldScene.h"

#include "Sunrise/Sunrise/world/simlink/SimlinkMessages.h"
#include "Sunrise/Sunrise/networking/networking.h"


namespace sunrise {



	void WindowCameraController::update()
	{
		for (size_t i = 0; i < world->app.windows.size(); i++)
		{
			auto& window = world->app.windows[i];

			auto camTrans = configSystem.global().cameras[i];

			window->camera.transform.position = world->playerTrans.position += camTrans.offset;
			window->camera.transform.rotation = world->playerLLARotation * glm::angleAxis(glm::radians(camTrans.rotAngleDeg),camTrans.rotAxis);
		}
	}

}
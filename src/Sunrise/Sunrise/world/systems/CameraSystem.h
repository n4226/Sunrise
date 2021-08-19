#pragma once

#include "srpch.h"
#include "../../Math.h"
#include "../../graphics/vulkan/renderer/RenderSystem.h"

#include "Sunrise/Sunrise/world/simlink/SimlinkMessages.h"


namespace sunrise {

	class NetworkManager;

	class CameraSystem : public System
	{
	public:
		CameraSystem();

		void update() override;
		void setup() override;
	private:


	};

}
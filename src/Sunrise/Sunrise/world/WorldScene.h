#pragma once

#include "srpch.h"
#include "../core/Window.h"
#include "../scene/Scene.h"
#include "../graphics/vulkan/renderer/Renderer.h"
#include "terrain/TerrainSystem.h"

namespace sunrise {

	class WorldScene : public Scene
	{

	public:

		WorldScene(Application& app);
		~WorldScene();

		virtual void load() override;

		virtual void update() override;

		virtual void unload() override;

		TerrainSystem* terrainSystem;

		std::vector<System*> generalSystems;

		glm::dvec3 origin = glm::dvec3(0, 0, 0);

		// player

		const glm::dvec3 initialPlayerLLA = glm::dvec3(40.610319941413, -74.039182662964, 100);

		glm::dvec3 playerLLA =
			//glm::dvec3(0, 0, 1000);
			//glm::dvec3(40.610319941413, -74.039182662964, 100);
			initialPlayerLLA;

		Transform playerTrans;

		Application& app;

	};


}
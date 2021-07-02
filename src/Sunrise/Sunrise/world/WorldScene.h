#pragma once

#include "srpch.h"
#include "../core/Window.h"
#include "../scene/Scene.h"
#include "../graphics/vulkan/renderer/Renderer.h"
#include "terrain/TerrainSystem.h"

namespace sunrise {

	/// <summary>
	/// scene which represents the earth
	/// right now just terrain
	/// </summary>
	class SUNRISE_API WorldScene : public Scene
	{

	public:
		WorldScene(Application* app);
		~WorldScene();

#pragma region Lifecycle
		virtual void load() override;

		virtual void lateLoad() override;

		virtual void update() override;

		virtual void unload() override;
#pragma endregion

		TerrainSystem* terrainSystem;

		std::vector<System*> generalSystems;

		glm::dvec3 origin = glm::dvec3(0, 0, 0);

#pragma region player
		const glm::dvec3 initialPlayerLLA = glm::dvec3(40.610319941413, -74.039182662964, 4);

		glm::dvec3 playerLLA =
			//glm::dvec3(0, 0, 1000);
			//glm::dvec3(40.610319941413, -74.039182662964, 100);
			initialPlayerLLA;

		Transform playerTrans;
#pragma endregion
	};


}
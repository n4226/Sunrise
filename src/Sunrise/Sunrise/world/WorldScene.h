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

#pragma region TerrainMasking
		//for development and singal chunk viewing - in future this should just be onde in another scene
		// if null does nothing
		// else dynamic terrain quad tree is overidden, chunks inside this array will be rendered and remain constant
		std::vector<math::Box>* terrainMask = nullptr;// math::Box(glm::dvec2(0),glm::dvec2(0));

		/// <summary>
		/// if a desired chunk in the mask is not found on disk should a default one be created
		/// </summary>
		bool doNotSparslyPopulateMask = true;

		/// <summary>
		/// if a terrain Mask is set, this reloads the terrain from disk in the static region
		/// </summary>
		void reloadTerrainInMask();
#pragma endregion
#pragma region player
		const glm::dvec3 initialPlayerLLA = glm::dvec3(40.610319941413, -74.039182662964, 4);

		glm::dvec3 playerLLA =
			//glm::dvec3(0, 0, 1000);
			//glm::dvec3(40.610319941413, -74.039182662964, 100);
			initialPlayerLLA;

		glm::qua<glm::float32> playerLLARotation{};

		Transform playerTrans;
#pragma endregion
	};


}
#include "srpch.h"
#include "WorldScene.h"
#include "systems/CameraSystem.h"
#include "systems/FloatingOriginSystem.h"
#include "systems/PlayerMovementSystem.h"
#include "systems/WindowCameraController.h"
#include "../core/Application.h"
#include "../graphics/vulkan/renderer/MaterialManager.h"
#include "rendering/WorldSceneRenderCoordinator.h"

namespace sunrise {


	WorldScene::WorldScene(Application* app) 
		: Scene(app)
	{
		PROFILE_FUNCTION;

		coordinator = new WorldSceneRenderCoordinator(this);
	}

	WorldScene::~WorldScene()
	{
		PROFILE_FUNCTION;

		delete coordinator;
	}

	void WorldScene::load()
	{
		PROFILE_FUNCTION;

		//TODO: Remove
		terrainMask = new std::vector<math::Box>{ math::Box(playerLLA, glm::dvec2(0.01)) };

		//renderer = new Renderer(window.device, window.physicalDevice, window);
		//renderer->world = this;
		terrainSystem = new TerrainSystem(app, *this, &origin);
		terrainSystem->trackedTransform = &playerTrans;
		terrainSystem->world = this;

		dynamic_cast<WorldSceneRenderCoordinator*>(coordinator)->createUniforms();

		/// <summary>
		/// it is important that the camera systemis befoer floating origin so that floating origin snaps before first frame
		/// </summary>
		generalSystems = { new PlayerMovementSystem(), new WindowCameraController(), new CameraSystem(), new FloatingOriginSystem() };

		for (System* sys : generalSystems) {
			sys->world = this;
			sys->setup();
		}

		time = 0;
		timef = 0;

		generalSystems[0]->update();
	}

	void WorldScene::lateLoad()
	{
		for (auto ren : app.renderers)
			ren->materialManager->loadStaticEarth();
	}


	void WorldScene::update()
	{
		PROFILE_FUNCTION;
		Scene::update();

		if (frameNum == 2000) {
			reloadTerrainInMask();
		}

		//if (frameNum == 2000) {
		//	char** stats = new char*;
		//	vmaBuildStatsString(app.renderers[0]->allocator, stats, VK_TRUE);
		//	//std::string statString(*stats);

		//	{
		//		std::ofstream out;
		//		out.open("vmaDump.profile", std::fstream::out);
		//		//out.open(file, std::fstream::out);
		//		out << *stats;
		//		out.close();
		//	}
		//	vmaFreeStatsString(app.renderers[0]->allocator, *stats);
		//}

		for (System* sys : generalSystems) {
			sys->update();
		}

		terrainSystem->update();

		if (app.getKey(GLFW_KEY_ESCAPE)) {
			app.quit();
		}

		/*if (app.mouseLeft) {
			SR_CORE_("Mouse at ({},{})", app.mousePos.x, app.mousePos.y);
		} else 
		SR_CORE_TRACE("Mouse at ({},{})", app.mousePos.x,app.mousePos.y);*/

		frameNum += 1;
	}

	void WorldScene::unload()
	{

		delete terrainSystem;

		for (System* sys : generalSystems) {
			sys->cleanup();
			delete sys;
		}

	} 

	void WorldScene::reloadTerrainInMask()
	{
		//terrainMask = new std::vector<math::Bo/x>{ math::Box(glm::dvec2(playerLLA) + glm::dvec2(0.1,0.1), glm::dvec2(0.01))};
		if (terrainMask != nullptr)
			terrainSystem->reloadTerrainInMask();
	}


}
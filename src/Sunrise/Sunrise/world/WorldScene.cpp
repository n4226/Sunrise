#include "srpch.h"
#include "WorldScene.h"
#include "systems/CameraSystem.h"
#include "systems/FloatingOriginSystem.h"
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

		// todo redo mats
		/*for (auto ren : app.renderers)
			ren->materialManager->loadStaticEarth();*/


		//renderer = new Renderer(window.device, window.physicalDevice, window);
		//renderer->world = this;
		terrainSystem = new TerrainSystem(app, *this, &origin);
		terrainSystem->trackedTransform = &playerTrans;
		terrainSystem->world = this;

		dynamic_cast<WorldSceneRenderCoordinator*>(coordinator)->createUniforms();

		/// <summary>
		/// importantthat the camera systemis befoer floating origin so that floating origin snaps before first frame
		/// </summary>
		generalSystems = { new CameraSystem(), new FloatingOriginSystem() };

		for (System* sys : generalSystems) {
			sys->world = this;
		}

		time = 0;
		timef = 0;

		generalSystems[0]->update();
	}


	void WorldScene::update()
	{
		PROFILE_FUNCTION;
		Scene::update();

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

		//TODO: move this to camera sys or another one?
		for (size_t i = 0; i < app.windows.size(); i++)
		{
			auto& window = app.windows[i];

			auto camTrans = configSystem.global().cameras[i];

			window->camera.transform = playerTrans;
			//window->camera.transform.rotation = glm::identity<glm::quat>();
			window->camera.transform.position
				//= glm::vec3(0);
				+= camTrans.offset;
		}

		frameNum += 1;
	}

	void WorldScene::unload()
	{

		delete terrainSystem;

		for (System* sys : generalSystems) {
			delete sys;
		}

	}


}
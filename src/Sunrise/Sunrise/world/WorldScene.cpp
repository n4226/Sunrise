#include "srpch.h"
#include "WorldScene.h"
#include "CameraSystem.h"
#include "FloatingOriginSystem.h"
#include "../core/Application.h"

namespace sunrise {


	WorldScene::WorldScene(Application& app) :
		app(app)
	{
		PROFILE_FUNCTION;

	}

	WorldScene::~WorldScene()
	{
		PROFILE_FUNCTION;


	}

	void WorldScene::load()
	{
		PROFILE_FUNCTION;
		//renderer = new Renderer(window.device, window.physicalDevice, window);
		//renderer->world = this;
		terrainSystem = new TerrainSystem(app, *this, &origin);
		terrainSystem->trackedTransform = &playerTrans;
		terrainSystem->world = this;

		for (auto render : app.renderers)
			render->terrainSystem = terrainSystem;

		generalSystems = { new FloatingOriginSystem(), new CameraSystem() };

		for (System* sys : generalSystems) {
			sys->world = this;
		}

		time = 0;
		timef = 0;

		generalSystems[0]->update();
	}


	void WorldScene::update()
	{
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
#include "srpch.h"
#include "WorldScene.h"
#include "systems/CameraSystem.h"
#include "systems/FloatingOriginSystem.h"
#include "systems/PlayerMovementSystem.h"
#include "systems/WindowCameraController.h"
#include "../core/Application.h"
#include "../graphics/vulkan/renderer/MaterialManager.h"
#include "rendering/WorldSceneRenderCoordinator.h"
#include "../fileSystem/FileManager.h"

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

	}

	void WorldScene::load()
	{
		PROFILE_FUNCTION;

		//TODO: Remove
		//terrainMask = new std::vector<math::Box>{ math::Box(playerLLA, glm::dvec2(0.01)) };

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

	void WorldScene::onDrawUI()
	{
		ImGui::Begin("World Scene Settings",nullptr,ImGuiWindowFlags_NoCollapse);

		ImGui::Text("World Scene Settings");

        glm::vec3 playerLLA = this->playerLLA;
		std::array<float,3> llaPos = {playerLLA.x, playerLLA.y, playerLLA.z};

		if (ImGui::InputFloat3("LLA Position", llaPos.data(), "%.5f")) {
			this->playerLLA.x = llaPos[0];
			this->playerLLA.y = llaPos[1];
			this->playerLLA.z = llaPos[2];
		}

		if (ImGui::Button("Reset POS")) {
			this->playerLLA = initialPlayerLLA;
		}

		//sunPos
        glm::vec3 sunLL = this->sunLL;
		std::array<float, 2> sun = { sunLL.x, sunLL.y };
		if (ImGui::SliderFloat2("Sun Pos", sun.data(), -180.f, 180.f, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
			this->sunLL.x = sun[0];
			this->sunLL.y = sun[1];
		}
		if (ImGui::Button("Reset Sun")) {
			this->sunLL = initialPlayerLLA;
		}

		ImGui::End();
	}


	void WorldScene::update()
	{
		PROFILE_FUNCTION;
		Scene::update();

//		if (frameNum == 200) {
//            char** stats = new char*;
//            vmaBuildStatsString(app.renderers[0]->allocator, stats, VK_FALSE);
//            std::string statString(*stats);
//
//            FileManager::saveStringToFile(statString, "vmaAlloc.json");
//		}

        
        
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

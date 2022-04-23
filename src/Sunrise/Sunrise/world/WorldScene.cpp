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
#include <marl/defer.h>


namespace sunrise {


	WorldScene::WorldScene(Application* app) 
		: Scene(app)
	{
		PROFILE_FUNCTION;

		coordinatorCreator = ([this](auto renderer) {
			return new WorldSceneRenderCoordinator(this, renderer);
		});
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
		terrainSystem = new TerrainSystem(app, this, &origin);
		terrainSystem->trackedTransform = &playerTrans;

		//now this is part of all coordinators and is called internally
		//dynamic_cast<WorldSceneRenderCoordinator*>(coordinator)->createUniforms();

		/// <summary>
		/// it is important that the camera systems before floating origin so that floating origin snaps before first frame
		/// </summary>
		systems = { new PlayerMovementSystem(), new WindowCameraController(), new CameraSystem(), new FloatingOriginSystem() };

	}

	void WorldScene::lateLoad()
	{
		PROFILE_FUNCTION
		Scene::lateLoad();

		systems[0]->update();


		//TODO: find better way to do this
		marl::WaitGroup preLoadingWaitGroup(app.renderers.size());
		SR_CORE_INFO("Beginning async material loading");
		for (auto ren : app.renderers) {
			marl::schedule([this, ren, preLoadingWaitGroup]() {
				defer(preLoadingWaitGroup.done());
				ren->materialManager->loadStaticEarth();
			});
		}
		preLoadingWaitGroup.wait();
		SR_CORE_INFO("ended async material loading");
	} 

	void WorldScene::onDrawUI()
	{
		ImGui::Begin("World Scene Settings", nullptr, ImGuiWindowFlags_NoCollapse);

		ImGui::Text("World Scene Settings");

		glm::vec3 playerLLA = this->playerLLA;
		std::array<float, 3> llaPos = { playerLLA.x, playerLLA.y, playerLLA.z };

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

		ImGui::Begin("World Debug");

		ImGui::Separator();

		ImGui::Text("GPU Memory");


		for (auto renderer : app.renderers) {

			ImGui::Text("GPU 1");

			renderer->gloablIndAllocator->imguiDrawDebug("Index Alocator");

			ImGui::Spacing();

			renderer->gloablVertAllocator->imguiDrawDebug("Vertex Alocator");

		}

		ImGui::Separator();

		//if (ImGui::BeginChild("Materials")) {

		//	//asymes 2d image
		//	for (auto image : app.renderers[0]->materialManager->allImages()) {
		//		ImGui::Text("Image");
		//		ImGui::Image(image->vkItem, { (float)image->size.width, (float)image->size.depth });
		//	}

		//	ImGui::EndChild();
		//}

		ImGui::End();

	}


	void WorldScene::onDrawMainMenu()
	{

		if (ImGui::BeginMenu("World Debug")) {

		//TODO: add debug tool to show loaded materials and their indecies / maybe render their textures
			ImGui::EndMenu();
		}
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

		Scene::unload();
	} 

	void WorldScene::reloadTerrainInMask()
	{
		//terrainMask = new std::vector<math::Bo/x>{ math::Box(glm::dvec2(playerLLA) + glm::dvec2(0.1,0.1), glm::dvec2(0.01))};
		if (terrainMask != nullptr)
			terrainSystem->reloadTerrainInMask();
	}


}

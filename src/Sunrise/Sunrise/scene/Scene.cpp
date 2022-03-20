#include "srpch.h"
#include "Scene.h"

#include "Sunrise/Sunrise/graphics/vulkan/renderer/SceneRenderCoordinator.h"
#include "Sunrise/core/Application.h"
#include "../core/Window.h"

namespace sunrise {

	Scene::Scene(Application* app, bool inControlOfCoordinatorLifecycle)
		: app(*app), inControlOfCoordinatorLifecycle(inControlOfCoordinatorLifecycle)
	{
		coordinator = new DefaultSceneRenderCoordinator(this);
	}

	Scene::Scene(Application* app, gfx::SceneRenderCoordinator* coordinator, bool inControlOfCoordinatorLifecycle)
		: app(*app), coordinator(coordinator), inControlOfCoordinatorLifecycle(inControlOfCoordinatorLifecycle)
	{

	}

	Scene::~Scene()
	{
		if (inControlOfCoordinatorLifecycle)
			delete coordinator;
	}

	void Scene::load()
	{
		//TODO: make all windows?
		//also cameras should be eventually seperated from bieng inside windows and owned by scene allowing multiple cameras per monityr - working with render coordinator to have multi camera per view or have camera render to texture
		for (size_t i = 0; i < app.windows.size(); i++)
		{
			auto window = app.windows[i];

			window->camera.transform.reset();
		}
	}

	void Scene::lateLoad()
	{
		for (System* sys : systems) {
			sys->scene = this;
			sys->setup();
		}

		time = 0;
		timef = 0;

	}

	void Scene::update()
	{
		static auto startTime = std::chrono::high_resolution_clock::now();


		auto currentTime = std::chrono::high_resolution_clock::now();
		
		PROFILE_FUNCTION;
		
		auto newtime = std::chrono::duration<double, std::chrono::seconds::period>(currentTime - startTime).count();
		auto newtimef = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		deltaTime = newtime - time;
		deltaTimef = newtimef - timef;

		time = newtime;
		timef = newtimef;

		for (System* sys : systems) {
			sys->update();
		}

		for (System* sys : systems) {
			sys->lateUpdate();
		}
	}

	void Scene::unload()
	{
		for (System* sys : systems) {
			sys->cleanup();
			delete sys;
		}
		systems.clear();
		registry.clear();
	}

}

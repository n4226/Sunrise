#include "srpch.h"
#include "Scene.h"

#include "Sunrise/Sunrise/graphics/vulkan/renderer/SceneRenderCoordinator.h"

namespace sunrise {

	Scene::Scene(Application* app, bool inControlOfCoordinatorLifecycle)
		: app(*app), inControlOfCoordinatorLifecycle(inControlOfCoordinatorLifecycle)
	{
		coordinator = new gfx::SceneRenderCoordinator(this);
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
	}

}

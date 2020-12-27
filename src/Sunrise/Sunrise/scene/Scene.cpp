#include "srpch.h"
#include "Scene.h"


namespace sunrise {

	Scene::Scene()
	{

	}

	Scene::~Scene()
	{
	}

	void Scene::update()
	{
		static auto startTime = std::chrono::high_resolution_clock::now();
		PROFILE_FUNCTION;


		auto currentTime = std::chrono::high_resolution_clock::now();
		auto newtime = std::chrono::duration<double, std::chrono::seconds::period>(currentTime - startTime).count();
		auto newtimef = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		deltaTime = newtime - time;
		deltaTimef = newtimef - timef;

		time = newtime;
		timef = newtimef;
	}

}
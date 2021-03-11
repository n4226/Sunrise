#pragma once

#include "srpch.h"

namespace sunrise {
	namespace gfx {
		class SceneRenderCoordinator;
	}

	class SUNRISE_API Scene
	{
	public:
		Scene(Application* app);
		Scene(Application* app, gfx::SceneRenderCoordinator* coordinator);
		virtual ~Scene();

		virtual void load() = 0;

		virtual void update();

		virtual void unload() = 0;


		double time = 0;
		float timef = 0;

		double deltaTime = 0;
		float  deltaTimef = 0;

		size_t frameNum = 0;

		Application& app;

		gfx::SceneRenderCoordinator* coordinator;
	protected:
		bool inControlOfCoordinatorLifecycle = false;
	};

}
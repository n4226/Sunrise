#pragma once

#include "srpch.h"

namespace sunrise {
	namespace gfx {
		class SceneRenderCoordinator;
	}

	class SUNRISE_API Scene
	{
	public:
		Scene(Application* app, bool inControlOfCoordinatorLifecycle = true);
		Scene(Application* app, gfx::SceneRenderCoordinator* coordinator, bool inControlOfCoordinatorLifecycle = false);
		virtual ~Scene();

		virtual void load() = 0;
		/// <summary>
		/// after coordinator is fully initilized
		/// </summary>
		virtual void lateLoad() {}

		virtual void update();

		//TODO: better abstract
		virtual void onDrawUI() {};
		virtual void onDrawMainMenu() {};

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

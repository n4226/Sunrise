#pragma once

#include "srpch.h"
#include "../graphics/vulkan/renderer/RenderSystem.h"

namespace sunrise {
	namespace gfx {
		class SceneRenderCoordinator;
	}

	class SUNRISE_API Scene
	{
	public:
		/// <summary>
		/// for proper initilization, subclsses should set their coordinator in the body of their constructor
		/// </summary>
		/// <param name="app"></param>
		/// <param name="inControlOfCoordinatorLifecycle"></param>
		Scene(Application* app, bool inControlOfCoordinatorLifecycle = true);
		Scene(Application* app, gfx::SceneRenderCoordinator* coordinator, bool inControlOfCoordinatorLifecycle = false);
		virtual ~Scene();

		virtual void load() = 0;
		/// <summary>
		/// after coordinator is fully initialized
		/// make sure to call super implimentation before running code when overriding this method
		/// </summary>
		virtual void lateLoad();

		virtual void update();

		//TODO: better abstract
		virtual void onDrawUI() {};
		virtual void onDrawMainMenu() {};

		virtual void unload();


		double time = 0;
		float timef = 0;

		double deltaTime = 0;
		float  deltaTimef = 0;

		size_t frameNum = 0;

		Application& app;

		gfx::SceneRenderCoordinator* coordinator;

		//TODO: wrap entt types
		//entities
		entt::registry registry;
	protected:	
		bool inControlOfCoordinatorLifecycle = false;

		std::vector<System*> systems;
	};

}

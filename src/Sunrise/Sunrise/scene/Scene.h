#pragma once

#include "srpch.h"
#include "../graphics/vulkan/renderer/RenderSystem.h"

namespace sunrise {
	namespace gfx {
		class Renderer;
		class SceneRenderCoordinator;
		using SceneRenderCoordinatorCreator = std::function<SceneRenderCoordinator* (Renderer*)>;
	}


	class SUNRISE_API Scene
	{
	public:
		/// <summary>
		/// for proper initilization, subclsses should set their coordinators in the body of their constructor by calling initCoordinators
		/// </summary>
		/// <param name="app"></param>
		/// <param name="inControlOfCoordinatorLifecycle"></param>
		Scene(Application* app, bool inControlOfCoordinatorLifecycle = true);
		Scene(Application* app, gfx::SceneRenderCoordinatorCreator creator, bool inControlOfCoordinatorLifecycle = false);
		virtual ~Scene();

		virtual void load();
		/// <summary>
		/// after coordinator is fully initialized
		/// make sure to call super implimentation before running code when overriding this method
		/// </summary>
		virtual void lateLoad();

		virtual void update();
		
		//udually not overidden called in system::update()
		virtual void earlyUpdate() {};

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

		/// <summary>
		/// MARK: deprecated, Reason: did not support multi gpu
		/// </summary>
		//gfx::SceneRenderCoordinator* coordinator;
		std::unordered_map<gfx::Renderer*, gfx::SceneRenderCoordinator*> coordinators{};

		//TODO: wrap entt types
		//entities
		entt::registry registry;

		glm::vec2 sunLL{};
	protected:

		friend Application;

		gfx::SceneRenderCoordinatorCreator coordinatorCreator;

		void initCoordinators();

		bool inControlOfCoordinatorLifecycle = false;

		std::vector<System*> systems;
	};

}

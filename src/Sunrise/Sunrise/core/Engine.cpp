#include "srpch.h"
#include "Engine.h"
#include "Application.h"

#include "Log.h"

namespace sunrise {
	
	Engine* engine = nullptr;

	Engine::Engine(Application* app)
		: app(app)
	{
	}
	Engine::~Engine()
	{
		delete app;
	}
	void Engine::startup()
	{
		{
			Log::Init();

			SR_CORE_INFO("Startup");


			auto noApp = dynamic_cast<sunrise::NO_APPLICATION*>(app);

			if (noApp != nullptr) {
				SR_CORE_WARN("initializing in no application mode -- only basic operations will be available");
			}
			else {
				app->startup();
			}

		}
		run();
	}
	void Engine::run()
	{

	}

	void Engine::shutdown()
	{

	}
}
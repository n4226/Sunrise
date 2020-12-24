#include "srpch.h"
#include "Engine.h"

#include "Log.h"

namespace Sunrise {
	
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
		Log::Init();

		SR_CORE_INFO("Startup");

	}
}
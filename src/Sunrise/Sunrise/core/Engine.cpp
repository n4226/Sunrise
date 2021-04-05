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
		auto terminate = false;
		Instrumentor::Get().BeginSession("Launch", "instruments_Launch.profile");
		{
			PROFILE_FUNCTION;
			try {
				Log::Init();

				//SR_CORE_INFO("Startup");


				auto noApp = dynamic_cast<sunrise::NO_APPLICATION*>(app);


				if (noApp != nullptr) {
					SR_CORE_WARN("initializing in no application mode -- only basic operations will be available");
				}
				else {
					app->startup();
				}
			}
			catch (const std::exception& e) {
				SR_CORE_CRITICAL("Terminating from unhandled runtime exeption upon startup: {}", e.what());
				//throw e;
				terminate = true;
			}

		}
		Instrumentor::Get().EndSession();

		if (!terminate)
			run();
	}
	void Engine::run()
	{
		Instrumentor::Get().BeginSession("Run", "instruments_Run.profile");
		{
			PROFILE_FUNCTION;


			try {
				auto noApp = dynamic_cast<sunrise::NO_APPLICATION*>(app);
				if (noApp != nullptr) {
					SR_CORE_WARN("running in no application mode -- only basic operations will be available");
				}
				else {
					app->run();
				}
			}
			catch (const std::exception& e) {
				SR_CORE_CRITICAL("Terminating from unhandled runtime exeption durring run loop: {}", e.what());
				SR_CORE_ERROR("Terminating run loop during frame: {}", app->currentFrameID);
				//throw e;
			}
		}
		Instrumentor::Get().EndSession();

		shutdown();
	}

	void Engine::shutdown()
	{
		Instrumentor::Get().BeginSession("Shutdown", "instruments_Shutdown.profile");
		{
			PROFILE_FUNCTION;

			try {
				auto noApp = dynamic_cast<sunrise::NO_APPLICATION*>(app);
				if (noApp != nullptr) {
					SR_CORE_WARN("shutting down in no application mode -- only basic operations will be available");
				}
				else {
					app->shutdown();
				}
			}
			catch (const std::exception& e) {
				SR_CORE_CRITICAL("Terminating from unhandled runtime exeption durring sutdown loop: {}", e.what());
				SR_CORE_ERROR("Terminating shutdown during frame: {}", app->currentFrameID);
				//throw e;
			}

		}
		Instrumentor::Get().EndSession();
	}
}